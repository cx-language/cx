#include "asmjit_x64.h"
#pragma warning(push, 0)
#include <asmjit/core.h>
#include <asmjit/x86.h>
#include <llvm/ADT/StringSwitch.h>
#include <llvm/Support/DynamicLibrary.h>
#pragma warning(pop)

#include <algorithm>
#include <bit>
#include <cstdint>
#include <cstring>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../ast/mangle.h"
#include "../support/utility.h"
#include "ir.h"
#include "jit-pins.h"

using namespace cx;

namespace {
using namespace asmjit;
using namespace asmjit::x86;

// LLP64 type layouts (Windows x64): c_long/c_ulong are 32-bit, unlike the
// LP64 layouts in asmjit.cpp. Kept in sync with LLVM's DataLayout by
// construction; the ABI lit tests pin behavior.
uint64_t typeSize(IRType* type);
uint32_t typeAlign(IRType* type);

uint64_t basicSize(llvm::StringRef name) {
    // c_long matches C's long and c_size_t matches size_t; the host widths
    // are the target widths (no cross-compilation).
    if (name == "c_long" || name == "c_ulong") return sizeof(long);
    if (name == "c_size_t") return sizeof(void*);
    return llvm::StringSwitch<uint64_t>(name)
        .Cases({"bool", "char", "int8", "uint8", "c_schar", "c_uchar"}, 1)
        .Cases({"int16", "uint16", "c_short", "c_ushort"}, 2)
        .Cases({"int32", "uint32", "c_int", "c_uint", "float32", "c_float"}, 4)
        .Cases({"int64", "uint64", "c_longlong", "c_ulonglong", "float64", "c_double"}, 8)
        .Case("float80", 16)
        .Case("void", 0)
        .Default(0);
}

uint64_t typeSize(IRType* type) {
    switch (type->kind) {
    case IRTypeKind::IRBasicType: {
        uint64_t size = basicSize(type->getName());
        ASSERT(size > 0 && "unknown basic type");
        return size;
    }
    case IRTypeKind::IRPointerType:
    case IRTypeKind::IRFunctionType:
        return 8;
    case IRTypeKind::IRArrayType: {
        auto* arrayType = llvm::cast<IRArrayType>(type);
        uint64_t count;
        if (arrayType->hasSymbolicSize()) {
            count = typeSize(getIRType(arrayType->sizeofOperand));
        } else {
            ASSERT(arrayType->size >= 0);
            count = (uint64_t)arrayType->size;
        }
        return count * typeSize(arrayType->elementType);
    }
    case IRTypeKind::IRStructType: {
        auto* structType = llvm::cast<IRStructType>(type);
        uint64_t offset = 0;
        uint32_t align = 1;
        for (const auto& field : structType->fields) {
            uint32_t fieldAlign = structType->packed ? 1 : typeAlign(field.type);
            align = std::max(align, fieldAlign);
            offset = (offset + fieldAlign - 1) / fieldAlign * fieldAlign + typeSize(field.type);
        }
        if (structType->packed) align = 1;
        return (offset + align - 1) / align * align;
    }
    case IRTypeKind::IRUnionType: {
        uint64_t size = 0;
        for (const auto& field : type->getFields())
            size = std::max(size, typeSize(field.type));
        uint32_t align = typeAlign(type);
        return (size + align - 1) / align * align;
    }
    }
    llvm_unreachable("all cases handled");
}

uint32_t typeAlign(IRType* type) {
    switch (type->kind) {
    case IRTypeKind::IRBasicType: {
        llvm::StringRef name = type->getName();
        if (name == "void") return 1;
        return (uint32_t)basicSize(name);
    }
    case IRTypeKind::IRPointerType:
    case IRTypeKind::IRFunctionType:
        return 8;
    case IRTypeKind::IRArrayType:
        return typeAlign(llvm::cast<IRArrayType>(type)->elementType);
    case IRTypeKind::IRStructType: {
        auto* structType = llvm::cast<IRStructType>(type);
        if (structType->packed) return 1;
        uint32_t align = 1;
        for (const auto& field : structType->fields)
            align = std::max(align, typeAlign(field.type));
        return align;
    }
    case IRTypeKind::IRUnionType: {
        uint32_t align = 1;
        for (const auto& field : type->getFields())
            align = std::max(align, typeAlign(field.type));
        return align;
    }
    }
    llvm_unreachable("all cases handled");
}

uint64_t fieldOffset(IRType* structType, int index) {
    ASSERT(structType->isStruct());
    auto* type = llvm::cast<IRStructType>(structType);
    uint64_t offset = 0;
    for (int i = 0; i <= index; ++i) {
        uint32_t fieldAlign = type->packed ? 1 : typeAlign(type->fields[i].type);
        offset = (offset + fieldAlign - 1) / fieldAlign * fieldAlign;
        if (i < index) offset += typeSize(type->fields[i].type);
    }
    return offset;
}

// How a value crosses the Win64 C ABI. Small aggregates lower to one
// integer register like clang (mirrors LLVMGenerator::getAbiCoercedType);
// larger ones cross by pointer. Win64 never uses a second register or
// vector registers for aggregates.
struct AbiClass {
    enum class Kind {
        Direct, // One register; typeId is the AsmJit type.
        Chunk, // One integer register covering the bytes (u32 or u64).
        Indirect, // Pointer to caller memory.
        Empty, // Zero-size: occupies no ABI slot.
    };
    Kind kind;
    TypeId typeId = TypeId::kVoid; // Direct only.
    bool chunkIs64 = true; // Chunk only (u64 chunk, else u32).
};

AbiClass classifyType(IRType* type) {
    if (type->isVoid()) return {AbiClass::Kind::Direct, TypeId::kVoid};
    if (type->isBool()) return {AbiClass::Kind::Direct, TypeId::kUInt32};
    if (type->isChar()) return {AbiClass::Kind::Direct, TypeId::kUInt32};
    if (type->isInteger()) {
        int width = getIntegerBitWidth(type);
        TypeId id = width <= 8  ? (type->isSignedInteger() ? TypeId::kInt8 : TypeId::kUInt8)
                  : width <= 16 ? (type->isSignedInteger() ? TypeId::kInt16 : TypeId::kUInt16)
                  : width <= 32 ? (type->isSignedInteger() ? TypeId::kInt32 : TypeId::kUInt32)
                                : (type->isSignedInteger() ? TypeId::kInt64 : TypeId::kUInt64);
        return {AbiClass::Kind::Direct, id};
    }
    if (type->isFloatingPoint()) {
        llvm::StringRef name = type->getName();
        if (name == "float32" || name == "c_float") return {AbiClass::Kind::Direct, TypeId::kFloat32};
        if (name == "float64" || name == "c_double") return {AbiClass::Kind::Direct, TypeId::kFloat64};
        ASSERT(name == "float80");
        // float80 travels as a pointer to a 16-byte home (cx-internal
        // convention; float80 never crosses an extern boundary here).
        return {AbiClass::Kind::Indirect};
    }
    if (type->isPointerType() || type->isFunctionType()) return {AbiClass::Kind::Direct, TypeId::kUInt64};
    // Aggregates.
    uint64_t size = typeSize(type);
    if (size == 0) return {AbiClass::Kind::Empty};
    // Win64 passes aggregates up to 8 bytes as integers, floats included
    // (mirrors the LLVM backend); larger ones cross by pointer.
    if (size <= 8) {
        AbiClass cls{AbiClass::Kind::Chunk};
        cls.chunkIs64 = size > 4;
        return cls;
    }
    return {AbiClass::Kind::Indirect};
}

// True when a float80 crosses by value rather than behind a pointer.
// Extern boundaries reject these: C's long double is 64-bit on Win64, so
// neither passing convention agrees with the 16-byte homes used internally.
bool hasByValueFloat80(IRType* type) {
    if (!type) return false;
    switch (type->kind) {
    case IRTypeKind::IRBasicType:
        return type->getName() == "float80";
    case IRTypeKind::IRPointerType:
    case IRTypeKind::IRFunctionType:
        return false;
    case IRTypeKind::IRArrayType:
        return hasByValueFloat80(llvm::cast<IRArrayType>(type)->elementType);
    case IRTypeKind::IRStructType:
    case IRTypeKind::IRUnionType:
        for (const auto& field : type->getFields())
            if (hasByValueFloat80(field.type)) return true;
        return false;
    }
    llvm_unreachable("all cases handled");
}

struct JitMemory {
    // Process-lifetime JIT memory (globals, strings, function table). Freed at exit.
    std::vector<void*> blocks;
    ~JitMemory() {
        for (void* block : blocks)
            free(block);
    }
    void* alloc(size_t size, size_t align = 8) {
        // Plain malloc: 16-byte aligned everywhere, and no cx type aligns past 16.
        ASSERT(align <= 16);
        void* block = calloc(size ? size : 1, 1);
        if (!block) ABORT("out of memory in JIT allocator");
        blocks.push_back(block);
        return block;
    }
};

struct X64AsmJitGenerator {
    JitRuntime runtime;
    JitMemory memory;
    // Function address table: every function (defined, extern, or referenced
    // before compilation) occupies one slot. Direct calls and function values
    // load the slot at runtime, so emission order never matters.
    std::unordered_map<const Function*, size_t> funcIndex;
    void** funcTable = nullptr;

    std::unordered_map<const GlobalVariable*, void*> globalAddr;
    std::unordered_map<std::string, void*> globalAddrByName;
    std::unordered_map<std::string, void*> stringAddr;
    std::unordered_map<std::string, void*> externCache;

    // Per-function emission state.
    x86::Compiler* cc = nullptr;
    std::unordered_map<const Value*, Gp> gpValues;
    std::unordered_map<const Value*, Vec> vecValues;
    std::unordered_map<const BasicBlock*, Label> blockLabels;
    std::unordered_map<const Value*, Gp> checkedOverflow;
    Gp scratch0, scratch1;

    void codegenModules(const std::vector<IRModule*>& modules);
    void* codegenFunction(const Function* function);
    void codegenInst(const Value* value);
    Gp getGp(const Value* value);
    Vec getVec(const Value* value);
    Gp newIntReg(IRType* type);
    void canonicalize(Gp reg, IRType* type);
    Gp emitIntBinary(Token::Kind op, Gp left, Gp right, IRType* type, IRType* rightType);
    void emitDivMod(Gp out, Gp left, Gp right, bool is64, bool isSigned, bool isMod);
    Vec emitFloatBinary(Token::Kind op, Vec left, Vec right, bool isDouble);
    Gp emitIntCompare(Token::Kind op, Gp left, Gp right, IRType* type);
    Gp emitFloatCompare(Token::Kind op, Vec left, Vec right, bool isDouble);
    void emitCast(const CastInst* inst);
    Vec callFmod(Vec left, Vec right, Token::Kind op, bool isDouble);
    void emitCall(const CallInst* inst);
    void emitReturn(const ReturnInst* inst);
    void emitCheckedArith(const CheckedArithInst* inst);
    void emitArrayOp(const ArrayOpInst* inst);

    // Current function ABI state for returns.
    AbiClass currentRetClass{AbiClass::Kind::Direct, TypeId::kVoid};
    IRType* currentRetType = nullptr;
    Gp currentSret;
    void* getGlobalAddr(const GlobalVariable* global);
    void* getStringAddr(const std::string& value);
    void* resolveExtern(llvm::StringRef name);
    void emitMemcpy(Gp dest, Gp src, uint64_t size);
    // Memory operand for [base + offset]; x86 displacements span the full
    // 32-bit range, so folding only triggers defensively.
    x86::Mem memAt(Gp base, uint64_t offset, uint32_t size);
    Gp homeAddr(uint64_t size, uint32_t align);
    uint64_t evalConstInt(const Value* value);
    llvm::APFloat evalConstFloat(const Value* value);
    void initGlobal(const GlobalVariable* global);
    void storeConstToAddr(const Value* value, char* addr);

    // float80 lowers to calls into raw-x87 helpers (the x86 Compiler has no
    // x87 register allocation); values are pointers to 16-byte homes.
    enum class F80Op {
        Add, Sub, Mul, Div, Neg, Mod, CmpFlags,
        FromF32, FromF64, ToF32, ToF64,
        FromI32, FromI64, FromU32, FromU64,
        ToI32, ToI64, ToU32, ToU64, ToBool,
        Count,
    };
    void* f80helpers[(size_t)F80Op::Count] = {};
    void* getF80Helper(F80Op op);
    void emitF80Helper(x86::Assembler& as, F80Op op);
    Gp emitF80Binary(Token::Kind op, Gp left, Gp right);
    Gp emitF80Compare(Token::Kind op, Gp left, Gp right);
    void emitF80Cast(const CastInst* inst, IRType* sourceType, IRType* type);
    void callF80Arith(F80Op op, Gp out, Gp a, Gp b);
    void callF80Unary(F80Op op, Gp out, Gp a);
    Gp callF80Cmp(Gp a, Gp b);
    Gp callF80ToWord(F80Op op, Gp a, bool is64);
    void callF80FromWord(F80Op op, Gp out, Gp value, bool is64);
};

void* X64AsmJitGenerator::resolveExtern(llvm::StringRef name) {
    static bool librariesLoaded = [] {
        llvm::sys::DynamicLibrary::getPermanentLibrary(nullptr);
        // Mirror the LLVM JIT: Linux keeps libm separate from libc.
        for (const char* lib : {"libm.so.6", "libm.so"}) {
            if (!llvm::sys::DynamicLibrary::LoadLibraryPermanently(lib)) break;
        }
        return true;
    }();
    (void)librariesLoaded;
    std::string key = name.str();
    auto it = externCache.find(key);
    if (it != externCache.end()) return it->second;
    void* addr = nullptr;
#ifdef _WIN32
    // Pinned libc names resolve to the host's address so JITed code binds
    // the UCRT instead of legacy msvcrt.dll (same as the LLVM JIT).
    addr = lookupPinnedLibcSymbol(name);
#endif
    if (!addr) addr = llvm::sys::DynamicLibrary::SearchForAddressOfSymbol(stripAsmLabelMarker(name).str());
    externCache.emplace(std::move(key), addr);
    return addr;
}

void* X64AsmJitGenerator::getGlobalAddr(const GlobalVariable* global) {
    auto it = globalAddr.find(global);
    if (it != globalAddr.end()) return it->second;
    // Same-name globals across modules alias one home, like linked symbols.
    auto named = globalAddrByName.find(global->name);
    if (named != globalAddrByName.end()) {
        globalAddr.emplace(global, named->second);
        return named->second;
    }
    void* addr;
    if (!global->value) {
        addr = resolveExtern(global->name);
        ASSERT(addr && "ineligible extern slipped into AsmJit session");
    } else {
        addr = memory.alloc((size_t)typeSize(global->type), typeAlign(global->type));
    }
    globalAddr.emplace(global, addr);
    globalAddrByName.emplace(global->name, addr);
    return addr;
}

void* X64AsmJitGenerator::getStringAddr(const std::string& value) {
    auto it = stringAddr.find(value);
    if (it != stringAddr.end()) return it->second;
    char* addr = (char*)memory.alloc(value.size() + 1, 1);
    memcpy(addr, value.data(), value.size());
    stringAddr.emplace(value, addr);
    return addr;
}

// Chunk stores write whole 4/8-byte units (a 3-byte struct takes four), so
// exact-sized slots would overrun into densely packed neighbors.
uint64_t padHome(uint64_t size) {
    return std::max<uint64_t>(8, (size + 7) & ~7ULL);
}

Gp X64AsmJitGenerator::homeAddr(uint64_t size, uint32_t align) {
    uint64_t padded = padHome(size);
    x86::Mem slot = cc->new_stack((uint32_t)padded, align == 0 ? 1 : align);
    Gp addr = cc->new_gp64();
    cc->lea(addr, slot);
    return addr;
}

x86::Mem X64AsmJitGenerator::memAt(Gp base, uint64_t offset, uint32_t size) {
    Gp addr = base;
    if (offset > (uint64_t)INT32_MAX) {
        // Defensive; JIT frames never reach 2GB.
        Gp tmp = cc->new_gp64();
        cc->mov(tmp, offset);
        cc->add(tmp, base);
        addr = tmp;
        offset = 0;
    }
    int32_t disp = (int32_t)offset;
    switch (size) {
    case 1:
        return x86::byte_ptr(addr, disp);
    case 2:
        return x86::word_ptr(addr, disp);
    case 4:
        return x86::dword_ptr(addr, disp);
    case 8:
        return x86::qword_ptr(addr, disp);
    case 16:
        return x86::xmmword_ptr(addr, disp);
    }
    llvm_unreachable("bad mem size");
}

void X64AsmJitGenerator::emitMemcpy(Gp dest, Gp src, uint64_t size) {
    uint64_t offset = 0;
    for (; offset + 8 <= size; offset += 8) {
        cc->mov(scratch0, memAt(src, offset, 8));
        cc->mov(memAt(dest, offset, 8), scratch0);
    }
    if (offset + 4 <= size) {
        Gp tmp = cc->new_gp32();
        cc->mov(tmp, memAt(src, offset, 4));
        cc->mov(memAt(dest, offset, 4), tmp);
        offset += 4;
    }
    if (offset + 2 <= size) {
        Gp tmp = cc->new_gp32();
        cc->mov(tmp.r16(), memAt(src, offset, 2));
        cc->mov(memAt(dest, offset, 2), tmp.r16());
        offset += 2;
    }
    if (offset < size) {
        Gp tmp = cc->new_gp32();
        cc->mov(tmp.r8(), memAt(src, offset, 1));
        cc->mov(memAt(dest, offset, 1), tmp.r8());
    }
}

// Canonical in-register forms: bool/char/int8/int16/uint8/uint16 live
// extended in 32-bit registers (sign- or zero- per type), int32/uint32 in
// 32-bit, int64/uint64/pointers in 64-bit, floats in XMM registers.
// Aggregates and float80 always live in memory; their value is a pointer.
bool isDoubleType(IRType* type) {
    llvm::StringRef name = type->getName();
    return name == "float64" || name == "c_double";
}

bool isFloat80Type(IRType* type) {
    return type->isBasicType() && type->getName() == "float80";
}

// Values living in memory rather than registers: aggregates plus float80,
// whose values are pointers to 16-byte homes.
bool livesInMemory(IRType* type) {
    return type->isStruct() || type->isUnion() || type->isArrayType() || isFloat80Type(type);
}

// Values with no home block (constants, undef, globals, function addresses)
// are rematerialized at every use instead of cached: a cached virtual
// register defined at the first use site does not dominate uses in other
// blocks, and AsmJit liveness is CFG-based, so the register arrives dead.
bool isRematerializable(ValueKind kind) {
    switch (kind) {
    case ValueKind::ConstantInt:
    case ValueKind::ConstantFP:
    case ValueKind::ConstantBool:
    case ValueKind::ConstantNull:
    case ValueKind::ConstantString:
    case ValueKind::Undefined:
    case ValueKind::GlobalVariable:
    case ValueKind::Function:
        return true;
    default:
        return false;
    }
}

Gp X64AsmJitGenerator::getGp(const Value* value) {
    auto it = gpValues.find(value);
    if (it != gpValues.end()) return it->second;
    codegenInst(value);
    auto found = gpValues.find(value);
    ASSERT(found != gpValues.end() && "instruction produced no Gp value");
    Gp reg = found->second;
    if (isRematerializable(value->kind)) gpValues.erase(found);
    return reg;
}

Vec X64AsmJitGenerator::getVec(const Value* value) {
    auto it = vecValues.find(value);
    if (it != vecValues.end()) return it->second;
    codegenInst(value);
    auto found = vecValues.find(value);
    ASSERT(found != vecValues.end() && "instruction produced no Vec value");
    Vec reg = found->second;
    if (isRematerializable(value->kind)) vecValues.erase(found);
    return reg;
}

void X64AsmJitGenerator::codegenInst(const Value* value) {
    switch (value->kind) {
    case ValueKind::AllocaInst: {
        if (gpValues.find(value) != gpValues.end()) return; // hoisted in prologue
        auto* inst = llvm::cast<AllocaInst>(value);
        gpValues.emplace(value, homeAddr(typeSize(inst->allocatedType), typeAlign(inst->allocatedType)));
        return;
    }
    case ValueKind::ConstantInt: {
        auto* inst = llvm::cast<ConstantInt>(value);
        int width = getIntegerBitWidth(inst->type);
        if (width <= 0) width = inst->type->isPointerType() ? 64 : 8; // char/bool constants
        // The APSInt's own signedness is unreliable; extend per the type.
        uint64_t bits = inst->value.extOrTrunc((unsigned)width).getZExtValue();
        bool typeSigned = inst->type->isSignedInteger();
        if (width > 32) {
            int64_t extended = (int64_t)bits;
            if (typeSigned && width < 64 && (bits & (1ULL << (width - 1)))) extended |= (int64_t)(~0ULL << width);
            Gp reg = cc->new_gp64();
            cc->mov(reg, extended);
            gpValues.emplace(value, reg);
        } else {
            // Canonical 32-bit form: narrow values arrive already extended.
            int64_t extended = (int64_t)bits;
            if (typeSigned && width < 32 && (bits & (1ULL << (width - 1)))) extended |= (int64_t)(~0ULL << width);
            Gp reg = cc->new_gp32();
            cc->mov(reg, (int32_t)extended);
            gpValues.emplace(value, reg);
        }
        return;
    }
    case ValueKind::ConstantFP: {
        auto* inst = llvm::cast<ConstantFP>(value);
        llvm::StringRef fpName = inst->type->getName();
        ASSERT(fpName == "float32" || fpName == "c_float" || fpName == "float64" || fpName == "c_double" || fpName == "float80");
        if (isFloat80Type(inst->type)) {
            Gp reg = cc->new_gp64();
            void* home = memory.alloc(16, 16);
            llvm::APInt apBits = inst->value.bitcastToAPInt();
            ASSERT(apBits.getBitWidth() == 80);
            memcpy(home, apBits.getRawData(), 10);
            cc->mov(reg, (uint64_t)home);
            gpValues.emplace(value, reg);
            return;
        }
        if (isDoubleType(inst->type)) {
            Vec reg = cc->new_xmm_sd();
            x86::Mem mem = cc->new_int64_const(ConstPoolScope::kLocal, (int64_t)inst->value.bitcastToAPInt().getZExtValue());
            cc->movsd(reg, mem);
            vecValues.emplace(value, reg);
        } else {
            Vec reg = cc->new_xmm_ss();
            uint32_t bits = (uint32_t)inst->value.bitcastToAPInt().getZExtValue();
            x86::Mem mem = cc->new_int32_const(ConstPoolScope::kLocal, (int32_t)bits);
            cc->movss(reg, mem);
            vecValues.emplace(value, reg);
        }
        return;
    }
    case ValueKind::ConstantBool: {
        Gp reg = cc->new_gp32();
        cc->mov(reg, llvm::cast<ConstantBool>(value)->value ? 1 : 0);
        gpValues.emplace(value, reg);
        return;
    }
    case ValueKind::ConstantNull: {
        Gp reg = cc->new_gp64();
        cc->mov(reg, 0);
        gpValues.emplace(value, reg);
        return;
    }
    case ValueKind::ConstantString: {
        auto* inst = llvm::cast<ConstantString>(value);
        Gp reg = cc->new_gp64();
        cc->mov(reg, (uint64_t)getStringAddr(inst->value));
        gpValues.emplace(value, reg);
        return;
    }
    case ValueKind::Undefined: {
        // Any value goes; zero keeps tools quiet. Aggregates need a pointer:
        // point at fresh stack space (never legally read).
        IRType* type = value->getType();
        if (type->isFloatingPoint() && !isFloat80Type(type)) {
            Vec reg = isDoubleType(type) ? cc->new_xmm_sd() : cc->new_xmm_ss();
            cc->xorps(reg, reg);
            vecValues.emplace(value, reg);
            return;
        }
        if (livesInMemory(type)) {
            gpValues.emplace(value, homeAddr(typeSize(type), typeAlign(type)));
            return;
        }
        Gp reg = cc->new_gp32();
        if (type->isPointerType() || (type->isInteger() && getIntegerBitWidth(type) > 32)) reg = cc->new_gp64();
        cc->mov(reg, 0);
        gpValues.emplace(value, reg);
        return;
    }
    case ValueKind::GlobalVariable: {
        Gp reg = cc->new_gp64();
        cc->mov(reg, (uint64_t)getGlobalAddr(llvm::cast<GlobalVariable>(value)));
        gpValues.emplace(value, reg);
        return;
    }
    case ValueKind::Function: {
        // Load the table slot at runtime: the callee may compile later.
        auto* function = llvm::cast<Function>(value);
        Gp slot = cc->new_gp64();
        cc->mov(slot, (uint64_t)&funcTable[funcIndex.at(function)]);
        Gp reg = cc->new_gp64();
        cc->mov(reg, x86::qword_ptr(slot));
        gpValues.emplace(value, reg);
        return;
    }
    case ValueKind::Parameter:
        llvm_unreachable("parameters are pre-assigned, never emitted");
    case ValueKind::BinaryInst: {
        auto* inst = llvm::cast<BinaryInst>(value);
        IRType* type = inst->left->getType();
        if (isFloat80Type(type)) {
            Gp left = getGp(inst->left), right = getGp(inst->right);
            switch (inst->op) {
            case Token::Equal:
            case Token::NotEqual:
            case Token::Less:
            case Token::LessOrEqual:
            case Token::Greater:
            case Token::GreaterOrEqual:
                gpValues.emplace(value, emitF80Compare(inst->op, left, right));
                return;
            default:
                gpValues.emplace(value, emitF80Binary(inst->op, left, right));
                return;
            }
        }
        if (type->isFloatingPoint()) {
            bool isDouble = isDoubleType(type);
            Vec left = getVec(inst->left), right = getVec(inst->right);
            switch (inst->op) {
            case Token::Equal:
            case Token::NotEqual:
            case Token::Less:
            case Token::LessOrEqual:
            case Token::Greater:
            case Token::GreaterOrEqual:
                gpValues.emplace(value, emitFloatCompare(inst->op, left, right, isDouble));
                return;
            default:
                vecValues.emplace(value, emitFloatBinary(inst->op, left, right, isDouble));
                return;
            }
        }
        Gp left = getGp(inst->left), right = getGp(inst->right);
        switch (inst->op) {
        case Token::Equal:
        case Token::NotEqual:
        case Token::Less:
        case Token::LessOrEqual:
        case Token::Greater:
        case Token::GreaterOrEqual:
            gpValues.emplace(value, emitIntCompare(inst->op, left, right, type));
            return;
        default:
            gpValues.emplace(value, emitIntBinary(inst->op, left, right, type, inst->right->getType()));
            return;
        }
    }
    case ValueKind::UnaryInst: {
        auto* inst = llvm::cast<UnaryInst>(value);
        IRType* type = inst->operand->getType();
        if (inst->op == Token::Star) {
            gpValues.emplace(value, getGp(inst->operand));
            return;
        }
        if (inst->op == Token::Plus) {
            if (type->isFloatingPoint() && !isFloat80Type(type))
                vecValues.emplace(value, getVec(inst->operand));
            else
                gpValues.emplace(value, getGp(inst->operand));
            return;
        }
        if (isFloat80Type(type)) {
            ASSERT(inst->op == Token::Minus);
            Gp home = homeAddr(16, 16);
            callF80Unary(F80Op::Neg, home, getGp(inst->operand));
            gpValues.emplace(value, home);
            return;
        }
        if (type->isFloatingPoint()) {
            ASSERT(inst->op == Token::Minus);
            bool isDouble = isDoubleType(type);
            Vec in = getVec(inst->operand);
            Vec out = isDouble ? cc->new_xmm_sd() : cc->new_xmm_ss();
            Vec mask = isDouble ? cc->new_xmm_sd() : cc->new_xmm_ss();
            if (isDouble) {
                cc->movsd(mask, cc->new_int64_const(ConstPoolScope::kLocal, INT64_MIN));
                cc->movsd(out, in);
                cc->xorpd(out, mask);
            } else {
                cc->movss(mask, cc->new_int32_const(ConstPoolScope::kLocal, INT32_MIN));
                cc->movss(out, in);
                cc->xorps(out, mask);
            }
            vecValues.emplace(value, out);
            return;
        }
        Gp in = getGp(inst->operand);
        Gp out = newIntReg(type);
        if (inst->op == Token::Minus) {
            cc->mov(out, in);
            cc->neg(out);
        } else if (inst->op == Token::Not) {
            cc->xor_(out, out);
            cc->cmp(in, 0);
            cc->setz(out.r8());
            gpValues.emplace(value, out);
            return;
        } else {
            ASSERT(inst->op == Token::Tilde);
            if (type->isBool()) {
                cc->mov(out, in);
                cc->xor_(out, 1);
            } else {
                cc->mov(out, in);
                cc->not_(out);
            }
        }
        canonicalize(out, type);
        gpValues.emplace(value, out);
        return;
    }
    case ValueKind::CastInst:
        emitCast(llvm::cast<CastInst>(value));
        return;
    case ValueKind::LoadInst: {
        auto* inst = llvm::cast<LoadInst>(value);
        IRType* type = inst->getType();
        Gp ptr = getGp(inst->value);
        if (livesInMemory(type)) {
            // Copy to a fresh home: the source may be stored through later.
            uint64_t size = typeSize(type);
            Gp home = homeAddr(size, typeAlign(type));
            emitMemcpy(home, ptr, size);
            gpValues.emplace(value, home);
            return;
        }
        if (type->isFloatingPoint()) {
            Vec out = isDoubleType(type) ? cc->new_xmm_sd() : cc->new_xmm_ss();
            if (isDoubleType(type))
                cc->movsd(out, x86::qword_ptr(ptr));
            else
                cc->movss(out, x86::dword_ptr(ptr));
            vecValues.emplace(value, out);
            return;
        }
        Gp out = newIntReg(type);
        int width = type->isInteger() ? getIntegerBitWidth(type) : type->isBool() || type->isChar() ? 8 : 64;
        if (width == 64 || type->isPointerType()) {
            cc->mov(out, x86::qword_ptr(ptr));
        } else if (width == 32) {
            cc->mov(out, x86::dword_ptr(ptr));
        } else if (width == 16) {
            if (type->isSignedInteger())
                cc->movsx(out, x86::word_ptr(ptr));
            else
                cc->movzx(out, x86::word_ptr(ptr));
        } else {
            if (type->isSignedInteger())
                cc->movsx(out, x86::byte_ptr(ptr));
            else
                cc->movzx(out, x86::byte_ptr(ptr));
        }
        gpValues.emplace(value, out);
        return;
    }
    case ValueKind::StoreInst: {
        auto* inst = llvm::cast<StoreInst>(value);
        if (inst->value->kind == ValueKind::Undefined) return;
        IRType* type = inst->value->getType();
        Gp ptr = getGp(inst->pointer);
        if (livesInMemory(type)) {
            emitMemcpy(ptr, getGp(inst->value), typeSize(type));
            return;
        }
        if (type->isFloatingPoint()) {
            if (isDoubleType(type))
                cc->movsd(x86::qword_ptr(ptr), getVec(inst->value));
            else
                cc->movss(x86::dword_ptr(ptr), getVec(inst->value));
            return;
        }
        int width = type->isInteger() ? getIntegerBitWidth(type) : type->isBool() || type->isChar() ? 8 : 64;
        Gp val = getGp(inst->value);
        if (width == 64 || type->isPointerType()) {
            cc->mov(x86::qword_ptr(ptr), val);
        } else if (width == 32) {
            cc->mov(x86::dword_ptr(ptr), val);
        } else if (width == 16) {
            cc->mov(x86::word_ptr(ptr), val.r16());
        } else {
            cc->mov(x86::byte_ptr(ptr), val.r8());
        }
        return;
    }
    case ValueKind::GEPInst: {
        auto* inst = llvm::cast<GEPInst>(value);
        Gp base = getGp(inst->pointer);
        Gp out = cc->new_gp64();
        cc->mov(out, base);
        IRType* type = inst->pointer->getType()->getPointee();
        bool first = true;
        for (Value* index : inst->indexes) {
            uint64_t stride;
            if (first) {
                stride = typeSize(type);
                first = false;
            } else if (type->isArrayType()) {
                type = type->getElementType();
                stride = typeSize(type);
            } else {
                llvm_unreachable("non-leading GEP index over non-array type");
            }
            if (auto* constant = llvm::dyn_cast<ConstantInt>(index)) {
                int64_t i = (int64_t)constant->value.extOrTrunc(64).getZExtValue();
                if (i == 0) continue;
                int64_t bytes = i * (int64_t)stride;
                if (bytes >= 0 && bytes <= 4095) {
                    cc->add(out, bytes);
                } else if (bytes < 0 && bytes >= -4095) {
                    cc->sub(out, -bytes);
                } else {
                    Gp tmp = cc->new_gp64();
                    cc->mov(tmp, bytes);
                    cc->add(out, tmp);
                }
            } else {
                if (stride == 0) continue; // Indexing over a zero-size type adds nothing.
                Gp idx = getGp(index);
                Gp wide = cc->new_gp64();
                if (idx.size() == 4) {
                    // 32-bit indexes sign-extend for negative indexing.
                    cc->movsxd(wide, idx);
                } else {
                    cc->mov(wide, idx);
                }
                if (stride == 1) {
                    cc->add(out, wide);
                } else if ((stride & (stride - 1)) == 0) {
                    cc->shl(wide, std::countr_zero(stride));
                    cc->add(out, wide);
                } else {
                    Gp tmp = cc->new_gp64();
                    cc->mov(tmp, stride);
                    cc->imul(wide, tmp);
                    cc->add(out, wide);
                }
            }
        }
        gpValues.emplace(value, out);
        return;
    }
    case ValueKind::ConstGEPInst: {
        auto* inst = llvm::cast<ConstGEPInst>(value);
        IRType* base = inst->pointer->getType()->getPointee();
        Gp ptr = getGp(inst->pointer);
        uint64_t offset;
        if (base->isStruct())
            offset = fieldOffset(base, inst->index);
        else if (base->isUnion())
            offset = 0;
        else {
            ASSERT(base->isArrayType());
            offset = (uint64_t)inst->index * typeSize(base->getElementType());
        }
        if (offset == 0) {
            gpValues.emplace(value, ptr);
            return;
        }
        Gp out = cc->new_gp64();
        if (offset <= 4095) {
            cc->mov(out, ptr);
            cc->add(out, (int64_t)offset);
        } else {
            Gp tmp = cc->new_gp64();
            cc->mov(tmp, offset);
            cc->mov(out, ptr);
            cc->add(out, tmp);
        }
        gpValues.emplace(value, out);
        return;
    }
    case ValueKind::InsertInst: {
        auto* inst = llvm::cast<InsertInst>(value);
        IRType* type = inst->aggregate->getType();
        ASSERT(type->isStruct() || type->isUnion() || type->isArrayType());
        uint64_t size = typeSize(type);
        Gp home = homeAddr(size, typeAlign(type));
        if (inst->aggregate->kind != ValueKind::Undefined) emitMemcpy(home, getGp(inst->aggregate), size);
        IRType* fieldType = inst->value->getType();
        uint64_t offset;
        if (type->isStruct())
            offset = fieldOffset(type, inst->index);
        else if (type->isUnion())
            offset = 0;
        else
            offset = (uint64_t)inst->index * typeSize(type->getElementType());
        if (inst->value->kind != ValueKind::Undefined) {
            if (livesInMemory(fieldType)) {
                Gp fieldPtr = cc->new_gp64();
                cc->mov(fieldPtr, home);
                if (offset > 0) {
                    if (offset <= 4095) {
                        cc->add(fieldPtr, (int64_t)offset);
                    } else {
                        Gp tmp = cc->new_gp64();
                        cc->mov(tmp, offset);
                        cc->add(fieldPtr, tmp);
                    }
                }
                emitMemcpy(fieldPtr, getGp(inst->value), typeSize(fieldType));
            } else if (fieldType->isFloatingPoint()) {
                if (isDoubleType(fieldType))
                    cc->movsd(memAt(home, offset, 8), getVec(inst->value));
                else
                    cc->movss(memAt(home, offset, 4), getVec(inst->value));
            } else {
                int width = fieldType->isInteger() ? getIntegerBitWidth(fieldType) : fieldType->isBool() || fieldType->isChar() ? 8 : 64;
                Gp val = getGp(inst->value);
                if (width == 64 || fieldType->isPointerType())
                    cc->mov(memAt(home, offset, 8), val);
                else if (width == 32)
                    cc->mov(memAt(home, offset, 4), val);
                else if (width == 16)
                    cc->mov(memAt(home, offset, 2), val.r16());
                else
                    cc->mov(memAt(home, offset, 1), val.r8());
            }
        }
        gpValues.emplace(value, home);
        return;
    }
    case ValueKind::ExtractInst: {
        auto* inst = llvm::cast<ExtractInst>(value);
        IRType* aggType = inst->aggregate->getType();
        IRType* type = inst->getType();
        Gp base = getGp(inst->aggregate);
        uint64_t offset;
        if (aggType->isStruct())
            offset = fieldOffset(aggType, inst->index);
        else if (aggType->isUnion())
            offset = 0;
        else
            offset = (uint64_t)inst->index * typeSize(aggType->getElementType());
        if (livesInMemory(type)) {
            uint64_t size = typeSize(type);
            Gp home = homeAddr(size, typeAlign(type));
            Gp fieldPtr = cc->new_gp64();
            cc->mov(fieldPtr, base);
            if (offset > 0) {
                if (offset <= 4095) {
                    cc->add(fieldPtr, (int64_t)offset);
                } else {
                    Gp tmp = cc->new_gp64();
                    cc->mov(tmp, offset);
                    cc->add(fieldPtr, tmp);
                }
            }
            emitMemcpy(home, fieldPtr, size);
            gpValues.emplace(value, home);
            return;
        }
        if (type->isFloatingPoint()) {
            Vec out = isDoubleType(type) ? cc->new_xmm_sd() : cc->new_xmm_ss();
            if (isDoubleType(type))
                cc->movsd(out, memAt(base, offset, 8));
            else
                cc->movss(out, memAt(base, offset, 4));
            vecValues.emplace(value, out);
            return;
        }
        Gp out = newIntReg(type);
        int width = type->isInteger() ? getIntegerBitWidth(type) : type->isBool() || type->isChar() ? 8 : 64;
        if (width == 64 || type->isPointerType())
            cc->mov(out, memAt(base, offset, 8));
        else if (width == 32)
            cc->mov(out, memAt(base, offset, 4));
        else if (width == 16) {
            if (type->isSignedInteger())
                cc->movsx(out, memAt(base, offset, 2));
            else
                cc->movzx(out, memAt(base, offset, 2));
        } else {
            if (type->isSignedInteger())
                cc->movsx(out, memAt(base, offset, 1));
            else
                cc->movzx(out, memAt(base, offset, 1));
        }
        gpValues.emplace(value, out);
        return;
    }
    case ValueKind::CallInst:
        emitCall(llvm::cast<CallInst>(value));
        return;
    case ValueKind::ReturnInst:
        emitReturn(llvm::cast<ReturnInst>(value));
        return;
    case ValueKind::BranchInst: {
        auto* inst = llvm::cast<BranchInst>(value);
        if (inst->destination->parameter) {
            auto it = gpValues.find(inst->destination->parameter);
            if (it != gpValues.end())
                cc->mov(it->second, getGp(inst->argument));
            else if (isDoubleType(inst->destination->parameter->type))
                cc->movsd(vecValues.at(inst->destination->parameter), getVec(inst->argument));
            else
                cc->movss(vecValues.at(inst->destination->parameter), getVec(inst->argument));
        }
        cc->jmp(blockLabels.at(inst->destination));
        return;
    }
    case ValueKind::CondBranchInst: {
        auto* inst = llvm::cast<CondBranchInst>(value);
        Gp cond = getGp(inst->condition);
        auto moveArg = [&](const BasicBlock* succ) {
            if (!succ->parameter || !inst->argument) return;
            auto it = gpValues.find(succ->parameter);
            if (it != gpValues.end())
                cc->mov(it->second, getGp(inst->argument));
            else if (isDoubleType(succ->parameter->type))
                cc->movsd(vecValues.at(succ->parameter), getVec(inst->argument));
            else
                cc->movss(vecValues.at(succ->parameter), getVec(inst->argument));
        };
        cc->test(cond, cond);
        if (inst->trueBlock->parameter) {
            // The true-edge copy needs its own home; block labels bind once
            // at emission, so route through a trampoline.
            Label tramp = cc->new_label();
            cc->jnz(tramp);
            moveArg(inst->falseBlock);
            cc->jmp(blockLabels.at(inst->falseBlock));
            cc->bind(tramp);
            moveArg(inst->trueBlock);
            cc->jmp(blockLabels.at(inst->trueBlock));
            return;
        }
        cc->jnz(blockLabels.at(inst->trueBlock));
        moveArg(inst->falseBlock);
        cc->jmp(blockLabels.at(inst->falseBlock));
        return;
    }
    case ValueKind::SwitchInst: {
        auto* inst = llvm::cast<SwitchInst>(value);
        Gp cond = getGp(inst->condition);
        bool is64 = cond.size() == 8;
        // Switch carries no argument, so targets never have parameters (the
        // LLVM backend asserts the same when building PHIs).
        for (auto& [caseValue, target] : inst->cases) {
            auto* constant = llvm::cast<ConstantInt>(caseValue);
            // Extend per the condition type to match the canonical form.
            IRType* condType = inst->condition->getType();
            int width = condType->isInteger() ? getIntegerBitWidth(condType) : 32;
            uint64_t bits = constant->value.extOrTrunc((unsigned)(width > 0 ? width : 32)).getZExtValue();
            int64_t key = (int64_t)bits;
            if (condType->isSignedInteger() && width < 64 && (bits & (1ULL << (width - 1)))) key |= (int64_t)(~0ULL << width);
            if (is64) {
                Gp tmp = cc->new_gp64();
                cc->mov(tmp, key);
                cc->cmp(cond, tmp);
            } else {
                Gp tmp = cc->new_gp32();
                cc->mov(tmp, (int32_t)key);
                cc->cmp(cond, tmp);
            }
            cc->je(blockLabels.at(target));
        }
        cc->jmp(blockLabels.at(inst->defaultBlock));
        return;
    }
    case ValueKind::UnreachableInst:
        cc->int3();
        return;
    case ValueKind::CheckedArithInst:
        emitCheckedArith(llvm::cast<CheckedArithInst>(value));
        return;
    case ValueKind::ArithOverflowInst: {
        auto* inst = llvm::cast<ArithOverflowInst>(value);
        getGp(inst->checked); // Emit the arithmetic first; the flag is stashed.
        gpValues.emplace(value, checkedOverflow.at(inst->checked));
        return;
    }
    case ValueKind::SaturatingArithInst: {
        auto* inst = llvm::cast<SaturatingArithInst>(value);
        IRType* type = inst->left->getType();
        ASSERT(type->isInteger());
        int width = getIntegerBitWidth(type);
        bool isSigned = type->isSignedInteger();
        bool is64 = width > 32;
        Gp left = getGp(inst->left), right = getGp(inst->right);
        Gp out = is64 ? cc->new_gp64() : cc->new_gp32();
        if (width < 32) {
            // Narrow operands can't overflow 32 bits, so clamp the full
            // 32-bit result against the operand width instead of flags.
            Gp full = cc->new_gp32();
            cc->mov(full, left);
            if (inst->op == Token::Plus)
                cc->add(full, right);
            else
                cc->sub(full, right);
            if (!isSigned) {
                int64_t max = (1LL << width) - 1;
                if (inst->op == Token::Plus) {
                    Gp maxReg = cc->new_gp32();
                    cc->mov(maxReg, (int32_t)max);
                    cc->mov(out, full);
                    cc->cmp(full, maxReg);
                    cc->cmova(out, maxReg);
                } else {
                    // Borrow exactly when left < right.
                    cc->mov(out, full);
                    Gp zero = cc->new_gp32();
                    cc->mov(zero, 0);
                    cc->cmp(left, right);
                    cc->cmovb(out, zero);
                }
            } else {
                int64_t lo = -(1LL << (width - 1));
                int64_t hi = (1LL << (width - 1)) - 1;
                Gp loReg = cc->new_gp32();
                Gp hiReg = cc->new_gp32();
                cc->mov(loReg, (int32_t)lo);
                cc->mov(hiReg, (int32_t)hi);
                Gp tmp = cc->new_gp32();
                cc->mov(tmp, full);
                cc->cmp(full, hiReg);
                cc->cmovg(tmp, hiReg);
                cc->mov(out, tmp);
                cc->cmp(full, loReg);
                cc->cmovl(out, loReg);
            }
            canonicalize(out, type);
            gpValues.emplace(value, out);
            return;
        }
        Gp wrapped = is64 ? cc->new_gp64() : cc->new_gp32();
        cc->mov(wrapped, left);
        if (inst->op == Token::Plus)
            cc->add(wrapped, right);
        else
            cc->sub(wrapped, right);
        if (!isSigned) {
            Gp clamp = is64 ? cc->new_gp64() : cc->new_gp32();
            cc->mov(clamp, inst->op == Token::Minus ? 0 : -1);
            // Unsigned add overflows on carry, sub on borrow (x86 CF is the
            // borrow flag, unlike AArch64's inverted carry).
            cc->mov(out, wrapped);
            cc->cmovc(out, clamp);
        } else {
            Label sat = cc->new_label();
            Label done = cc->new_label();
            cc->jo(sat);
            cc->mov(out, wrapped);
            cc->jmp(done);
            cc->bind(sat);
            Gp lo = is64 ? cc->new_gp64() : cc->new_gp32();
            Gp hi = is64 ? cc->new_gp64() : cc->new_gp32();
            if (is64) {
                cc->mov(lo, INT64_MIN);
                cc->mov(hi, INT64_MAX);
            } else {
                cc->mov(lo, INT32_MIN);
                cc->mov(hi, INT32_MAX);
            }
            cc->mov(out, hi);
            cc->cmp(left, 0);
            cc->cmovl(out, lo);
            cc->bind(done);
        }
        canonicalize(out, type);
        gpValues.emplace(value, out);
        return;
    }
    case ValueKind::ArrayOpInst:
        emitArrayOp(llvm::cast<ArrayOpInst>(value));
        return;
    case ValueKind::SizeofInst: {
        auto* inst = llvm::cast<SizeofInst>(value);
        uint64_t size = typeSize(inst->type);
        IRType* resultType = inst->getType();
        if (isFloat80Type(resultType)) {
            Gp home = homeAddr(16, 16);
            Gp tmp = cc->new_gp64();
            cc->mov(tmp, size);
            callF80FromWord(F80Op::FromU64, home, tmp, /*is64=*/true);
            gpValues.emplace(value, home);
            return;
        }
        if (resultType->isFloatingPoint()) {
            Vec out = isDoubleType(resultType) ? cc->new_xmm_sd() : cc->new_xmm_ss();
            Gp tmp = cc->new_gp64();
            cc->mov(tmp, size);
            if (isDoubleType(resultType))
                cc->cvtsi2sd(out, tmp);
            else
                cc->cvtsi2ss(out, tmp);
            vecValues.emplace(value, out);
            return;
        }
        int width = getIntegerBitWidth(resultType);
        if (width > 32) {
            Gp out = cc->new_gp64();
            cc->mov(out, size);
            gpValues.emplace(value, out);
        } else {
            Gp out = cc->new_gp32();
            cc->mov(out, (int32_t)(uint32_t)size);
            gpValues.emplace(value, out);
        }
        return;
    }
    case ValueKind::BasicBlock:
        return; // Labels are bound during body emission.
    default:
        break;
    }
    ABORT("unhandled value kind in AsmJit backend");
}

void X64AsmJitGenerator::emitReturn(const ReturnInst* inst) {
    if (currentRetClass.kind == AbiClass::Kind::Indirect) {
        if (inst->value) emitMemcpy(currentSret, getGp(inst->value), typeSize(currentRetType));
        // Win64 callees return the sret pointer in RAX.
        cc->mov(x86::rax, currentSret);
        cc->ret();
        return;
    }
    if (!inst->value || currentRetClass.kind == AbiClass::Kind::Empty) {
        cc->ret();
        return;
    }
    if (currentRetClass.kind == AbiClass::Kind::Direct) {
        if (currentRetType->isFloatingPoint()) {
            Vec v = getVec(inst->value);
            cc->ret(v);
        } else {
            Gp r = getGp(inst->value);
            cc->ret(r);
        }
        return;
    }
    ASSERT(currentRetClass.kind == AbiClass::Kind::Chunk);
    Gp home = getGp(inst->value);
    if (!currentRetClass.chunkIs64) {
        Gp chunk = cc->new_gp32();
        cc->mov(chunk, memAt(home, 0, 4));
        cc->ret(chunk);
    } else {
        Gp chunk = cc->new_gp64();
        cc->mov(chunk, memAt(home, 0, 8));
        cc->ret(chunk);
    }
}

void X64AsmJitGenerator::emitCheckedArith(const CheckedArithInst* inst) {
    IRType* type = inst->left->getType();
    ASSERT(type->isInteger());
    int width = getIntegerBitWidth(type);
    bool isSigned = type->isSignedInteger();
    Gp left = getGp(inst->left), right = getGp(inst->right);
    Gp out = width > 32 ? cc->new_gp64() : cc->new_gp32();
    Gp overflow = cc->new_gp32();
    cc->xor_(overflow, overflow);
    if (width < 32 && inst->op != Token::Star) {
        // Narrow operands can't overflow 32 bits; check the full result
        // against the operand width instead of 32-bit flags.
        Gp full = cc->new_gp32();
        cc->mov(full, left);
        if (inst->op == Token::Plus)
            cc->add(full, right);
        else
            cc->sub(full, right);
        if (!isSigned) {
            if (inst->op == Token::Plus) {
                Gp maxReg = cc->new_gp32();
                cc->mov(maxReg, (int32_t)((1LL << width) - 1));
                cc->cmp(full, maxReg);
                cc->seta(overflow.r8());
            } else {
                cc->cmp(left, right);
                cc->setb(overflow.r8());
            }
        } else {
            Gp loReg = cc->new_gp32();
            Gp hiReg = cc->new_gp32();
            cc->mov(loReg, (int32_t)(-(1LL << (width - 1))));
            cc->mov(hiReg, (int32_t)((1LL << (width - 1)) - 1));
            cc->cmp(full, hiReg);
            cc->setg(overflow.r8());
            Label done = cc->new_label();
            cc->jg(done);
            cc->cmp(full, loReg);
            cc->setl(overflow.r8());
            cc->bind(done);
        }
        cc->mov(out, full);
    } else if (inst->op == Token::Plus) {
        cc->mov(out, left);
        cc->add(out, right);
        if (isSigned)
            cc->seto(overflow.r8());
        else
            cc->setc(overflow.r8());
    } else if (inst->op == Token::Minus) {
        cc->mov(out, left);
        cc->sub(out, right);
        if (isSigned)
            cc->seto(overflow.r8());
        else
            cc->setc(overflow.r8());
    } else {
        ASSERT(inst->op == Token::Star);
        if (width == 64) {
            if (isSigned) {
                cc->mov(out, left);
                cc->imul(out, right);
                cc->seto(overflow.r8());
            } else {
                // mul produces the full 128-bit product; a nonzero high
                // half means the result doesn't fit.
                Gp lo = cc->new_gp64();
                Gp hi = cc->new_gp64();
                cc->mov(lo, left);
                cc->mul(hi, lo, right);
                cc->test(hi, hi);
                cc->setnz(overflow.r8());
                cc->mov(out, lo);
            }
        } else if (width == 32) {
            if (isSigned) {
                cc->mov(out, left);
                cc->imul(out, right);
                cc->seto(overflow.r8());
            } else {
                Gp lo = cc->new_gp32();
                Gp hi = cc->new_gp32();
                cc->mov(lo, left);
                cc->mul(hi, lo, right);
                cc->test(hi, hi);
                cc->setnz(overflow.r8());
                cc->mov(out, lo);
            }
        } else {
            // Narrow operands can't overflow 32 bits; check the truncation
            // to the operand width instead.
            Gp full = cc->new_gp32();
            cc->mov(full, left);
            cc->imul(full, right);
            Gp trunc = cc->new_gp32();
            cc->mov(trunc, full);
            canonicalize(trunc, type);
            cc->cmp(full, trunc);
            cc->setne(overflow.r8());
            cc->mov(out, trunc);
        }
    }
    canonicalize(out, type);
    gpValues.emplace(inst, out);
    checkedOverflow.emplace(inst, overflow);
}

void X64AsmJitGenerator::emitArrayOp(const ArrayOpInst* inst) {
    auto* arrayType = llvm::cast<IRArrayType>(inst->arrayType);
    int size = arrayType->size;
    IRType* elemType = arrayType->elementType;
    bool isComparison = inst->op == Token::Equal || inst->op == Token::NotEqual;
    bool foldAnd = inst->op == Token::Equal;
    bool isF80 = isFloat80Type(elemType);
    bool isFloat = elemType->isFloatingPoint() && !isF80;
    bool isDouble = isFloat && isDoubleType(elemType);
    auto isArraySide = [](const Value* side) { return side->getType()->isPointerType() && side->getType()->getPointee()->isArrayType(); };
    bool leftIsArray = isArraySide(inst->left);
    bool rightIsArray = isArraySide(inst->right);
    ASSERT(leftIsArray || rightIsArray);
    Gp lhsPtr, rhsPtr;
    if (leftIsArray) lhsPtr = getGp(inst->left);
    if (rightIsArray) rhsPtr = getGp(inst->right);

    if (size == 0) {
        if (isComparison) {
            Gp out = cc->new_gp32();
            cc->mov(out, foldAnd ? 1 : 0);
            gpValues.emplace(inst, out);
        } else {
            gpValues.emplace(inst, homeAddr(typeSize(inst->arrayType), typeAlign(inst->arrayType)));
        }
        return;
    }

    uint64_t elemSize = typeSize(elemType);
    uint64_t count = (uint64_t)size;
    Gp resultHome;
    Gp acc;
    if (isComparison) {
        acc = cc->new_gp32();
        cc->mov(acc, foldAnd ? 1 : 0);
    } else {
        resultHome = homeAddr(typeSize(inst->arrayType), typeAlign(inst->arrayType));
    }

    // Counted loop over the elements; fixed arrays can be large, so unrolling
    // risks code bloat. Byte offset plus base avoids index scaling.
    Gp lhs = cc->new_gp64();
    Gp rhs = cc->new_gp64();
    Gp dst = cc->new_gp64();
    if (leftIsArray) cc->mov(lhs, lhsPtr);
    if (rightIsArray) cc->mov(rhs, rhsPtr);
    if (!isComparison) cc->mov(dst, resultHome);
    Gp bound = cc->new_gp64();
    cc->mov(bound, count * elemSize);
    Gp off = cc->new_gp64();
    cc->mov(off, 0);
    auto elemAddr = [&](Gp base) {
        Gp addr = cc->new_gp64();
        cc->mov(addr, base);
        cc->add(addr, off);
        return addr;
    };
    Label cond = cc->new_label();
    Label body = cc->new_label();
    cc->jmp(cond);
    cc->bind(body);
    if (isF80) {
        Gp lPtr = leftIsArray ? elemAddr(lhs) : getGp(inst->left);
        Gp rPtr = rightIsArray ? elemAddr(rhs) : getGp(inst->right);
        if (isComparison) {
            Gp bit = emitF80Compare(inst->op, lPtr, rPtr);
            if (foldAnd)
                cc->and_(acc, bit);
            else
                cc->or_(acc, bit);
        } else {
            Gp elem = emitF80Binary(inst->op, lPtr, rPtr);
            emitMemcpy(elemAddr(dst), elem, 16);
        }
    } else {
        auto loadElem = [&](Gp ptr) -> std::pair<Gp, Vec> {
            Gp addr = elemAddr(ptr);
            if (isFloat) {
                Vec v = isDouble ? cc->new_xmm_sd() : cc->new_xmm_ss();
                if (isDouble)
                    cc->movsd(v, x86::qword_ptr(addr));
                else
                    cc->movss(v, x86::dword_ptr(addr));
                return {Gp(), v};
            }
            Gp g = elemSize > 4 ? cc->new_gp64() : cc->new_gp32();
            if (elemSize == 8) {
                cc->mov(g, x86::qword_ptr(addr));
            } else if (elemSize == 4) {
                cc->mov(g, x86::dword_ptr(addr));
            } else if (elemSize == 2) {
                if (elemType->isSignedInteger())
                    cc->movsx(g, x86::word_ptr(addr));
                else
                    cc->movzx(g, x86::word_ptr(addr));
            } else {
                if (elemType->isSignedInteger())
                    cc->movsx(g, x86::byte_ptr(addr));
                else
                    cc->movzx(g, x86::byte_ptr(addr));
            }
            return {g, Vec()};
        };
        Gp lGp, rGp;
        Vec lVec, rVec;
        if (leftIsArray) {
            auto [g, v] = loadElem(lhs);
            lGp = g;
            lVec = v;
        } else if (isFloat) {
            lVec = getVec(inst->left);
        } else {
            lGp = getGp(inst->left);
        }
        if (rightIsArray) {
            auto [g, v] = loadElem(rhs);
            rGp = g;
            rVec = v;
        } else if (isFloat) {
            rVec = getVec(inst->right);
        } else {
            rGp = getGp(inst->right);
        }
        if (isComparison) {
            Gp bit = isFloat ? emitFloatCompare(inst->op, lVec, rVec, isDouble) : emitIntCompare(inst->op, lGp, rGp, elemType);
            if (foldAnd)
                cc->and_(acc, bit);
            else
                cc->or_(acc, bit);
        } else if (isFloat) {
            Vec elem = emitFloatBinary(inst->op, lVec, rVec, isDouble);
            Gp addr = elemAddr(dst);
            if (isDouble)
                cc->movsd(x86::qword_ptr(addr), elem);
            else
                cc->movss(x86::dword_ptr(addr), elem);
        } else {
            Gp elem = emitIntBinary(inst->op, lGp, rGp, elemType, elemType);
            Gp addr = elemAddr(dst);
            if (elemSize == 8) {
                cc->mov(x86::qword_ptr(addr), elem);
            } else if (elemSize == 4) {
                cc->mov(x86::dword_ptr(addr), elem);
            } else if (elemSize == 2) {
                cc->mov(x86::word_ptr(addr), elem.r16());
            } else {
                cc->mov(x86::byte_ptr(addr), elem.r8());
            }
        }
    }
    if (elemSize <= (uint64_t)INT32_MAX) {
        cc->add(off, (int64_t)elemSize);
    } else {
        Gp step = cc->new_gp64();
        cc->mov(step, elemSize);
        cc->add(off, step);
    }
    cc->bind(cond);
    cc->cmp(off, bound);
    cc->jb(body);
    if (isComparison)
        gpValues.emplace(inst, acc);
    else
        gpValues.emplace(inst, resultHome);
}

// Blocks reachable from the entry via terminator edges. Dead blocks (join
// blocks after early returns, ...) are skipped entirely: AsmJit's RA pass
// crashes on blocks no CFG edge reaches.
std::unordered_set<const BasicBlock*> reachableBlocks(const Function* function) {
    std::unordered_set<const BasicBlock*> reachable;
    if (function->body.empty()) return reachable;
    std::unordered_map<const BasicBlock*, const BasicBlock*> nextInLayout;
    for (size_t i = 0; i + 1 < function->body.size(); ++i)
        nextInLayout[function->body[i]] = function->body[i + 1];
    std::vector<const BasicBlock*> worklist = {function->body.front()};
    while (!worklist.empty()) {
        const BasicBlock* block = worklist.back();
        worklist.pop_back();
        if (!reachable.insert(block).second) continue;
        auto visit = [&](const BasicBlock* succ) {
            if (succ && !reachable.count(succ)) worklist.push_back(succ);
        };
        if (block->body.empty()) continue;
        const Instruction* term = block->body.back();
        switch (term->kind) {
        case ValueKind::BranchInst:
            visit(llvm::cast<BranchInst>(term)->destination);
            break;
        case ValueKind::CondBranchInst: {
            auto* inst = llvm::cast<CondBranchInst>(term);
            visit(inst->trueBlock);
            visit(inst->falseBlock);
            break;
        }
        case ValueKind::SwitchInst: {
            auto* inst = llvm::cast<SwitchInst>(term);
            for (auto& [caseValue, target] : inst->cases)
                visit(target);
            visit(inst->defaultBlock);
            break;
        }
        default:
            // Return/unreachable end control flow; anything else falls through.
            if (term->kind != ValueKind::ReturnInst && term->kind != ValueKind::UnreachableInst) {
                auto it = nextInLayout.find(block);
                if (it != nextInLayout.end()) visit(it->second);
            }
            break;
        }
    }
    return reachable;
}

void* X64AsmJitGenerator::codegenFunction(const Function* function) {
    if (std::getenv("CX_ASMJIT_TRACE")) llvm::errs() << "; codegen " << function->mangledName << "\n";
    CodeHolder code;
    StringLogger logger;
    struct LogHandler : ErrorHandler {
        void handle_error(Error err, const char* message, BaseEmitter*) override {
            llvm::errs() << "asmjit error " << (unsigned)err << ": " << message << "\n";
        }
    } handler;
    if (std::getenv("CX_ASMJIT_LOG")) {
        code.set_logger(&logger);
        code.set_error_handler(&handler);
    }
    [[maybe_unused]] Error err = code.init(runtime.environment(), runtime.cpu_features());
    ASSERT(err == Error::kOk);
    x86::Compiler compiler(&code);
    cc = &compiler;
    gpValues.clear();
    vecValues.clear();
    blockLabels.clear();
    checkedOverflow.clear();
    scratch0 = cc->new_gp64();
    scratch1 = cc->new_gp64();

    // Signature.
    FuncSignature sig;
    currentRetType = function->returnType;
    currentRetClass = function->returnType->isVoid() ? AbiClass{AbiClass::Kind::Direct, TypeId::kVoid} : classifyType(function->returnType);
    if (currentRetClass.kind == AbiClass::Kind::Indirect) {
        sig.add_arg(TypeId::kUInt64);
    } else if (currentRetClass.kind == AbiClass::Kind::Chunk) {
        sig.set_ret(currentRetClass.chunkIs64 ? TypeId::kUInt64 : TypeId::kUInt32);
    } else if (currentRetClass.kind != AbiClass::Kind::Empty) {
        sig.set_ret(currentRetClass.typeId);
    }
    struct ParamSlots {
        const Parameter* param;
        AbiClass cls;
        bool decayedArray = false;
    };
    std::vector<ParamSlots> params;
    for (auto& param : function->params) {
        if (function->declaredExternC && param.type->isArrayType()) {
            params.push_back({&param, {AbiClass::Kind::Direct, TypeId::kUInt64}, true});
            sig.add_arg(TypeId::kUInt64);
            continue;
        }
        AbiClass cls = classifyType(param.type);
        params.push_back({&param, cls});
        if (cls.kind == AbiClass::Kind::Empty) continue;
        if (cls.kind == AbiClass::Kind::Direct || cls.kind == AbiClass::Kind::Indirect) {
            sig.add_arg(cls.kind == AbiClass::Kind::Indirect ? TypeId::kUInt64 : cls.typeId);
        } else if (cls.kind == AbiClass::Kind::Chunk) {
            sig.add_arg(cls.chunkIs64 ? TypeId::kUInt64 : TypeId::kUInt32);
        }
    }
    if (function->isVariadic) sig.set_va_index(sig.arg_count());

    FuncNode* funcNode = cc->add_func(sig);

    unsigned argNo = 0;
    if (currentRetClass.kind == AbiClass::Kind::Indirect) {
        currentSret = cc->new_gp64();
        funcNode->set_arg(argNo++, currentSret);
    }
    for (auto& [param, cls, decayed] : params) {
        if (cls.kind == AbiClass::Kind::Empty) {
            gpValues.emplace(param, homeAddr(1, 1));
            continue;
        }
        if (decayed || cls.kind == AbiClass::Kind::Indirect) {
            Gp ptr = cc->new_gp64();
            funcNode->set_arg(argNo++, ptr);
            gpValues.emplace(param, ptr);
            continue;
        }
        if (cls.kind == AbiClass::Kind::Direct) {
            if (param->type->isFloatingPoint()) {
                Vec v = isDoubleType(param->type) ? cc->new_xmm_sd() : cc->new_xmm_ss();
                funcNode->set_arg(argNo++, v);
                vecValues.emplace(param, v);
            } else {
                Gp r = newIntReg(param->type);
                funcNode->set_arg(argNo++, r);
                gpValues.emplace(param, r);
            }
            continue;
        }
        // Chunks materialize into a home; aggregates live in memory.
        ASSERT(cls.kind == AbiClass::Kind::Chunk);
        uint64_t size = typeSize(param->type);
        Gp home = homeAddr(size, typeAlign(param->type));
        if (!cls.chunkIs64) {
            Gp chunk = cc->new_gp32();
            funcNode->set_arg(argNo++, chunk);
            cc->mov(memAt(home, 0, 4), chunk);
        } else {
            Gp chunk = cc->new_gp64();
            funcNode->set_arg(argNo++, chunk);
            cc->mov(memAt(home, 0, 8), chunk);
        }
        gpValues.emplace(param, home);
    }

    std::unordered_set<const BasicBlock*> live = reachableBlocks(function);

    // Block labels and parameters up front: branches reference both.
    for (auto* block : function->body) {
        if (!live.count(block)) continue;
        blockLabels.emplace(block, cc->new_label());
        if (block->parameter) {
            IRType* type = block->parameter->type;
            if (type->isFloatingPoint() && !isFloat80Type(type))
                vecValues.emplace(block->parameter, isDoubleType(type) ? cc->new_xmm_sd() : cc->new_xmm_ss());
            else if (livesInMemory(type))
                gpValues.emplace(block->parameter, cc->new_gp64());
            else
                gpValues.emplace(block->parameter, newIntReg(type));
        }
    }

    // Hoist allocas: their address registers must dominate every use, and lazy
    // materialization at first use would not. (Constants and other positionless
    // values rematerialize per use instead; see getGp.)
    for (auto* block : function->body) {
        if (!live.count(block)) continue;
        for (auto* inst : block->body)
            if (inst->kind == ValueKind::AllocaInst) codegenInst(inst);
    }

    for (auto* block : function->body) {
        if (!live.count(block)) continue;
        cc->bind(blockLabels.at(block));
        for (auto* inst : block->body)
            codegenInst(inst);
    }
    cc->end_func();
    err = cc->finalize();
    if (std::getenv("CX_ASMJIT_LOG")) llvm::errs() << "; function " << function->mangledName << "\n" << logger.data() << "\n";
    if (err != Error::kOk) {
        ABORT("AsmJit finalize failed for '" << function->name << "' (error " << (unsigned)err << ")");
    }
    void* address = nullptr;
    err = runtime.add(&address, &code);
    if (err != Error::kOk || !address) ABORT("AsmJit runtime.add failed for '" << function->name << "'");
    cc = nullptr;
    return address;
}

const llvm::fltSemantics& fpSemantics(IRType* type) {
    if (isFloat80Type(type)) return llvm::APFloat::x87DoubleExtended();
    if (isDoubleType(type)) return llvm::APFloat::IEEEdouble();
    return llvm::APFloat::IEEEsingle();
}

llvm::APFloat X64AsmJitGenerator::evalConstFloat(const Value* value) {
    switch (value->kind) {
    case ValueKind::ConstantFP:
        return llvm::cast<ConstantFP>(value)->value;
    case ValueKind::BinaryInst: {
        auto* inst = llvm::cast<BinaryInst>(value);
        llvm::APFloat left = evalConstFloat(inst->left);
        llvm::APFloat right = evalConstFloat(inst->right);
        bool ignored = false;
        // Operands share the result semantics; convert defensively.
        left.convert(fpSemantics(inst->getType()), llvm::APFloat::rmNearestTiesToEven, &ignored);
        right.convert(left.getSemantics(), llvm::APFloat::rmNearestTiesToEven, &ignored);
        switch (inst->op) {
        case Token::Plus:
            left.add(right, llvm::APFloat::rmNearestTiesToEven);
            return left;
        case Token::Minus:
            left.subtract(right, llvm::APFloat::rmNearestTiesToEven);
            return left;
        case Token::Star:
            left.multiply(right, llvm::APFloat::rmNearestTiesToEven);
            return left;
        case Token::Slash:
            left.divide(right, llvm::APFloat::rmNearestTiesToEven);
            return left;
        default:
            llvm_unreachable("unexpected const float operator");
        }
    }
    case ValueKind::UnaryInst: {
        auto* inst = llvm::cast<UnaryInst>(value);
        llvm::APFloat operand = evalConstFloat(inst->operand);
        if (inst->op == Token::Minus) operand.changeSign();
        if (inst->op == Token::Plus || inst->op == Token::Minus) return operand;
        llvm_unreachable("unexpected const float operator");
    }
    case ValueKind::CastInst: {
        auto* inst = llvm::cast<CastInst>(value);
        bool ignored = false;
        if (inst->value->getType()->isFloatingPoint()) {
            llvm::APFloat operand = evalConstFloat(inst->value);
            operand.convert(fpSemantics(inst->type), llvm::APFloat::rmNearestTiesToEven, &ignored);
            return operand;
        }
        llvm::APFloat result = llvm::APFloat::getZero(fpSemantics(inst->type));
        bool isSigned = inst->value->getType()->isSignedInteger();
        llvm::APInt bits(64, evalConstInt(inst->value));
        result.convertFromAPInt(bits, isSigned, llvm::APFloat::rmNearestTiesToEven);
        return result;
    }
    default:
        llvm_unreachable("not a constant float expression");
    }
}

uint64_t X64AsmJitGenerator::evalConstInt(const Value* value) {
    switch (value->kind) {
    case ValueKind::ConstantInt: {
        auto* inst = llvm::cast<ConstantInt>(value);
        int width = getIntegerBitWidth(inst->type);
        if (width <= 0) width = 64;
        uint64_t bits = inst->value.extOrTrunc((unsigned)width).getZExtValue();
        if (inst->type->isSignedInteger() && width < 64 && (bits & (1ULL << (width - 1)))) bits |= ~0ULL << width;
        return bits;
    }
    case ValueKind::ConstantBool:
        return llvm::cast<ConstantBool>(value)->value ? 1 : 0;
    case ValueKind::ConstantNull:
        return 0;
    case ValueKind::SizeofInst:
        return typeSize(llvm::cast<SizeofInst>(value)->type);
    case ValueKind::CastInst: {
        auto* inst = llvm::cast<CastInst>(value);
        IRType* sourceType = inst->value->getType();
        if (sourceType->isFloatingPoint()) {
            llvm::APFloat f = evalConstFloat(inst->value);
            bool isSigned = inst->type->isSignedInteger();
            bool ignored = false;
            llvm::APSInt result(64, !isSigned);
            f.convertToInteger(result, llvm::APFloat::rmTowardZero, &ignored);
            if (inst->type->isBool()) return result.isZero() ? 0 : 1;
            return result.getZExtValue();
        }
        uint64_t bits = evalConstInt(inst->value);
        int width = getIntegerBitWidth(inst->type);
        if (width <= 0 || width >= 64) return bits;
        uint64_t mask = (1ULL << width) - 1;
        if (inst->type->isSignedInteger() && (bits & (1ULL << (width - 1)))) return bits | ~mask;
        return bits & mask;
    }
    case ValueKind::BinaryInst: {
        auto* inst = llvm::cast<BinaryInst>(value);
        if (inst->left->getType()->isFloatingPoint()) {
            llvm::APFloat left = evalConstFloat(inst->left), right = evalConstFloat(inst->right);
            auto cmp = left.compare(right);
            switch (inst->op) {
            case Token::Equal:
                return cmp == llvm::APFloat::cmpEqual;
            case Token::NotEqual:
                return cmp != llvm::APFloat::cmpEqual;
            case Token::Less:
                return cmp == llvm::APFloat::cmpLessThan;
            case Token::LessOrEqual:
                return cmp == llvm::APFloat::cmpLessThan || cmp == llvm::APFloat::cmpEqual;
            case Token::Greater:
                return cmp == llvm::APFloat::cmpGreaterThan;
            case Token::GreaterOrEqual:
                return cmp == llvm::APFloat::cmpGreaterThan || cmp == llvm::APFloat::cmpEqual;
            default:
                llvm_unreachable("float arithmetic in integer const eval");
            }
        }
        uint64_t left = evalConstInt(inst->left), right = evalConstInt(inst->right);
        int width = getIntegerBitWidth(inst->getType());
        if (width <= 0) width = 64;
        uint64_t mask = width == 64 ? ~0ULL : ((1ULL << width) - 1);
        uint64_t result;
        switch (inst->op) {
        case Token::Plus:
            result = left + right;
            break;
        case Token::Minus:
            result = left - right;
            break;
        case Token::Star:
            result = left * right;
            break;
        case Token::Slash:
            ASSERT(right != 0);
            result = inst->left->getType()->isSignedInteger() ? (uint64_t)((int64_t)left / (int64_t)right) : left / right;
            break;
        case Token::Modulo:
            ASSERT(right != 0);
            result = inst->left->getType()->isSignedInteger() ? (uint64_t)((int64_t)left % (int64_t)right) : left % right;
            break;
        case Token::And:
            result = left & right;
            break;
        case Token::Or:
            result = left | right;
            break;
        case Token::Xor:
            result = left ^ right;
            break;
        case Token::LeftShift:
            result = left << (right & 63);
            break;
        case Token::RightShift:
            result = inst->left->getType()->isSignedInteger() ? (uint64_t)((int64_t)left >> (right & 63)) : left >> (right & 63);
            break;
        case Token::Equal:
            result = left == right;
            break;
        case Token::NotEqual:
            result = left != right;
            break;
        case Token::Less:
            result = inst->left->getType()->isSignedInteger() ? (int64_t)left < (int64_t)right : left < right;
            break;
        case Token::LessOrEqual:
            result = inst->left->getType()->isSignedInteger() ? (int64_t)left <= (int64_t)right : left <= right;
            break;
        case Token::Greater:
            result = inst->left->getType()->isSignedInteger() ? (int64_t)left > (int64_t)right : left > right;
            break;
        case Token::GreaterOrEqual:
            result = inst->left->getType()->isSignedInteger() ? (int64_t)left >= (int64_t)right : left >= right;
            break;
        default:
            llvm_unreachable("unexpected const binary operator");
        }
        return result & mask;
    }
    case ValueKind::UnaryInst: {
        auto* inst = llvm::cast<UnaryInst>(value);
        uint64_t operand = evalConstInt(inst->operand);
        int width = getIntegerBitWidth(inst->getType());
        if (width <= 0) width = 64;
        uint64_t mask = width == 64 ? ~0ULL : ((1ULL << width) - 1);
        switch (inst->op) {
        case Token::Plus:
            return operand & mask;
        case Token::Minus:
            return (0ULL - operand) & mask;
        case Token::Tilde:
            if (inst->operand->getType()->isBool()) return operand ? 0 : 1;
            return ~operand & mask;
        case Token::Not:
            return operand == 0 ? 1 : 0;
        default:
            llvm_unreachable("unexpected const unary operator");
        }
    }
    default:
        llvm_unreachable("not a constant integer expression");
    }
}

void X64AsmJitGenerator::storeConstToAddr(const Value* value, char* addr) {
    switch (value->kind) {
    case ValueKind::ConstantInt:
    case ValueKind::ConstantBool:
    case ValueKind::ConstantNull:
    case ValueKind::SizeofInst:
    case ValueKind::BinaryInst:
    case ValueKind::UnaryInst:
    case ValueKind::CastInst: {
        if (value->getType()->isFloatingPoint()) {
            llvm::APFloat f = evalConstFloat(value);
            if (isFloat80Type(value->getType())) {
                llvm::APInt apBits = f.bitcastToAPInt();
                ASSERT(apBits.getBitWidth() == 80);
                memcpy(addr, apBits.getRawData(), 10);
            } else if (isDoubleType(value->getType())) {
                double d = f.convertToDouble();
                memcpy(addr, &d, 8);
            } else {
                float s = f.convertToFloat();
                memcpy(addr, &s, 4);
            }
            return;
        }
        uint64_t v = evalConstInt(value);
        uint64_t size = typeSize(value->getType());
        memcpy(addr, &v, size == 0 ? 0 : std::min<uint64_t>(size, 8));
        return;
    }
    case ValueKind::ConstantFP: {
        auto* inst = llvm::cast<ConstantFP>(value);
        if (isFloat80Type(inst->type)) {
            llvm::APInt apBits = inst->value.bitcastToAPInt();
            ASSERT(apBits.getBitWidth() == 80);
            memcpy(addr, apBits.getRawData(), 10);
        } else if (isDoubleType(inst->type)) {
            uint64_t bits = inst->value.bitcastToAPInt().getZExtValue();
            memcpy(addr, &bits, 8);
        } else {
            uint32_t bits = (uint32_t)inst->value.bitcastToAPInt().getZExtValue();
            memcpy(addr, &bits, 4);
        }
        return;
    }
    case ValueKind::ConstantString: {
        void* str = getStringAddr(llvm::cast<ConstantString>(value)->value);
        memcpy(addr, &str, 8);
        return;
    }
    case ValueKind::Function: {
        void* fn = funcTable[funcIndex.at(llvm::cast<Function>(value))];
        ASSERT(fn);
        memcpy(addr, &fn, 8);
        return;
    }
    case ValueKind::GlobalVariable: {
        void* g = getGlobalAddr(llvm::cast<GlobalVariable>(value));
        memcpy(addr, &g, 8);
        return;
    }
    case ValueKind::Undefined:
        return; // Homes start zeroed; padding stays zero.
    case ValueKind::InsertInst: {
        auto* insert = llvm::cast<InsertInst>(value);
        IRType* aggregateType = insert->aggregate->getType();
        int count = aggregateType->isArrayType() ? aggregateType->getArraySize() : (int)aggregateType->getFields().size();
        std::vector<const Value*> elements(count, nullptr);
        for (auto* current = insert; current; current = llvm::dyn_cast<InsertInst>(current->aggregate)) {
            if (!elements[current->index]) elements[current->index] = current->value;
        }
        for (int i = 0; i < count; ++i) {
            if (!elements[i]) continue;
            uint64_t offset;
            if (aggregateType->isStruct())
                offset = fieldOffset(aggregateType, i);
            else if (aggregateType->isUnion())
                offset = 0;
            else
                offset = (uint64_t)i * typeSize(aggregateType->getElementType());
            storeConstToAddr(elements[i], addr + offset);
        }
        return;
    }
    default:
        llvm_unreachable("unexpected value in global initializer");
    }
}

void X64AsmJitGenerator::initGlobal(const GlobalVariable* global) {
    if (!global->value) return; // extern
    // Initialize every defined global, not just address-taken ones: a global
    // can be observed indirectly through another global's initializer.
    storeConstToAddr(global->value, (char*)getGlobalAddr(global));
}

void X64AsmJitGenerator::codegenModules(const std::vector<IRModule*>& modules) {
    // Slot functions by mangled name: body-less references to functions
    // defined in another module alias the definition's slot.
    std::unordered_map<std::string, size_t> indexByName;
    std::vector<const Function*> owners;
    auto slotFor = [&](const Function* function) {
        auto it = indexByName.find(function->mangledName);
        if (it != indexByName.end()) {
            funcIndex.emplace(function, it->second);
            return it->second;
        }
        size_t index = owners.size();
        indexByName.emplace(function->mangledName, index);
        owners.push_back(function);
        funcIndex.emplace(function, index);
        return index;
    };
    for (auto* module : modules) {
        for (auto* function : module->functions)
            slotFor(function);
    }
    funcTable = (void**)memory.alloc(owners.size() * sizeof(void*));
    for (size_t i = 0; i < owners.size(); ++i) {
        // Prefer a definition when several modules declare the same name.
        const Function* defined = owners[i]->body.empty() ? nullptr : owners[i];
        if (!defined || owners[i]->isExtern) {
            for (auto* module : modules) {
                for (auto* function : module->functions) {
                    if (function->mangledName == owners[i]->mangledName && !function->isExtern && !function->body.empty()) defined = function;
                }
            }
        }
        if (defined) {
            funcTable[i] = codegenFunction(defined);
        } else {
            funcTable[i] = resolveExtern(owners[i]->mangledName);
            ASSERT(funcTable[i] && "ineligible extern slipped into AsmJit session");
        }
    }
    for (auto* module : modules) {
        for (auto* global : module->globalVariables)
            initGlobal(global);
    }
}

Gp X64AsmJitGenerator::newIntReg(IRType* type) {
    if (type->isPointerType() || (type->isInteger() && getIntegerBitWidth(type) > 32)) return cc->new_gp64();
    return cc->new_gp32();
}

// Narrow integers live extended in 32-bit registers; re-extend after every
// 32-bit operation so the canonical form holds for all later uses.
void X64AsmJitGenerator::canonicalize(Gp reg, IRType* type) {
    if (type->isBool()) return; // produced 0/1 already
    if (type->isChar()) {
        cc->movzx(reg, reg.r8());
        return;
    }
    if (!type->isInteger()) return;
    int width = getIntegerBitWidth(type);
    if (width >= 32) return;
    bool isSigned = type->isSignedInteger();
    if (width == 8) {
        if (isSigned)
            cc->movsx(reg, reg.r8());
        else
            cc->movzx(reg, reg.r8());
    } else {
        ASSERT(width == 16);
        if (isSigned)
            cc->movsx(reg, reg.r16());
        else
            cc->movzx(reg, reg.r16());
    }
}

Gp X64AsmJitGenerator::emitIntCompare(Token::Kind op, Gp left, Gp right, IRType* type) {
    Gp out = cc->new_gp32();
    cc->xor_(out, out);
    cc->cmp(left, right);
    bool isSigned = type->isSignedInteger();
    // char compares unsigned (it zero-extends); bool is 0/1 either way.
    switch (op) {
    case Token::Equal:
        cc->sete(out.r8());
        break;
    case Token::NotEqual:
        cc->setne(out.r8());
        break;
    case Token::Less:
        if (isSigned)
            cc->setl(out.r8());
        else
            cc->setb(out.r8());
        break;
    case Token::LessOrEqual:
        if (isSigned)
            cc->setle(out.r8());
        else
            cc->setbe(out.r8());
        break;
    case Token::Greater:
        if (isSigned)
            cc->setg(out.r8());
        else
            cc->seta(out.r8());
        break;
    case Token::GreaterOrEqual:
        if (isSigned)
            cc->setge(out.r8());
        else
            cc->setae(out.r8());
        break;
    default:
        llvm_unreachable("not a comparison");
    }
    return out;
}

Gp X64AsmJitGenerator::emitFloatCompare(Token::Kind op, Vec left, Vec right, bool isDouble) {
    Gp out = cc->new_gp32();
    cc->xor_(out, out);
    // The scratch register is zeroed BEFORE the compare: xor clears the
    // flags that setcc reads, so nothing may set flags between ucomi and
    // the last setcc.
    bool needsScratch = op != Token::Greater && op != Token::GreaterOrEqual;
    Gp scratch;
    if (needsScratch) {
        scratch = cc->new_gp32();
        cc->xor_(scratch, scratch);
    }
    if (isDouble)
        cc->ucomisd(left, right);
    else
        cc->ucomiss(left, right);
    // Ordered predicates (NaN compares false except !=), matching clang.
    // ucomi sets ZF/PF/CF on unordered, so every predicate but greater and
    // greater-or-equal must also check the parity flag.
    switch (op) {
    case Token::Equal:
        cc->setnp(scratch.r8());
        cc->setz(out.r8());
        cc->and_(out, scratch);
        break;
    case Token::NotEqual:
        cc->setp(out.r8());
        cc->setnz(scratch.r8());
        cc->or_(out, scratch);
        break;
    case Token::Less:
        cc->setb(out.r8());
        cc->setnp(scratch.r8());
        cc->and_(out, scratch);
        break;
    case Token::LessOrEqual:
        cc->setbe(out.r8());
        cc->setnp(scratch.r8());
        cc->and_(out, scratch);
        break;
    case Token::Greater:
        cc->seta(out.r8());
        break;
    case Token::GreaterOrEqual:
        cc->setae(out.r8());
        break;
    default:
        llvm_unreachable("not a comparison");
    }
    return out;
}

// Integer division and remainder at the operand width. x86 traps on division
// by zero and signed overflow (INT_MIN / -1), matching LLVM codegen.
void X64AsmJitGenerator::emitDivMod(Gp out, Gp left, Gp right, bool is64, bool isSigned, bool isMod) {
    if (is64) {
        Gp lo = cc->new_gp64();
        Gp hi = cc->new_gp64();
        cc->mov(lo, left);
        if (isSigned)
            cc->cqo(hi, lo);
        else
            cc->xor_(hi, hi);
        if (isSigned)
            cc->idiv(hi, lo, right);
        else
            cc->div(hi, lo, right);
        cc->mov(out, isMod ? hi : lo);
    } else {
        Gp lo = cc->new_gp32();
        Gp hi = cc->new_gp32();
        cc->mov(lo, left);
        if (isSigned)
            cc->cdq(hi, lo);
        else
            cc->xor_(hi, hi);
        if (isSigned)
            cc->idiv(hi, lo, right);
        else
            cc->div(hi, lo, right);
        cc->mov(out, isMod ? hi : lo);
    }
}

Gp X64AsmJitGenerator::emitIntBinary(Token::Kind op, Gp left, Gp right, IRType* type, IRType* rightType) {
    // Pointer arithmetic (char* + int) lowers on the raw addresses.
    Gp out = type->isPointerType() ? cc->new_gp64() : newIntReg(type);
    bool isSigned = type->isSignedInteger();
    if (type->isPointerType() && (rightType->isChar() || (rightType->isInteger() && getIntegerBitWidth(rightType) <= 32))) {
        // Extend a 32-bit index to 64 explicitly; unlike AArch64, x86 has no
        // ISA guarantee that the upper half of a 32-bit value is zero.
        Gp wide = cc->new_gp64();
        if (rightType->isSignedInteger())
            cc->movsxd(wide, right);
        else
            cc->mov(wide.r32(), right);
        right = wide;
    }
    switch (op) {
    case Token::Plus:
        cc->mov(out, left);
        cc->add(out, right);
        break;
    case Token::Minus:
        cc->mov(out, left);
        cc->sub(out, right);
        break;
    case Token::Star:
        cc->mov(out, left);
        cc->imul(out, right);
        break;
    case Token::Slash:
        emitDivMod(out, left, right, out.size() == 8, isSigned, /*isMod=*/false);
        break;
    case Token::Modulo:
    case Token::PositiveModulo: {
        emitDivMod(out, left, right, out.size() == 8, isSigned, /*isMod=*/true);
        if (op == Token::PositiveModulo && isSigned) {
            // ((a % b) + b) % b.
            bool is64 = out.size() == 8;
            Gp shifted = is64 ? cc->new_gp64() : cc->new_gp32();
            cc->mov(shifted, out);
            cc->add(shifted, right);
            emitDivMod(out, shifted, right, is64, isSigned, /*isMod=*/true);
        }
        break;
    }
    case Token::And:
    case Token::AndAnd:
        cc->mov(out, left);
        cc->and_(out, right);
        break;
    case Token::Or:
    case Token::OrOr:
        cc->mov(out, left);
        cc->or_(out, right);
        break;
    case Token::Xor:
        cc->mov(out, left);
        cc->xor_(out, right);
        break;
    case Token::LeftShift:
    case Token::RightShift: {
        cc->mov(out, left);
        Gp count = right.size() == 8 ? right.r32() : right;
        cc->mov(x86::ecx, count);
        if (op == Token::LeftShift)
            cc->shl(out, x86::cl);
        else if (isSigned)
            cc->sar(out, x86::cl);
        else
            cc->shr(out, x86::cl);
        break;
    }
    default:
        llvm_unreachable("invalid integer binary operation");
    }
    if (!type->isPointerType()) canonicalize(out, type);
    return out;
}

Vec X64AsmJitGenerator::emitFloatBinary(Token::Kind op, Vec left, Vec right, bool isDouble) {
    if (op == Token::Modulo || op == Token::PositiveModulo) return callFmod(left, right, op, isDouble);
    Vec out = isDouble ? cc->new_xmm_sd() : cc->new_xmm_ss();
    if (isDouble)
        cc->movsd(out, left);
    else
        cc->movss(out, left);
    switch (op) {
    case Token::Plus:
        if (isDouble)
            cc->addsd(out, right);
        else
            cc->addss(out, right);
        break;
    case Token::Minus:
        if (isDouble)
            cc->subsd(out, right);
        else
            cc->subss(out, right);
        break;
    case Token::Star:
        if (isDouble)
            cc->mulsd(out, right);
        else
            cc->mulss(out, right);
        break;
    case Token::Slash:
        if (isDouble)
            cc->divsd(out, right);
        else
            cc->divss(out, right);
        break;
    default:
        llvm_unreachable("invalid float binary operation");
    }
    return out;
}

// Float remainder lowers to a fmod/fmodf libcall like LLVM's frem.
Vec X64AsmJitGenerator::callFmod(Vec left, Vec right, Token::Kind op, bool isDouble) {
    auto callOnce = [&](Vec a, Vec b) {
        void* target = resolveExtern(isDouble ? "fmod" : "fmodf");
        ASSERT(target && "fmod not resolvable in JIT process");
        Gp targetReg = cc->new_gp64();
        cc->mov(targetReg, (uint64_t)target);
        FuncSignature sig;
        sig.set_ret(isDouble ? TypeId::kFloat64 : TypeId::kFloat32);
        sig.add_arg(isDouble ? TypeId::kFloat64 : TypeId::kFloat32);
        sig.add_arg(isDouble ? TypeId::kFloat64 : TypeId::kFloat32);
        InvokeNode* invoke;
        [[maybe_unused]] Error err = cc->invoke(Out(invoke), targetReg, sig);
        ASSERT(err == Error::kOk);
        invoke->set_arg(0, a);
        invoke->set_arg(1, b);
        Vec out = isDouble ? cc->new_xmm_sd() : cc->new_xmm_ss();
        invoke->set_ret(0, out);
        return out;
    };
    Vec rem = callOnce(left, right);
    if (op == Token::PositiveModulo) {
        Vec shifted = isDouble ? cc->new_xmm_sd() : cc->new_xmm_ss();
        if (isDouble) {
            cc->movsd(shifted, rem);
            cc->addsd(shifted, right);
        } else {
            cc->movss(shifted, rem);
            cc->addss(shifted, right);
        }
        rem = callOnce(shifted, right);
    }
    return rem;
}

void X64AsmJitGenerator::emitCast(const CastInst* inst) {
    IRType* sourceType = inst->value->getType();
    IRType* type = inst->type;
    if (isFloat80Type(type) || isFloat80Type(sourceType)) {
        emitF80Cast(inst, sourceType, type);
        return;
    }
    if (type->isFloatingPoint()) {
        bool destDouble = isDoubleType(type);
        Vec out = destDouble ? cc->new_xmm_sd() : cc->new_xmm_ss();
        // x86 has no unsigned int64->float conversion; halve (exact),
        // convert the half, and double (exact), keeping the lost bit.
        // This is LLVM's lowering, sticky-bit rounding included.
        auto u64ToFloat = [&](Vec dst, Gp in, bool dbl) {
            Gp half = cc->new_gp64();
            Gp bit = cc->new_gp64();
            cc->mov(half, in);
            cc->shr(half, 1);
            cc->mov(bit, in);
            cc->and_(bit, 1);
            cc->or_(half, bit);
            if (dbl) {
                cc->cvtsi2sd(dst, half);
                cc->addsd(dst, dst);
            } else {
                cc->cvtsi2ss(dst, half);
                cc->addss(dst, dst);
            }
        };
        if (sourceType->isFloatingPoint()) {
            if (isDoubleType(sourceType) == destDouble) {
                vecValues.emplace(inst, getVec(inst->value));
                return;
            }
            if (destDouble)
                cc->cvtss2sd(out, getVec(inst->value));
            else
                cc->cvtsd2ss(out, getVec(inst->value));
        } else if (sourceType->isPointerType()) {
            // Pointers convert as unsigned 64-bit (mirrors the AArch64 backend's ucutf).
            u64ToFloat(out, getGp(inst->value), destDouble);
        } else if (sourceType->isSignedInteger()) {
            Gp in = getGp(inst->value);
            if (destDouble)
                cc->cvtsi2sd(out, in);
            else
                cc->cvtsi2ss(out, in);
        } else {
            // Unsigned integers and chars zero-extend; canonical form holds that.
            Gp in = getGp(inst->value);
            int width = sourceType->isInteger() ? getIntegerBitWidth(sourceType) : 8;
            if (width == 64) {
                u64ToFloat(out, in, destDouble);
            } else {
                // Zero-extend to 64 so the signed conversion sees a positive value.
                Gp wide = cc->new_gp64();
                cc->mov(wide.r32(), in);
                if (destDouble)
                    cc->cvtsi2sd(out, wide);
                else
                    cc->cvtsi2ss(out, wide);
            }
        }
        vecValues.emplace(inst, out);
        return;
    }
    if (sourceType->isFloatingPoint()) {
        bool srcDouble = isDoubleType(sourceType);
        Vec in = getVec(inst->value);
        if (type->isBool()) {
            Gp tmp = cc->new_gp32();
            if (srcDouble)
                cc->cvttsd2si(tmp, in);
            else
                cc->cvttss2si(tmp, in);
            Gp out = cc->new_gp32();
            cc->xor_(out, out);
            cc->cmp(tmp, 0);
            cc->setnz(out.r8());
            gpValues.emplace(inst, out);
            return;
        }
        Gp out = newIntReg(type);
        int width = type->isInteger() ? getIntegerBitWidth(type) : type->isPointerType() ? 64 : 8;
        if (width == 64 && !type->isSignedInteger()) {
            // Values at or above 2^63 don't fit the signed conversion;
            // convert the excess and flip the high bit back (mirrors LLVM).
            x86::Mem bound = srcDouble ? cc->new_double_const(ConstPoolScope::kLocal, 9223372036854775808.0)
                                       : cc->new_float_const(ConstPoolScope::kLocal, 9223372036854775808.0f);
            Vec limit = srcDouble ? cc->new_xmm_sd() : cc->new_xmm_ss();
            Label fits = cc->new_label();
            Label done = cc->new_label();
            if (srcDouble) {
                cc->movsd(limit, bound);
                cc->ucomisd(in, limit);
            } else {
                cc->movss(limit, bound);
                cc->ucomiss(in, limit);
            }
            cc->jb(fits);
            Vec excess = srcDouble ? cc->new_xmm_sd() : cc->new_xmm_ss();
            if (srcDouble) {
                cc->movsd(excess, in);
                cc->subsd(excess, limit);
                cc->cvttsd2si(out, excess);
            } else {
                cc->movss(excess, in);
                cc->subss(excess, limit);
                cc->cvttss2si(out, excess);
            }
            Gp highBit = cc->new_gp64();
            cc->mov(highBit, INT64_MIN);
            cc->xor_(out, highBit);
            cc->jmp(done);
            cc->bind(fits);
            if (srcDouble)
                cc->cvttsd2si(out, in);
            else
                cc->cvttss2si(out, in);
            cc->bind(done);
        } else {
            if (srcDouble)
                cc->cvttsd2si(out, in);
            else
                cc->cvttss2si(out, in);
        }
        canonicalize(out, type);
        gpValues.emplace(inst, out);
        return;
    }
    // Integer, bool, char, and pointer casts.
    Gp in = getGp(inst->value);
    if (type->isBool()) {
        Gp out = cc->new_gp32();
        cc->xor_(out, out);
        cc->cmp(in, 0);
        cc->setnz(out.r8());
        gpValues.emplace(inst, out);
        return;
    }
    if (type->isPointerType() || sourceType->isPointerType()) {
        // Pointer-int casts are no-ops on LLP64; narrow ints extend canonically.
        Gp out = cc->new_gp64();
        if (sourceType->isPointerType() && type->isPointerType()) {
            gpValues.emplace(inst, in);
            return;
        }
        if (type->isPointerType()) {
            int width = sourceType->isInteger() ? getIntegerBitWidth(sourceType) : 8;
            if (width > 32) {
                gpValues.emplace(inst, in);
            } else if (sourceType->isSignedInteger()) {
                cc->movsxd(out, in);
                gpValues.emplace(inst, out);
            } else {
                // Zero-extend explicitly (no upper-half guarantee on x86).
                cc->mov(out.r32(), in);
                gpValues.emplace(inst, out);
            }
            return;
        }
        // Pointer to integer: truncate to the canonical form.
        if (type->isInteger() && getIntegerBitWidth(type) > 32) {
            gpValues.emplace(inst, in);
        } else {
            Gp narrow = cc->new_gp32();
            cc->mov(narrow, in.r32());
            canonicalize(narrow, type);
            gpValues.emplace(inst, narrow);
        }
        return;
    }
    int sourceWidth = sourceType->isPointerType() ? 64 : sourceType->isInteger() ? getIntegerBitWidth(sourceType) : 8;
    int destWidth = type->isInteger() ? getIntegerBitWidth(type) : type->isChar() ? 8 : 32;
    bool sourceSigned = sourceType->isSignedInteger() && !sourceType->isPointerType();
    if (destWidth <= 32 && sourceWidth <= 32) {
        Gp out = cc->new_gp32();
        cc->mov(out, in);
        canonicalize(out, type);
        gpValues.emplace(inst, out);
        return;
    }
    if (destWidth > 32 && sourceWidth <= 32) {
        Gp out = cc->new_gp64();
        if (sourceSigned)
            cc->movsxd(out, in);
        else
            cc->mov(out.r32(), in);
        gpValues.emplace(inst, out);
        return;
    }
    if (destWidth <= 32) {
        Gp out = cc->new_gp32();
        cc->mov(out, in.r32());
        canonicalize(out, type);
        gpValues.emplace(inst, out);
        return;
    }
    Gp out = cc->new_gp64();
    cc->mov(out, in);
    gpValues.emplace(inst, out);
}

void X64AsmJitGenerator::emitF80Cast(const CastInst* inst, IRType* sourceType, IRType* type) {
    bool destF80 = isFloat80Type(type);
    bool srcF80 = isFloat80Type(sourceType);
    if (destF80 && srcF80) {
        gpValues.emplace(inst, getGp(inst->value));
        return;
    }
    if (destF80) {
        Gp home = homeAddr(16, 16);
        if (sourceType->isFloatingPoint()) {
            bool srcDouble = isDoubleType(sourceType);
            Gp bits = srcDouble ? cc->new_gp64() : cc->new_gp32();
            if (srcDouble)
                cc->movq(bits, getVec(inst->value));
            else
                cc->movd(bits, getVec(inst->value));
            callF80FromWord(srcDouble ? F80Op::FromF64 : F80Op::FromF32, home, bits, srcDouble);
        } else {
            Gp in = getGp(inst->value);
            bool pass64 = sourceType->isPointerType() || (sourceType->isInteger() && getIntegerBitWidth(sourceType) > 32);
            bool isSigned = sourceType->isSignedInteger();
            F80Op op;
            if (!sourceType->isInteger() && !sourceType->isPointerType())
                op = F80Op::FromU32; // bool and char zero-extend; canonical form holds that.
            else if (pass64)
                op = isSigned ? F80Op::FromI64 : F80Op::FromU64;
            else
                op = isSigned ? F80Op::FromI32 : F80Op::FromU32;
            callF80FromWord(op, home, in, pass64);
        }
        gpValues.emplace(inst, home);
        return;
    }
    // float80 source.
    Gp home = getGp(inst->value);
    if (type->isFloatingPoint()) {
        bool destDouble = isDoubleType(type);
        Gp bits = callF80ToWord(destDouble ? F80Op::ToF64 : F80Op::ToF32, home, destDouble);
        Vec out = destDouble ? cc->new_xmm_sd() : cc->new_xmm_ss();
        if (destDouble)
            cc->movq(out, bits);
        else
            cc->movd(out, bits);
        vecValues.emplace(inst, out);
        return;
    }
    if (type->isBool()) {
        gpValues.emplace(inst, callF80ToWord(F80Op::ToBool, home, /*is64=*/false));
        return;
    }
    bool want64 = type->isPointerType() || (type->isInteger() && getIntegerBitWidth(type) > 32);
    bool isSigned = type->isSignedInteger();
    F80Op op;
    if (!type->isInteger() && !type->isPointerType())
        op = F80Op::ToU32; // char truncates below.
    else if (want64)
        op = isSigned ? F80Op::ToI64 : F80Op::ToU64;
    else
        op = isSigned ? F80Op::ToI32 : F80Op::ToU32;
    Gp out = callF80ToWord(op, home, want64);
    canonicalize(out, type);
    gpValues.emplace(inst, out);
}

Gp X64AsmJitGenerator::emitF80Binary(Token::Kind op, Gp left, Gp right) {
    Gp home = homeAddr(16, 16);
    switch (op) {
    case Token::Plus:
        callF80Arith(F80Op::Add, home, left, right);
        break;
    case Token::Minus:
        callF80Arith(F80Op::Sub, home, left, right);
        break;
    case Token::Star:
        callF80Arith(F80Op::Mul, home, left, right);
        break;
    case Token::Slash:
        callF80Arith(F80Op::Div, home, left, right);
        break;
    case Token::Modulo:
        callF80Arith(F80Op::Mod, home, left, right);
        break;
    case Token::PositiveModulo: {
        // ((a % b) + b) % b.
        Gp rem = homeAddr(16, 16);
        callF80Arith(F80Op::Mod, rem, left, right);
        Gp shifted = homeAddr(16, 16);
        callF80Arith(F80Op::Add, shifted, rem, right);
        callF80Arith(F80Op::Mod, home, shifted, right);
        break;
    }
    default:
        llvm_unreachable("invalid float80 binary operation");
    }
    return home;
}

Gp X64AsmJitGenerator::emitF80Compare(Token::Kind op, Gp left, Gp right) {
    // Ordered predicates (NaN compares false except !=), matching clang.
    // The helper packs CF/ZF/PF into bits 0/1/2 of the result.
    Gp flags = callF80Cmp(left, right);
    Gp out = cc->new_gp32();
    cc->xor_(out, out);
    switch (op) {
    case Token::Equal: { // ZF && !PF
        Gp masked = cc->new_gp32();
        cc->mov(masked, flags);
        cc->and_(masked, 0b110);
        cc->cmp(masked, 0b010);
        cc->sete(out.r8());
        break;
    }
    case Token::NotEqual: {
        Gp masked = cc->new_gp32();
        cc->mov(masked, flags);
        cc->and_(masked, 0b110);
        cc->cmp(masked, 0b010);
        cc->setne(out.r8());
        break;
    }
    case Token::Less: { // CF && !PF
        Gp masked = cc->new_gp32();
        cc->mov(masked, flags);
        cc->and_(masked, 0b101);
        cc->cmp(masked, 0b001);
        cc->sete(out.r8());
        break;
    }
    case Token::LessOrEqual: { // (CF || ZF) && !PF
        Gp some = cc->new_gp32();
        cc->xor_(some, some);
        cc->test(flags, 0b011);
        cc->setnz(some.r8());
        Gp ordered = cc->new_gp32();
        cc->xor_(ordered, ordered);
        cc->test(flags, 0b100);
        cc->setz(ordered.r8());
        cc->mov(out, some);
        cc->and_(out, ordered);
        break;
    }
    case Token::Greater: // !CF && !ZF
        cc->test(flags, 0b011);
        cc->setz(out.r8());
        break;
    case Token::GreaterOrEqual: // !CF
        cc->test(flags, 0b001);
        cc->setz(out.r8());
        break;
    default:
        llvm_unreachable("not a comparison");
    }
    return out;
}

void X64AsmJitGenerator::callF80Arith(F80Op op, Gp out, Gp a, Gp b) {
    Gp targetReg = cc->new_gp64();
    cc->mov(targetReg, (uint64_t)getF80Helper(op));
    FuncSignature sig;
    sig.add_arg(TypeId::kUInt64);
    sig.add_arg(TypeId::kUInt64);
    sig.add_arg(TypeId::kUInt64);
    InvokeNode* invoke;
    [[maybe_unused]] Error err = cc->invoke(Out(invoke), targetReg, sig);
    ASSERT(err == Error::kOk);
    invoke->set_arg(0, out);
    invoke->set_arg(1, a);
    invoke->set_arg(2, b);
}

void X64AsmJitGenerator::callF80Unary(F80Op op, Gp out, Gp a) {
    Gp targetReg = cc->new_gp64();
    cc->mov(targetReg, (uint64_t)getF80Helper(op));
    FuncSignature sig;
    sig.add_arg(TypeId::kUInt64);
    sig.add_arg(TypeId::kUInt64);
    InvokeNode* invoke;
    [[maybe_unused]] Error err = cc->invoke(Out(invoke), targetReg, sig);
    ASSERT(err == Error::kOk);
    invoke->set_arg(0, out);
    invoke->set_arg(1, a);
}

Gp X64AsmJitGenerator::callF80Cmp(Gp a, Gp b) {
    Gp targetReg = cc->new_gp64();
    cc->mov(targetReg, (uint64_t)getF80Helper(F80Op::CmpFlags));
    FuncSignature sig;
    sig.set_ret(TypeId::kUInt32);
    sig.add_arg(TypeId::kUInt64);
    sig.add_arg(TypeId::kUInt64);
    InvokeNode* invoke;
    [[maybe_unused]] Error err = cc->invoke(Out(invoke), targetReg, sig);
    ASSERT(err == Error::kOk);
    invoke->set_arg(0, a);
    invoke->set_arg(1, b);
    Gp out = cc->new_gp32();
    invoke->set_ret(0, out);
    return out;
}

Gp X64AsmJitGenerator::callF80ToWord(F80Op op, Gp a, bool is64) {
    Gp targetReg = cc->new_gp64();
    cc->mov(targetReg, (uint64_t)getF80Helper(op));
    FuncSignature sig;
    sig.set_ret(is64 ? TypeId::kUInt64 : TypeId::kUInt32);
    sig.add_arg(TypeId::kUInt64);
    InvokeNode* invoke;
    [[maybe_unused]] Error err = cc->invoke(Out(invoke), targetReg, sig);
    ASSERT(err == Error::kOk);
    invoke->set_arg(0, a);
    Gp out = is64 ? cc->new_gp64() : cc->new_gp32();
    invoke->set_ret(0, out);
    return out;
}

void X64AsmJitGenerator::callF80FromWord(F80Op op, Gp out, Gp value, bool is64) {
    Gp targetReg = cc->new_gp64();
    cc->mov(targetReg, (uint64_t)getF80Helper(op));
    FuncSignature sig;
    sig.add_arg(TypeId::kUInt64);
    sig.add_arg(is64 ? TypeId::kUInt64 : TypeId::kUInt32);
    InvokeNode* invoke;
    [[maybe_unused]] Error err = cc->invoke(Out(invoke), targetReg, sig);
    ASSERT(err == Error::kOk);
    invoke->set_arg(0, out);
    invoke->set_arg(1, value);
}

void* X64AsmJitGenerator::getF80Helper(F80Op op) {
    void*& slot = f80helpers[(size_t)op];
    if (slot) return slot;
    CodeHolder code;
    StringLogger logger;
    if (std::getenv("CX_ASMJIT_LOG")) code.set_logger(&logger);
    [[maybe_unused]] Error err = code.init(runtime.environment(), runtime.cpu_features());
    ASSERT(err == Error::kOk);
    x86::Assembler as(&code);
    emitF80Helper(as, op);
    if (std::getenv("CX_ASMJIT_LOG")) llvm::errs() << "; f80 helper " << (unsigned)op << ":\n" << logger.data() << "\n";
    err = runtime.add(&slot, &code);
    ASSERT(err == Error::kOk && slot);
    return slot;
}

void X64AsmJitGenerator::emitF80Helper(x86::Assembler& as, F80Op op) {
    // Win64 ABI throughout: pointer/word args in rcx/rdx/r8, word results in
    // rax/eax. RSP is 16-aligned at entry; the scratch slots below it
    // misalign by 8, which is fine with no further calls inside. Every path
    // leaves the x87 stack exactly as it found it.
    switch (op) {
    case F80Op::Add:
    case F80Op::Sub:
    case F80Op::Mul:
    case F80Op::Div: {
        // (f80* out, const f80* a, const f80* b). x87 arithmetic has no
        // tbyte memory form (only fld/fstp do), so load both operands
        // and use the register-pop form: st1 = st1 OP st0.
        as.fld(x86::tbyte_ptr(x86::rdx));
        as.fld(x86::tbyte_ptr(x86::r8));
        if (op == F80Op::Add)
            as.faddp();
        else if (op == F80Op::Sub)
            as.fsubp();
        else if (op == F80Op::Mul)
            as.fmulp();
        else
            as.fdivp();
        as.fstp(x86::tbyte_ptr(x86::rcx));
        as.ret();
        return;
    }
    case F80Op::Neg: {
        as.fld(x86::tbyte_ptr(x86::rdx));
        as.fchs();
        as.fstp(x86::tbyte_ptr(x86::rcx));
        as.ret();
        return;
    }
    case F80Op::Mod: {
        // fprem loop (the partial-remainder C2 bit requests another pass).
        as.fld(x86::tbyte_ptr(x86::r8));
        as.fld(x86::tbyte_ptr(x86::rdx));
        Label again = as.new_label();
        as.bind(again);
        as.fprem();
        as.fstsw(x86::ax);
        as.sahf(x86::ah);
        as.jp(again);
        as.fstp(x86::st1);
        as.fstp(x86::tbyte_ptr(x86::rcx));
        as.ret();
        return;
    }
    case F80Op::CmpFlags: {
        // (const f80* a, const f80* b) -> bit0=CF(a<b), bit1=ZF(a==b), bit2=PF(unordered).
        as.fld(x86::tbyte_ptr(x86::rdx));
        as.fld(x86::tbyte_ptr(x86::rcx));
        as.fucompp();
        // fucompp reports to the x87 status word, not EFLAGS.
        as.fstsw(x86::ax);
        as.sahf(x86::ah);
        as.setc(x86::al);
        as.setz(x86::cl);
        as.setp(x86::dl);
        as.movzx(x86::eax, x86::al);
        as.movzx(x86::ecx, x86::cl);
        as.movzx(x86::edx, x86::dl);
        as.shl(x86::ecx, 1);
        as.shl(x86::edx, 2);
        as.or_(x86::eax, x86::ecx);
        as.or_(x86::eax, x86::edx);
        as.ret();
        return;
    }
    case F80Op::FromF32: {
        // (f80* out, uint32_t bits).
        as.sub(x86::rsp, 8);
        as.mov(x86::dword_ptr(x86::rsp), x86::edx);
        as.fld(x86::dword_ptr(x86::rsp));
        as.add(x86::rsp, 8);
        as.fstp(x86::tbyte_ptr(x86::rcx));
        as.ret();
        return;
    }
    case F80Op::FromF64: {
        as.sub(x86::rsp, 8);
        as.mov(x86::qword_ptr(x86::rsp), x86::rdx);
        as.fld(x86::qword_ptr(x86::rsp));
        as.add(x86::rsp, 8);
        as.fstp(x86::tbyte_ptr(x86::rcx));
        as.ret();
        return;
    }
    case F80Op::ToF32: {
        // (const f80* in) -> uint32_t bits.
        as.fld(x86::tbyte_ptr(x86::rcx));
        as.sub(x86::rsp, 8);
        as.fstp(x86::dword_ptr(x86::rsp));
        as.mov(x86::eax, x86::dword_ptr(x86::rsp));
        as.add(x86::rsp, 8);
        as.ret();
        return;
    }
    case F80Op::ToF64: {
        as.fld(x86::tbyte_ptr(x86::rcx));
        as.sub(x86::rsp, 8);
        as.fstp(x86::qword_ptr(x86::rsp));
        as.mov(x86::rax, x86::qword_ptr(x86::rsp));
        as.add(x86::rsp, 8);
        as.ret();
        return;
    }
    case F80Op::FromI32: {
        as.sub(x86::rsp, 8);
        as.mov(x86::dword_ptr(x86::rsp), x86::edx);
        as.fild(x86::dword_ptr(x86::rsp));
        as.add(x86::rsp, 8);
        as.fstp(x86::tbyte_ptr(x86::rcx));
        as.ret();
        return;
    }
    case F80Op::FromI64: {
        as.sub(x86::rsp, 8);
        as.mov(x86::qword_ptr(x86::rsp), x86::rdx);
        as.fild(x86::qword_ptr(x86::rsp));
        as.add(x86::rsp, 8);
        as.fstp(x86::tbyte_ptr(x86::rcx));
        as.ret();
        return;
    }
    case F80Op::FromU32: {
        as.sub(x86::rsp, 8);
        as.mov(x86::dword_ptr(x86::rsp), x86::edx);
        as.mov(x86::dword_ptr(x86::rsp, 4), 0);
        as.fild(x86::qword_ptr(x86::rsp));
        as.add(x86::rsp, 8);
        as.fstp(x86::tbyte_ptr(x86::rcx));
        as.ret();
        return;
    }
    case F80Op::FromU64: {
        // Halve (exact), convert the non-negative half, double (exact),
        // and add the lost bit back; all exact in 80-bit precision.
        as.mov(x86::rax, x86::rdx);
        as.and_(x86::rax, 1);
        as.shr(x86::rdx, 1);
        as.sub(x86::rsp, 8);
        as.mov(x86::qword_ptr(x86::rsp), x86::rdx);
        as.fild(x86::qword_ptr(x86::rsp));
        as.add(x86::rsp, 8);
        as.fld(x86::st0);
        as.faddp();
        as.test(x86::rax, x86::rax);
        Label done = as.new_label();
        as.jz(done);
        as.fld1();
        as.faddp();
        as.bind(done);
        as.fstp(x86::tbyte_ptr(x86::rcx));
        as.ret();
        return;
    }
    case F80Op::ToI32:
    case F80Op::ToU32: {
        // fisttp truncates toward zero like cvtt; invalid yields INT64_MIN,
        // whose low half matches cvttss2si's INT32_MIN.
        as.fld(x86::tbyte_ptr(x86::rcx));
        as.sub(x86::rsp, 8);
        as.fisttp(x86::qword_ptr(x86::rsp));
        as.mov(x86::eax, x86::dword_ptr(x86::rsp));
        as.add(x86::rsp, 8);
        as.ret();
        return;
    }
    case F80Op::ToI64: {
        as.fld(x86::tbyte_ptr(x86::rcx));
        as.sub(x86::rsp, 8);
        as.fisttp(x86::qword_ptr(x86::rsp));
        as.mov(x86::rax, x86::qword_ptr(x86::rsp));
        as.add(x86::rsp, 8);
        as.ret();
        return;
    }
    case F80Op::ToU64: {
        // result = 2*trunc(q) + trunc(v - 2*trunc(q)) with q = v/2. A real
        // division is inexact for odd v near 2^64 (2^64-1 halves to 2^63),
        // so halve by decrementing the stored exponent instead, which is
        // exact for every finite v. |v| < 1 truncates to 0 (also -0.0).
        as.fld(x86::tbyte_ptr(x86::rcx));
        as.sub(x86::rsp, 32);
        as.fstp(x86::tbyte_ptr(x86::rsp));
        as.movzx(x86::eax, x86::word_ptr(x86::rsp, 8));
        as.and_(x86::eax, 0x7FFF);
        as.cmp(x86::eax, 0x3FFF);
        Label tiny = as.new_label();
        as.jb(tiny);
        as.dec(x86::word_ptr(x86::rsp, 8));
        as.fld(x86::tbyte_ptr(x86::rsp));
        as.fld(x86::st0);
        as.fisttp(x86::qword_ptr(x86::rsp, 16));
        as.fild(x86::qword_ptr(x86::rsp, 16));
        as.fld(x86::st0);
        as.faddp();
        as.fld(x86::tbyte_ptr(x86::rcx));
        as.fsubrp(x86::st1);
        as.fisttp(x86::qword_ptr(x86::rsp, 24));
        as.fstp(x86::st0);
        as.mov(x86::rax, x86::qword_ptr(x86::rsp, 16));
        as.add(x86::rax, x86::rax);
        as.add(x86::rax, x86::qword_ptr(x86::rsp, 24));
        as.add(x86::rsp, 32);
        as.ret();
        as.bind(tiny);
        as.xor_(x86::eax, x86::eax);
        as.add(x86::rsp, 32);
        as.ret();
        return;
    }
    case F80Op::ToBool: {
        // v != 0, with NaN comparing true like IEEE not-equal.
        as.fld(x86::tbyte_ptr(x86::rcx));
        as.fldz();
        as.fucompp();
        // fucompp reports to the x87 status word, not EFLAGS.
        as.fstsw(x86::ax);
        as.sahf(x86::ah);
        as.setp(x86::al);
        as.setnz(x86::cl);
        as.movzx(x86::eax, x86::al);
        as.movzx(x86::ecx, x86::cl);
        as.or_(x86::eax, x86::ecx);
        as.ret();
        return;
    }
    case F80Op::Count:
        break;
    }
    llvm_unreachable("all cases handled");
}

void X64AsmJitGenerator::emitCall(const CallInst* inst) {
    IRType* cxFunctionType = inst->function->getType();
    if (cxFunctionType->isPointerType()) cxFunctionType = cxFunctionType->getPointee();
    ASSERT(cxFunctionType->isFunctionType());
    auto* functionType = llvm::cast<IRFunctionType>(cxFunctionType);
    auto* callee = llvm::dyn_cast<Function>(inst->function);
    bool isExternC = callee && callee->declaredExternC;

    FuncSignature sig;
    unsigned vaIndex = FuncSignature::kNoVarArgs;
    std::vector<AbiClass> fixedClasses;
    for (IRType* param : functionType->paramTypes) {
        if (isExternC && param->isArrayType()) {
            fixedClasses.push_back({AbiClass::Kind::Direct, TypeId::kUInt64}); // decayed
        } else {
            fixedClasses.push_back(classifyType(param));
        }
    }

    // Return slot first (sret pointer when indirect).
    IRType* returnType = functionType->returnType;
    AbiClass retClass = returnType->isVoid() ? AbiClass{AbiClass::Kind::Direct, TypeId::kVoid} : classifyType(returnType);
    Gp sretHome;
    if (retClass.kind == AbiClass::Kind::Indirect) {
        sig.add_arg(TypeId::kUInt64);
    } else if (retClass.kind == AbiClass::Kind::Chunk) {
        sig.set_ret(retClass.chunkIs64 ? TypeId::kUInt64 : TypeId::kUInt32);
    } else if (retClass.kind != AbiClass::Kind::Empty) {
        sig.set_ret(retClass.typeId);
    }

    // Fixed params.
    for (size_t i = 0; i < functionType->paramTypes.size(); ++i) {
        const AbiClass& cls = fixedClasses[i];
        if (cls.kind == AbiClass::Kind::Empty) continue;
        if (cls.kind == AbiClass::Kind::Direct || cls.kind == AbiClass::Kind::Indirect) {
            sig.add_arg(cls.kind == AbiClass::Kind::Indirect ? TypeId::kUInt64 : cls.typeId);
        } else if (cls.kind == AbiClass::Kind::Chunk) {
            sig.add_arg(cls.chunkIs64 ? TypeId::kUInt64 : TypeId::kUInt32);
        }
    }
    // Variadic extras.
    if (functionType->isVariadic) {
        vaIndex = sig.arg_count();
        for (size_t i = functionType->paramTypes.size(); i < inst->args.size(); ++i) {
            IRType* argType = inst->args[i]->getType();
            if (isExternC && argType->isArrayType()) {
                sig.add_arg(TypeId::kUInt64);
                continue;
            }
            AbiClass cls = classifyType(argType);
            if (cls.kind == AbiClass::Kind::Empty) continue;
            if (cls.kind == AbiClass::Kind::Direct) {
                sig.add_arg(cls.typeId);
            } else if (cls.kind == AbiClass::Kind::Chunk) {
                sig.add_arg(cls.chunkIs64 ? TypeId::kUInt64 : TypeId::kUInt32);
            } else {
                // Large aggregate varargs cross as a pointer to a caller copy.
                sig.add_arg(TypeId::kUInt64);
            }
        }
    }
    if (vaIndex != FuncSignature::kNoVarArgs) sig.set_va_index(vaIndex);

    // Materialize every operand BEFORE creating the invoke node: nodes
    // append at the cursor, so anything emitted after would land after the call.
    // Variadic calls address through r10: AsmJit synthesizes float vararg
    // moves into rcx/rdx/r8/r9 after register allocation, which would
    // clobber a target allocated there. r10 is volatile and never an
    // argument register on either 64-bit calling convention.
    Gp target = functionType->isVariadic ? x86::r10 : cc->new_gp64();
    if (callee) {
        Gp slot = cc->new_gp64();
        cc->mov(slot, (uint64_t)&funcTable[funcIndex.at(callee)]);
        cc->mov(target, x86::qword_ptr(slot));
    } else {
        cc->mov(target, getGp(inst->function));
    }
    if (retClass.kind == AbiClass::Kind::Indirect) {
        uint64_t size = typeSize(returnType);
        sretHome = homeAddr(size, typeAlign(returnType));
    }
    struct CallArg {
        Operand op;
        bool isVec = false;
    };
    std::vector<CallArg> callArgs;
    if (retClass.kind == AbiClass::Kind::Indirect) callArgs.push_back({sretHome});
    for (size_t i = 0; i < inst->args.size(); ++i) {
        bool isExtra = i >= functionType->paramTypes.size();
        IRType* argType = isExtra ? inst->args[i]->getType() : functionType->paramTypes[i];
        AbiClass cls = isExtra ? classifyType(argType) : fixedClasses[i];
        if (isExternC && argType->isArrayType()) {
            callArgs.push_back({getGp(inst->args[i])});
            continue;
        }
        if (cls.kind == AbiClass::Kind::Empty) continue;
        if (cls.kind == AbiClass::Kind::Direct) {
            if (argType->isFloatingPoint())
                callArgs.push_back({getVec(inst->args[i]), true});
            else
                callArgs.push_back({getGp(inst->args[i])});
            continue;
        }
        Gp home = getGp(inst->args[i]);
        if (cls.kind == AbiClass::Kind::Chunk) {
            if (!cls.chunkIs64) {
                Gp chunk = cc->new_gp32();
                cc->mov(chunk, memAt(home, 0, 4));
                callArgs.push_back({chunk});
            } else {
                Gp chunk = cc->new_gp64();
                cc->mov(chunk, memAt(home, 0, 8));
                callArgs.push_back({chunk});
            }
        } else {
            ASSERT(cls.kind == AbiClass::Kind::Indirect);
            if (isExternC || isExtra) {
                // The C callee may write its by-value copy; pass a copy.
                uint64_t size = typeSize(argType);
                Gp copy = homeAddr(size, typeAlign(argType));
                emitMemcpy(copy, home, size);
                home = copy;
            }
            callArgs.push_back({home});
        }
    }
    ASSERT(callArgs.size() == sig.arg_count());
    InvokeNode* invoke;
    [[maybe_unused]] Error err = cc->invoke(Out(invoke), target, sig);
    ASSERT(err == Error::kOk);
    for (size_t i = 0; i < callArgs.size(); ++i) {
        if (callArgs[i].isVec)
            invoke->set_arg((unsigned)i, callArgs[i].op.as<Vec>());
        else
            invoke->set_arg((unsigned)i, callArgs[i].op.as<Gp>());
    }

    // Returns.
    if (returnType->isVoid() || retClass.kind == AbiClass::Kind::Empty) {
        if (retClass.kind == AbiClass::Kind::Empty) {
            gpValues.emplace(inst, homeAddr(1, 1));
        }
        return;
    }
    if (retClass.kind == AbiClass::Kind::Indirect) {
        gpValues.emplace(inst, sretHome);
        return;
    }
    if (retClass.kind == AbiClass::Kind::Direct) {
        if (returnType->isFloatingPoint()) {
            Vec out = isDoubleType(returnType) ? cc->new_xmm_sd() : cc->new_xmm_ss();
            invoke->set_ret(0, out);
            vecValues.emplace(inst, out);
        } else {
            Gp out = newIntReg(returnType);
            invoke->set_ret(0, out);
            gpValues.emplace(inst, out);
        }
        return;
    }
    ASSERT(retClass.kind == AbiClass::Kind::Chunk);
    uint64_t size = typeSize(returnType);
    Gp home = homeAddr(size, typeAlign(returnType));
    if (!retClass.chunkIs64) {
        Gp chunk = cc->new_gp32();
        invoke->set_ret(0, chunk);
        cc->mov(memAt(home, 0, 4), chunk);
    } else {
        Gp chunk = cc->new_gp64();
        invoke->set_ret(0, chunk);
        cc->mov(memAt(home, 0, 8), chunk);
    }
    gpValues.emplace(inst, home);
}

} // namespace

static const Function* findMain(const std::vector<IRModule*>& modules) {
    for (auto* module : modules) {
        for (auto* function : module->functions) {
            if (function->mangledName == "main" && !function->isExtern && !function->body.empty()) return function;
        }
    }
    return nullptr;
}

static bool x64ExternResolvable(llvm::StringRef name) {
    if (name.empty()) return false;
#ifdef _WIN32
    // Pinned libc names resolve to the host's address (same as the LLVM
    // JIT); unpinned names that legacy msvcrt.dll also exports keep the
    // link-and-exec path instead of risking a split CRT.
    if (lookupPinnedLibcSymbol(name)) return true;
    if (isMsvcrtAmbiguous(name)) return false;
#endif
    return llvm::sys::DynamicLibrary::SearchForAddressOfSymbol(stripAsmLabelMarker(name).str()) != nullptr;
}

bool X64AsmJitSession::eligible(const std::vector<IRModule*>& modules) {
#if defined(_WIN32) && (defined(_M_X64) || defined(__x86_64__))
    const Function* main = findMain(modules);
    if (!main || (main->params.size() != 0 && main->params.size() != 2)) return false;
    llvm::sys::DynamicLibrary::getPermanentLibrary(nullptr);
    for (auto* module : modules) {
        for (auto* function : module->functions) {
            if ((function->isExtern || function->body.empty()) && !x64ExternResolvable(function->mangledName)) return false;
            // float80 never crosses an extern boundary (see hasByValueFloat80).
            if (function->isExtern || function->declaredExternC) {
                if (hasByValueFloat80(function->returnType)) return false;
                for (auto& param : function->params) {
                    if (hasByValueFloat80(param.type)) return false;
                }
            }
            // Variadic extras to extern calls aren't in the signature; scan
            // the call sites. Indirect calls are always internal (sema
            // rejects extern functions as values).
            if (!function->body.empty()) {
                for (auto* block : function->body) {
                    for (auto* value : block->body) {
                        auto* call = llvm::dyn_cast<CallInst>(value);
                        if (!call) continue;
                        auto* direct = llvm::dyn_cast<Function>(call->function);
                        if (!direct || !direct->isExtern) continue;
                        for (size_t i = direct->params.size(); i < call->args.size(); ++i) {
                            if (hasByValueFloat80(call->args[i]->getType())) return false;
                        }
                    }
                }
            }
        }
        for (auto* global : module->globalVariables) {
            if (!global->value) {
                if (!x64ExternResolvable(global->name)) return false;
                if (hasByValueFloat80(global->type)) return false;
            }
        }
    }
    return true;
#else
    (void)modules;
    return false;
#endif
}

int X64AsmJitSession::run(const std::vector<IRModule*>& modules, const std::string& argv0, const std::vector<std::string>& programArgs) {
    // Process lifetime: JIT code and globals must outlive main's return so
    // atexit handlers and static destructors can still call into them. The
    // destructor runs after user handlers by atexit LIFO order.
    static X64AsmJitGenerator generator;
    generator.codegenModules(modules);
    const Function* main = findMain(modules);
    ASSERT(main);
    void* mainAddr = generator.funcTable[generator.funcIndex.at(main)];
    ASSERT(mainAddr);
    std::vector<char*> argv;
    argv.reserve(programArgs.size() + 2);
    argv.push_back(const_cast<char*>(argv0.c_str()));
    for (const auto& arg : programArgs)
        argv.push_back(const_cast<char*>(arg.c_str()));
    int argc = static_cast<int>(argv.size());
    argv.push_back(nullptr);
    if (main->params.size() == 2) {
        auto* mainFn = reinterpret_cast<int (*)(int, char**)>(mainAddr);
        return mainFn(argc, argv.data());
    }
    auto* mainFn = reinterpret_cast<int (*)()>(mainAddr);
    return mainFn();
}
