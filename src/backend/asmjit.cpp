#include "asmjit.h"
#pragma warning(push, 0)
#include <asmjit/a64.h>
#include <asmjit/core.h>
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

using namespace cx;

namespace {
using namespace asmjit;
using namespace asmjit::a64;

// LP64 type layouts (macOS arm64 / Linux arm64 agree). Kept in sync with
// LLVM's DataLayout by construction; the ABI lit tests pin behavior.
uint64_t typeSize(IRType* type);
uint32_t typeAlign(IRType* type);

uint64_t basicSize(llvm::StringRef name) {
    return llvm::StringSwitch<uint64_t>(name)
        .Cases({"bool", "char", "int8", "uint8", "c_schar", "c_uchar"}, 1)
        .Cases({"int16", "uint16", "c_short", "c_ushort"}, 2)
        .Cases({"int32", "uint32", "c_int", "c_uint", "float32", "c_float"}, 4)
        .Cases({"int64", "uint64", "c_long", "c_ulong", "c_longlong", "c_ulonglong", "c_size_t", "float64", "c_double"}, 8)
        .Case("void", 0)
        .Default(0);
}

uint64_t typeSize(IRType* type) {
    switch (type->kind) {
    case IRTypeKind::IRBasicType: {
        llvm::StringRef name = type->getName();
        if (name == "float80") ABORT("float80 is x86-only and cannot lower on AArch64");
        uint64_t size = basicSize(name);
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
        if (name == "float80") ABORT("float80 is x86-only and cannot lower on AArch64");
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

// Homogeneous floating-point aggregate (AArch64): every scalar is the same
// float or double type, at most four of them (mirrors the LLVM backend).
struct HFAInfo {
    bool isDouble;
    unsigned count;
};

std::optional<HFAInfo> getHFAInfo(IRType* type) {
    if (type->isBasicType()) {
        llvm::StringRef name = type->getName();
        if (name == "float32" || name == "c_float") return HFAInfo{false, 1};
        if (name == "float64" || name == "c_double") return HFAInfo{true, 1};
        return std::nullopt;
    }
    if (type->isArrayType()) {
        auto* arrayType = llvm::cast<IRArrayType>(type);
        if (arrayType->hasSymbolicSize() || arrayType->size <= 0) return std::nullopt;
        auto elem = getHFAInfo(arrayType->elementType);
        if (!elem) return std::nullopt;
        unsigned count = elem->count * (unsigned)arrayType->size;
        if (count > 4) return std::nullopt;
        return HFAInfo{elem->isDouble, count};
    }
    if (type->isStruct() || type->isUnion()) {
        bool isUnion = type->isUnion();
        HFAInfo info{false, 0};
        bool first = true;
        for (const auto& field : type->getFields()) {
            auto fieldInfo = getHFAInfo(field.type);
            if (!fieldInfo || (!first && fieldInfo->isDouble != info.isDouble)) return std::nullopt;
            info = HFAInfo{fieldInfo->isDouble, isUnion ? std::max(info.count, fieldInfo->count) : info.count + fieldInfo->count};
            first = false;
        }
        if (first || info.count > 4) return std::nullopt;
        return info;
    }
    return std::nullopt;
}

// How an aggregate crosses the AArch64 C ABI. Direct scalars lower to one
// register; aggregates coerce like clang (mirrors LLVMGenerator::getAbiCoercedType).
struct AbiClass {
    enum class Kind {
        Direct, // One register; typeId is the AsmJit type.
        IntChunks, // 1-2 integer registers covering the bytes (u32, u64, or 2x u64).
        HFA, // N float/double registers (hfaCount members, hfaIsDouble element type).
        Indirect, // Pointer to caller memory.
        Empty, // Zero-size: occupies no ABI slot.
    };
    Kind kind;
    TypeId typeId = TypeId::kVoid; // Direct only.
    unsigned hfaCount = 0; // HFA only.
    bool hfaIsDouble = false; // HFA only.
    unsigned chunkCount = 0; // IntChunks only (1-2).
    bool chunkIs64 = true; // IntChunks only (u64 chunks, else a single u32).
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
        ABORT("float80 is x86-only and cannot lower on AArch64");
    }
    if (type->isPointerType() || type->isFunctionType()) return {AbiClass::Kind::Direct, TypeId::kUInt64};
    // Aggregates.
    uint64_t size = typeSize(type);
    if (size == 0) return {AbiClass::Kind::Empty};
    if (auto hfa = getHFAInfo(type)) {
        AbiClass cls{AbiClass::Kind::HFA};
        cls.hfaCount = hfa->count;
        cls.hfaIsDouble = hfa->isDouble;
        return cls;
    }
    // Non-HFA aggregates of 16 bytes or less cross in integer registers,
    // float-containing ones included (mirrors the LLVM backend).
    if (size <= 16) {
        AbiClass cls{AbiClass::Kind::IntChunks};
        if (size <= 4) {
            cls.chunkCount = 1;
            cls.chunkIs64 = false;
        } else if (size <= 8) {
            cls.chunkCount = 1;
            cls.chunkIs64 = true;
        } else {
            cls.chunkCount = 2;
            cls.chunkIs64 = true;
        }
        return cls;
    }
    return {AbiClass::Kind::Indirect};
}

// A scalar leaf of an HFA with its byte offset, for register materialization.
struct HFALeaf {
    uint64_t offset;
};

void collectHFALeaves(IRType* type, uint64_t base, std::vector<HFALeaf>& leaves) {
    if (type->isStruct()) {
        for (int i = 0; i < (int)type->getFields().size(); ++i) {
            collectHFALeaves(type->getFields()[i].type, base + fieldOffset(type, i), leaves);
        }
        return;
    }
    if (type->isUnion()) {
        // Unions pass their largest member's leaves (HFA count is the max).
        IRType* best = nullptr;
        unsigned bestCount = 0;
        for (const auto& field : type->getFields()) {
            auto info = getHFAInfo(field.type);
            if (info && info->count > bestCount) {
                bestCount = info->count;
                best = field.type;
            }
        }
        ASSERT(best && "collectHFALeaves called on non-HFA union");
        collectHFALeaves(best, base, leaves);
        return;
    }
    if (type->isArrayType()) {
        auto* arrayType = llvm::cast<IRArrayType>(type);
        ASSERT(!arrayType->hasSymbolicSize() && arrayType->size > 0);
        uint64_t stride = typeSize(arrayType->elementType);
        for (int i = 0; i < arrayType->size; ++i)
            collectHFALeaves(arrayType->elementType, base + i * stride, leaves);
        return;
    }
    leaves.push_back({base});
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

// Side-stack for functions whose aggregate temporaries would overflow AsmJit's
// frame addressing (AArch64 spill slots can't reach past ~32KB). LIFO save and
// restore at function boundaries keeps recursion and threads safe; 8MB per
// thread matches the main-thread stack.
constexpr size_t kJitSideStackSize = 8 * 1024 * 1024;

struct JitSideStack {
    char* base = nullptr;
    size_t bump = 0;
};

thread_local JitSideStack jitSideStack;

extern "C" void* cxJitSideAlloc(uint64_t size) {
    if (!jitSideStack.base) {
        jitSideStack.base = (char*)malloc(kJitSideStackSize);
        if (!jitSideStack.base) ABORT("out of memory in JIT side-stack");
    }
    size_t aligned = (jitSideStack.bump + 15) & ~(size_t)15;
    if (aligned + size > kJitSideStackSize) ABORT("JIT side-stack overflow");
    jitSideStack.bump = aligned + (size_t)size;
    return jitSideStack.base + aligned;
}

extern "C" uint64_t cxJitSideSave() {
    return jitSideStack.bump;
}

extern "C" void cxJitSideRestore(uint64_t saved) {
    jitSideStack.bump = (size_t)saved;
}

struct AsmJitGenerator {
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
    a64::Compiler* cc = nullptr;
    std::unordered_map<const Value*, Gp> gpValues;
    std::unordered_map<const Value*, Vec> vecValues;
    std::unordered_map<const BasicBlock*, Label> blockLabels;
    std::unordered_map<const Value*, Gp> checkedOverflow;
    Gp scratch0, scratch1;

    void codegenModules(const std::vector<IRModule*>& modules);
    void* codegenFunction(const Function* function);
    void codegenBody(const Function* function);
    void codegenInst(const Value* value);
    Gp getGp(const Value* value);
    Vec getVec(const Value* value);
    Gp newIntReg(IRType* type);
    void canonicalize(Gp reg, IRType* type);
    Gp emitIntBinary(Token::Kind op, Gp left, Gp right, IRType* type, IRType* rightType);
    Vec emitFloatBinary(Token::Kind op, Vec left, Vec right, bool isDouble);
    Gp emitIntCompare(Token::Kind op, Gp left, Gp right, IRType* type);
    Gp emitFloatCompare(Token::Kind op, Vec left, Vec right);
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
    // Big-frame mode routes aggregate homes to the side-stack.
    bool bigFrame = false;
    Gp sideSaved;
    Gp sideBase;
    uint64_t sideOffset = 0;
    uint64_t sideReserved = 0;
    uint64_t frameUsed = 0;
    void* getGlobalAddr(const GlobalVariable* global);
    void* getStringAddr(const std::string& value);
    void* resolveExtern(llvm::StringRef name);
    void emitMemcpy(Gp dest, Gp src, uint64_t size);
    // Memory operand for [base + offset]; folds large offsets into address
    // math since AArch64 unsigned immediates only span 12 scaled bits.
    a64::Mem memAt(Gp base, uint64_t offset, uint32_t size);
    Gp homeAddr(uint64_t size, uint32_t align);
    Gp callHelper0(void* target);
    Gp callHelper1(void* target, Gp arg);
    void callHelper1v(void* target, Gp arg);
    static uint64_t estimateFrame(const Function* function);
    uint64_t evalConstInt(const Value* value);
    llvm::APFloat evalConstFloat(const Value* value);
    void initGlobal(const GlobalVariable* global);
    void storeConstToAddr(const Value* value, char* addr);
};

void* AsmJitGenerator::resolveExtern(llvm::StringRef name) {
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
    void* addr = llvm::sys::DynamicLibrary::SearchForAddressOfSymbol(stripAsmLabelMarker(name).str());
    externCache.emplace(std::move(key), addr);
    return addr;
}

void* AsmJitGenerator::getGlobalAddr(const GlobalVariable* global) {
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

void* AsmJitGenerator::getStringAddr(const std::string& value) {
    auto it = stringAddr.find(value);
    if (it != stringAddr.end()) return it->second;
    char* addr = (char*)memory.alloc(value.size() + 1, 1);
    memcpy(addr, value.data(), value.size());
    stringAddr.emplace(value, addr);
    return addr;
}

// Chunk stores write whole 8-byte units (a 12-byte struct takes two), so
// exact-sized slots would overrun into densely packed neighbors.
uint64_t padHome(uint64_t size) {
    return std::max<uint64_t>(8, (size + 7) & ~7ULL);
}

Gp AsmJitGenerator::homeAddr(uint64_t size, uint32_t align) {
    if (bigFrame) {
        // Static offsets into the prologue reservation: side memory is
        // claimed once per invocation however often the site runs, so loops
        // cannot exhaust the side-stack.
        uint64_t chunk = std::max<uint64_t>(16, (size + 15) & ~15ULL);
        ASSERT(sideOffset + chunk <= sideReserved && "frame estimate under-counted; widen estimateFrame");
        Gp addr = cc->new_gp64();
        if (sideOffset <= 4095) {
            cc->add(addr, sideBase, (int64_t)sideOffset);
        } else {
            Gp tmp = cc->new_gp64();
            cc->mov(tmp, sideOffset);
            cc->add(addr, sideBase, tmp);
        }
        sideOffset += chunk;
        return addr;
    }
    uint64_t padded = padHome(size);
    frameUsed += padded;
    ASSERT(frameUsed < 32768 && "frame estimate missed a large home; widen estimateFrame");
    a64::Mem slot = cc->new_stack((uint32_t)padded, align == 0 ? 1 : align);
    Gp addr = cc->new_gp64();
    cc->load_address_of(addr, slot);
    return addr;
}

Gp AsmJitGenerator::callHelper0(void* target) {
    Gp targetReg = cc->new_gp64();
    cc->mov(targetReg, (uint64_t)target);
    FuncSignature sig;
    sig.set_ret(TypeId::kUInt64);
    InvokeNode* invoke;
    [[maybe_unused]] Error err = cc->invoke(Out(invoke), targetReg, sig);
    ASSERT(err == Error::kOk);
    Gp out = cc->new_gp64();
    invoke->set_ret(0, out);
    return out;
}

Gp AsmJitGenerator::callHelper1(void* target, Gp arg) {
    Gp targetReg = cc->new_gp64();
    cc->mov(targetReg, (uint64_t)target);
    FuncSignature sig;
    sig.set_ret(TypeId::kUInt64);
    sig.add_arg(TypeId::kUInt64);
    InvokeNode* invoke;
    [[maybe_unused]] Error err = cc->invoke(Out(invoke), targetReg, sig);
    ASSERT(err == Error::kOk);
    invoke->set_arg(0, arg);
    Gp out = cc->new_gp64();
    invoke->set_ret(0, out);
    return out;
}

void AsmJitGenerator::callHelper1v(void* target, Gp arg) {
    Gp targetReg = cc->new_gp64();
    cc->mov(targetReg, (uint64_t)target);
    FuncSignature sig;
    sig.add_arg(TypeId::kUInt64);
    InvokeNode* invoke;
    [[maybe_unused]] Error err = cc->invoke(Out(invoke), targetReg, sig);
    ASSERT(err == Error::kOk);
    invoke->set_arg(0, arg);
}

// Over-approximate every aggregate home the function can reserve, so the
// big-frame prologue reservation covers the real frame with margin to spare.
// Sizes use side-stack (16-byte) granularity; the small-frame path pads less,
// so this bounds both. Under-counting trips the homeAddr assert instead of
// corrupting memory.
uint64_t AsmJitGenerator::estimateFrame(const Function* function) {
    uint64_t total = 0;
    auto addBytes = [&](uint64_t size) { total += std::max<uint64_t>(16, (size + 15) & ~15ULL); };
    auto addType = [&](IRType* type) { addBytes(typeSize(type)); };
    // Undefined aggregates materialize a dummy home per use (never cached).
    auto addUndefUse = [&](const Value* value) {
        if (value && value->kind == ValueKind::Undefined) {
            IRType* type = value->getType();
            if (type->isStruct() || type->isUnion() || type->isArrayType()) addType(type);
        }
    };
    for (auto& param : function->params) {
        AbiClass cls = classifyType(param.type);
        if (cls.kind == AbiClass::Kind::Empty)
            addBytes(1);
        else if (cls.kind == AbiClass::Kind::IntChunks || cls.kind == AbiClass::Kind::HFA)
            addType(param.type);
    }
    for (auto* block : function->body) {
        for (auto* inst : block->body) {
            switch (inst->kind) {
            case ValueKind::AllocaInst:
                addType(llvm::cast<AllocaInst>(inst)->allocatedType);
                break;
            case ValueKind::InsertInst: {
                IRType* type = inst->getType();
                if (type->isStruct() || type->isUnion() || type->isArrayType()) addType(type);
                addUndefUse(llvm::cast<InsertInst>(inst)->value);
                break;
            }
            case ValueKind::ExtractInst: {
                IRType* type = inst->getType();
                if (type->isStruct() || type->isUnion() || type->isArrayType()) addType(type);
                addUndefUse(llvm::cast<ExtractInst>(inst)->aggregate);
                break;
            }
            case ValueKind::LoadInst: {
                IRType* type = inst->getType();
                if (type->isStruct() || type->isUnion() || type->isArrayType()) addType(type);
                break;
            }
            case ValueKind::BranchInst:
                addUndefUse(llvm::cast<BranchInst>(inst)->argument);
                break;
            case ValueKind::CondBranchInst:
                addUndefUse(llvm::cast<CondBranchInst>(inst)->argument);
                break;
            case ValueKind::ReturnInst:
                addUndefUse(llvm::cast<ReturnInst>(inst)->value);
                break;
            case ValueKind::ArrayOpInst: {
                auto* arrayOp = llvm::cast<ArrayOpInst>(inst);
                addType(arrayOp->arrayType);
                addUndefUse(arrayOp->left);
                addUndefUse(arrayOp->right);
                break;
            }
            case ValueKind::CallInst: {
                auto* call = llvm::cast<CallInst>(inst);
                IRType* cxFunctionType = call->function->getType();
                if (cxFunctionType->isPointerType()) cxFunctionType = cxFunctionType->getPointee();
                auto* functionType = llvm::cast<IRFunctionType>(cxFunctionType);
                AbiClass ret = functionType->returnType->isVoid() ? AbiClass{AbiClass::Kind::Direct, TypeId::kVoid} : classifyType(functionType->returnType);
                if (ret.kind == AbiClass::Kind::Indirect || ret.kind == AbiClass::Kind::IntChunks || ret.kind == AbiClass::Kind::HFA)
                    addType(functionType->returnType);
                else if (ret.kind == AbiClass::Kind::Empty)
                    addBytes(1);
                for (auto* arg : call->args) {
                    IRType* argType = arg->getType();
                    if (argType->isStruct() || argType->isUnion() || argType->isArrayType()) addType(argType);
                }
                break;
            }
            default:
                break;
            }
        }
    }
    return total;
}

a64::Mem AsmJitGenerator::memAt(Gp base, uint64_t offset, uint32_t size) {
    // Pair accesses (size 16) only span a 7-bit scaled displacement.
    uint64_t limit = size == 16 ? 504 : 4095 * size;
    if (offset <= limit) {
        a64::Mem mem = a64::ptr(base, (int32_t)offset);
        return mem;
    }
    Gp tmp = cc->new_gp64();
    cc->mov(tmp, offset);
    cc->add(tmp, base, tmp);
    a64::Mem mem = a64::ptr(tmp);
    return mem;
}

void AsmJitGenerator::emitMemcpy(Gp dest, Gp src, uint64_t size) {
    uint64_t offset = 0;
    // 16-byte pairs first; unaligned ldp/stp are legal on AArch64.
    for (; offset + 16 <= size; offset += 16) {
        cc->ldp(scratch0, scratch1, memAt(src, offset, 16));
        cc->stp(scratch0, scratch1, memAt(dest, offset, 16));
    }
    if (offset + 8 <= size) {
        cc->ldr(scratch0, memAt(src, offset, 8));
        cc->str(scratch0, memAt(dest, offset, 8));
        offset += 8;
    }
    if (offset + 4 <= size) {
        Gp tmp = cc->new_gp32();
        cc->ldr(tmp, memAt(src, offset, 4));
        cc->str(tmp, memAt(dest, offset, 4));
        offset += 4;
    }
    if (offset + 2 <= size) {
        Gp tmp = cc->new_gp32();
        cc->ldrh(tmp, memAt(src, offset, 2));
        cc->strh(tmp, memAt(dest, offset, 2));
        offset += 2;
    }
    if (offset < size) {
        Gp tmp = cc->new_gp32();
        cc->ldrb(tmp, memAt(src, offset, 1));
        cc->strb(tmp, memAt(dest, offset, 1));
    }
}

// Canonical in-register forms: bool/char/int8/int16/uint8/uint16 live
// extended in 32-bit registers (sign- or zero- per type), int32/uint32 in
// 32-bit, int64/uint64/pointers in 64-bit, floats in S/D vector registers.
// Aggregates always live in memory; their value is a 64-bit pointer.
bool isDoubleType(IRType* type) {
    llvm::StringRef name = type->getName();
    return name == "float64" || name == "c_double";
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

Gp AsmJitGenerator::getGp(const Value* value) {
    auto it = gpValues.find(value);
    if (it != gpValues.end()) return it->second;
    codegenInst(value);
    auto found = gpValues.find(value);
    ASSERT(found != gpValues.end() && "instruction produced no Gp value");
    Gp reg = found->second;
    if (isRematerializable(value->kind)) gpValues.erase(found);
    return reg;
}

Vec AsmJitGenerator::getVec(const Value* value) {
    auto it = vecValues.find(value);
    if (it != vecValues.end()) return it->second;
    codegenInst(value);
    auto found = vecValues.find(value);
    ASSERT(found != vecValues.end() && "instruction produced no Vec value");
    Vec reg = found->second;
    if (isRematerializable(value->kind)) vecValues.erase(found);
    return reg;
}

void AsmJitGenerator::codegenInst(const Value* value) {
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
            cc->mov(reg, extended);
            gpValues.emplace(value, reg);
        }
        return;
    }
    case ValueKind::ConstantFP: {
        auto* inst = llvm::cast<ConstantFP>(value);
        llvm::StringRef fpName = inst->type->getName();
        ASSERT(fpName == "float32" || fpName == "c_float" || fpName == "float64" || fpName == "c_double");
        if (isDoubleType(inst->type)) {
            Vec reg = cc->new_vec_d();
            a64::Mem mem = cc->new_dword_const(ConstPoolScope::kLocal, inst->value.bitcastToAPInt().getZExtValue());
            cc->ldr(reg, mem);
            vecValues.emplace(value, reg);
        } else {
            Vec reg = cc->new_vec_s();
            uint32_t bits = (uint32_t)inst->value.bitcastToAPInt().getZExtValue();
            a64::Mem mem = cc->new_word_const(ConstPoolScope::kLocal, bits);
            cc->ldr(reg, mem);
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
        if (type->isFloatingPoint()) {
            Vec reg = isDoubleType(type) ? cc->new_vec_d() : cc->new_vec_s();
            Gp zero = cc->new_gp64();
            cc->mov(zero, 0);
            cc->fmov(reg, zero);
            vecValues.emplace(value, reg);
            return;
        }
        if (type->isStruct() || type->isUnion() || type->isArrayType()) {
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
        a64::Mem mem = a64::ptr(slot);
        cc->ldr(reg, mem);
        gpValues.emplace(value, reg);
        return;
    }
    case ValueKind::Parameter:
        llvm_unreachable("parameters are pre-assigned, never emitted");
    case ValueKind::BinaryInst: {
        auto* inst = llvm::cast<BinaryInst>(value);
        IRType* type = inst->left->getType();
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
                gpValues.emplace(value, emitFloatCompare(inst->op, left, right));
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
            if (type->isFloatingPoint())
                vecValues.emplace(value, getVec(inst->operand));
            else
                gpValues.emplace(value, getGp(inst->operand));
            return;
        }
        if (type->isFloatingPoint()) {
            ASSERT(inst->op == Token::Minus);
            Vec in = getVec(inst->operand);
            Vec out = isDoubleType(type) ? cc->new_vec_d() : cc->new_vec_s();
            cc->fneg(out, in);
            vecValues.emplace(value, out);
            return;
        }
        Gp in = getGp(inst->operand);
        Gp out = newIntReg(type);
        if (inst->op == Token::Minus) {
            cc->neg(out, in);
        } else if (inst->op == Token::Not) {
            cc->cmp(in, 0);
            cc->cset(out, CondCode::kEQ);
            gpValues.emplace(value, out);
            return;
        } else {
            ASSERT(inst->op == Token::Tilde);
            if (type->isBool())
                cc->eor(out, in, 1);
            else
                cc->mvn(out, in);
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
        if (type->isStruct() || type->isUnion() || type->isArrayType()) {
            // Copy to a fresh home: the source may be stored through later.
            uint64_t size = typeSize(type);
            Gp home = homeAddr(size, typeAlign(type));
            emitMemcpy(home, ptr, size);
            gpValues.emplace(value, home);
            return;
        }
        if (type->isFloatingPoint()) {
            Vec out = isDoubleType(type) ? cc->new_vec_d() : cc->new_vec_s();
            a64::Mem mem = a64::ptr(ptr);
            cc->ldr(out, mem);
            vecValues.emplace(value, out);
            return;
        }
        Gp out = newIntReg(type);
        int width = type->isInteger() ? getIntegerBitWidth(type) : type->isBool() || type->isChar() ? 8 : 64;
        a64::Mem mem = a64::ptr(ptr);
        if (width == 64 || type->isPointerType()) {
            cc->ldr(out, mem);
        } else if (width == 32) {
            cc->ldr(out, mem);
        } else if (width == 16) {
            if (type->isSignedInteger())
                cc->ldrsh(out, mem);
            else
                cc->ldrh(out, mem);
        } else {
            if (type->isSignedInteger())
                cc->ldrsb(out, mem);
            else
                cc->ldrb(out, mem);
        }
        gpValues.emplace(value, out);
        return;
    }
    case ValueKind::StoreInst: {
        auto* inst = llvm::cast<StoreInst>(value);
        if (inst->value->kind == ValueKind::Undefined) return;
        IRType* type = inst->value->getType();
        Gp ptr = getGp(inst->pointer);
        if (type->isStruct() || type->isUnion() || type->isArrayType()) {
            emitMemcpy(ptr, getGp(inst->value), typeSize(type));
            return;
        }
        if (type->isFloatingPoint()) {
            a64::Mem mem = a64::ptr(ptr);
            cc->str(getVec(inst->value), mem);
            return;
        }
        int width = type->isInteger() ? getIntegerBitWidth(type) : type->isBool() || type->isChar() ? 8 : 64;
        a64::Mem mem = a64::ptr(ptr);
        Gp val = getGp(inst->value);
        if (width == 64 || type->isPointerType()) {
            cc->str(val, mem);
        } else if (width == 32) {
            cc->str(val, mem);
        } else if (width == 16) {
            cc->strh(val, mem);
        } else {
            cc->strb(val, mem);
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
                    cc->add(out, out, bytes);
                } else if (bytes < 0 && bytes >= -4095) {
                    cc->sub(out, out, -bytes);
                } else {
                    Gp tmp = cc->new_gp64();
                    cc->mov(tmp, bytes);
                    cc->add(out, out, tmp);
                }
            } else {
                if (stride == 0) continue; // Indexing over a zero-size type adds nothing.
                Gp idx = getGp(index);
                Gp wide = cc->new_gp64();
                if (idx.size() == 4) {
                    // 32-bit indexes sign-extend for negative indexing.
                    cc->sxtw(wide, idx);
                } else {
                    cc->mov(wide, idx);
                }
                if (stride == 1) {
                    cc->add(out, out, wide);
                } else if ((stride & (stride - 1)) == 0) {
                    int shift = std::countr_zero(stride);
                    cc->lsl(wide, wide, shift);
                    cc->add(out, out, wide);
                } else {
                    Gp tmp = cc->new_gp64();
                    cc->mov(tmp, stride);
                    cc->mul(wide, wide, tmp);
                    cc->add(out, out, wide);
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
            cc->add(out, ptr, (int64_t)offset);
        } else {
            Gp tmp = cc->new_gp64();
            cc->mov(tmp, offset);
            cc->add(out, ptr, tmp);
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
            if (fieldType->isStruct() || fieldType->isUnion() || fieldType->isArrayType()) {
                Gp fieldPtr = cc->new_gp64();
                if (offset <= 4095) {
                    cc->add(fieldPtr, home, (int64_t)offset);
                } else {
                    Gp tmp = cc->new_gp64();
                    cc->mov(tmp, offset);
                    cc->add(fieldPtr, home, tmp);
                }
                emitMemcpy(fieldPtr, getGp(inst->value), typeSize(fieldType));
            } else if (fieldType->isFloatingPoint()) {
                cc->str(getVec(inst->value), memAt(home, offset, isDoubleType(fieldType) ? 8 : 4));
            } else {
                int width = fieldType->isInteger() ? getIntegerBitWidth(fieldType) : fieldType->isBool() || fieldType->isChar() ? 8 : 64;
                Gp val = getGp(inst->value);
                if (width == 64 || fieldType->isPointerType())
                    cc->str(val, memAt(home, offset, 8));
                else if (width == 32)
                    cc->str(val, memAt(home, offset, 4));
                else if (width == 16)
                    cc->strh(val, memAt(home, offset, 2));
                else
                    cc->strb(val, memAt(home, offset, 1));
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
        if (type->isStruct() || type->isUnion() || type->isArrayType()) {
            uint64_t size = typeSize(type);
            Gp home = homeAddr(size, typeAlign(type));
            Gp fieldPtr = cc->new_gp64();
            if (offset <= 4095) {
                cc->add(fieldPtr, base, (int64_t)offset);
            } else {
                Gp tmp = cc->new_gp64();
                cc->mov(tmp, offset);
                cc->add(fieldPtr, base, tmp);
            }
            emitMemcpy(home, fieldPtr, size);
            gpValues.emplace(value, home);
            return;
        }
        if (type->isFloatingPoint()) {
            Vec out = isDoubleType(type) ? cc->new_vec_d() : cc->new_vec_s();
            cc->ldr(out, memAt(base, offset, isDoubleType(type) ? 8 : 4));
            vecValues.emplace(value, out);
            return;
        }
        Gp out = newIntReg(type);
        int width = type->isInteger() ? getIntegerBitWidth(type) : type->isBool() || type->isChar() ? 8 : 64;
        if (width == 64 || type->isPointerType())
            cc->ldr(out, memAt(base, offset, 8));
        else if (width == 32)
            cc->ldr(out, memAt(base, offset, 4));
        else if (width == 16) {
            if (type->isSignedInteger())
                cc->ldrsh(out, memAt(base, offset, 2));
            else
                cc->ldrh(out, memAt(base, offset, 2));
        } else {
            if (type->isSignedInteger())
                cc->ldrsb(out, memAt(base, offset, 1));
            else
                cc->ldrb(out, memAt(base, offset, 1));
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
            else
                cc->fmov(vecValues.at(inst->destination->parameter), getVec(inst->argument));
        }
        cc->b(blockLabels.at(inst->destination));
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
            else
                cc->fmov(vecValues.at(succ->parameter), getVec(inst->argument));
        };
        if (inst->trueBlock->parameter) {
            // The true-edge copy needs its own home; block labels bind once
            // at emission, so route through a trampoline.
            Label tramp = cc->new_label();
            cc->cbnz(cond, tramp);
            moveArg(inst->falseBlock);
            cc->b(blockLabels.at(inst->falseBlock));
            cc->bind(tramp);
            moveArg(inst->trueBlock);
            cc->b(blockLabels.at(inst->trueBlock));
            return;
        }
        cc->cbnz(cond, blockLabels.at(inst->trueBlock));
        moveArg(inst->falseBlock);
        cc->b(blockLabels.at(inst->falseBlock));
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
            Gp tmp = is64 ? cc->new_gp64() : cc->new_gp32();
            cc->mov(tmp, key);
            cc->cmp(cond, tmp);
            cc->b_eq(blockLabels.at(target));
        }
        cc->b(blockLabels.at(inst->defaultBlock));
        return;
    }
    case ValueKind::UnreachableInst:
        cc->brk(1);
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
            if (inst->op == Token::Plus)
                cc->add(full, left, right);
            else
                cc->sub(full, left, right);
            if (!isSigned) {
                int64_t max = (1LL << width) - 1;
                if (inst->op == Token::Plus) {
                    Gp maxReg = cc->new_gp32();
                    cc->mov(maxReg, max);
                    cc->cmp(full, maxReg);
                    cc->csel(out, maxReg, full, CondCode::kHI);
                } else {
                    // Borrow exactly when left < right.
                    Gp zero = cc->new_gp32();
                    cc->mov(zero, 0);
                    cc->cmp(left, right);
                    cc->csel(out, full, zero, CondCode::kCS);
                }
            } else {
                int64_t lo = -(1LL << (width - 1));
                int64_t hi = (1LL << (width - 1)) - 1;
                Gp loReg = cc->new_gp32();
                Gp hiReg = cc->new_gp32();
                cc->mov(loReg, lo);
                cc->mov(hiReg, hi);
                Gp tmp = cc->new_gp32();
                cc->cmp(full, hiReg);
                cc->csel(tmp, hiReg, full, CondCode::kGT);
                cc->cmp(full, loReg);
                cc->csel(out, loReg, tmp, CondCode::kLT);
            }
            canonicalize(out, type);
            gpValues.emplace(value, out);
            return;
        }
        Gp wrapped = is64 ? cc->new_gp64() : cc->new_gp32();
        if (inst->op == Token::Plus)
            cc->adds(wrapped, left, right);
        else
            cc->subs(wrapped, left, right);
        if (!isSigned) {
            Gp clamp = is64 ? cc->new_gp64() : cc->new_gp32();
            cc->mov(clamp, inst->op == Token::Minus ? 0 : -1);
            // Unsigned add overflows on carry, sub on borrow (carry clear).
            cc->csel(out, clamp, wrapped, inst->op == Token::Plus ? CondCode::kCS : CondCode::kCC);
        } else {
            Label sat = cc->new_label();
            Label done = cc->new_label();
            cc->b_vs(sat);
            cc->mov(out, wrapped);
            cc->b(done);
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
            cc->cmp(left, 0);
            cc->csel(out, lo, hi, CondCode::kLT);
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
        if (resultType->isFloatingPoint()) {
            Vec out = isDoubleType(resultType) ? cc->new_vec_d() : cc->new_vec_s();
            Gp tmp = cc->new_gp64();
            cc->mov(tmp, size);
            cc->ucvtf(out, tmp);
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
            cc->mov(out, (int64_t)(uint32_t)size);
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

void AsmJitGenerator::emitReturn(const ReturnInst* inst) {
    // Return registers materialize first; the side-stack restores just before
    // the ret node. Indirect returns memcpy into the caller's home first.
    auto restore = [&] {
        if (bigFrame) callHelper1v((void*)&cxJitSideRestore, sideSaved);
    };
    if (currentRetClass.kind == AbiClass::Kind::Indirect) {
        if (inst->value) emitMemcpy(currentSret, getGp(inst->value), typeSize(currentRetType));
        restore();
        cc->ret();
        return;
    }
    if (!inst->value || currentRetClass.kind == AbiClass::Kind::Empty) {
        restore();
        cc->ret();
        return;
    }
    if (currentRetClass.kind == AbiClass::Kind::Direct) {
        if (currentRetType->isFloatingPoint()) {
            Vec v = getVec(inst->value);
            restore();
            cc->ret(v);
        } else {
            Gp r = getGp(inst->value);
            restore();
            cc->ret(r);
        }
        return;
    }
    Gp home = getGp(inst->value);
    if (currentRetClass.kind == AbiClass::Kind::IntChunks) {
        if (!currentRetClass.chunkIs64) {
            Gp chunk = cc->new_gp32();
            cc->ldr(chunk, memAt(home, 0, 4));
            restore();
            cc->ret(chunk);
        } else if (currentRetClass.chunkCount == 1) {
            Gp chunk = cc->new_gp64();
            cc->ldr(chunk, memAt(home, 0, 8));
            restore();
            cc->ret(chunk);
        } else {
            Gp c0 = cc->new_gp64();
            Gp c1 = cc->new_gp64();
            cc->ldr(c0, memAt(home, 0, 8));
            cc->ldr(c1, memAt(home, 8, 8));
            restore();
            cc->add_ret(c0, c1);
        }
        return;
    }
    ASSERT(currentRetClass.kind == AbiClass::Kind::HFA);
    std::vector<HFALeaf> leaves;
    collectHFALeaves(currentRetType, 0, leaves);
    if (currentRetClass.hfaCount == 1) {
        Vec v = currentRetClass.hfaIsDouble ? cc->new_vec_d() : cc->new_vec_s();
        cc->ldr(v, memAt(home, leaves[0].offset, currentRetClass.hfaIsDouble ? 8 : 4));
        restore();
        cc->ret(v);
        return;
    }
    // Multi-member HFAs return via the register pack; the FuncNode detail
    // carries the extra assignments (see codegenFunction).
    Vec regs[4];
    for (unsigned c = 0; c < currentRetClass.hfaCount; ++c) {
        regs[c] = currentRetClass.hfaIsDouble ? cc->new_vec_d() : cc->new_vec_s();
        cc->ldr(regs[c], memAt(home, leaves[c].offset, currentRetClass.hfaIsDouble ? 8 : 4));
    }
    restore();
    FuncRetNode* retNode;
    [[maybe_unused]] Error err = cc->add_func_ret_node(Out(retNode), regs[0], regs[1]);
    ASSERT(err == Error::kOk);
    retNode->set_op_count(currentRetClass.hfaCount);
    for (unsigned c = 2; c < currentRetClass.hfaCount; ++c)
        retNode->set_op(c, regs[c]);
}

void AsmJitGenerator::emitCheckedArith(const CheckedArithInst* inst) {
    IRType* type = inst->left->getType();
    ASSERT(type->isInteger());
    int width = getIntegerBitWidth(type);
    bool isSigned = type->isSignedInteger();
    Gp left = getGp(inst->left), right = getGp(inst->right);
    Gp out = width > 32 ? cc->new_gp64() : cc->new_gp32();
    Gp overflow = cc->new_gp32();
    if (width < 32 && inst->op != Token::Star) {
        // Narrow operands can't overflow 32 bits; check the full result
        // against the operand width instead of 32-bit flags.
        Gp full = cc->new_gp32();
        if (inst->op == Token::Plus)
            cc->add(full, left, right);
        else
            cc->sub(full, left, right);
        if (!isSigned) {
            if (inst->op == Token::Plus) {
                Gp maxReg = cc->new_gp32();
                cc->mov(maxReg, (1LL << width) - 1);
                cc->cmp(full, maxReg);
                cc->cset(overflow, CondCode::kHI);
            } else {
                cc->cmp(left, right);
                cc->cset(overflow, CondCode::kCC);
            }
        } else {
            Gp loReg = cc->new_gp32();
            Gp hiReg = cc->new_gp32();
            cc->mov(loReg, -(1LL << (width - 1)));
            cc->mov(hiReg, (1LL << (width - 1)) - 1);
            cc->cmp(full, hiReg);
            cc->cset(overflow, CondCode::kGT);
            Label done = cc->new_label();
            cc->b_gt(done);
            cc->cmp(full, loReg);
            cc->cset(overflow, CondCode::kLT);
            cc->bind(done);
        }
        cc->mov(out, full);
    } else if (inst->op == Token::Plus) {
        cc->adds(out, left, right);
        cc->cset(overflow, isSigned ? CondCode::kVS : CondCode::kCS);
    } else if (inst->op == Token::Minus) {
        cc->subs(out, left, right);
        cc->cset(overflow, isSigned ? CondCode::kVS : CondCode::kCC);
    } else {
        ASSERT(inst->op == Token::Star);
        if (width == 64) {
            Gp lo = cc->new_gp64();
            Gp hi = cc->new_gp64();
            cc->mul(lo, left, right);
            if (isSigned)
                cc->smulh(hi, left, right);
            else
                cc->umulh(hi, left, right);
            if (isSigned) {
                Gp sign = cc->new_gp64();
                cc->asr(sign, lo, 63);
                cc->cmp(hi, sign);
            } else {
                cc->cmp(hi, 0);
            }
            cc->cset(overflow, CondCode::kNE);
            out = lo;
        } else {
            // Widening multiply, then check the truncation.
            Gp wideLeft = cc->new_gp64();
            Gp wideRight = cc->new_gp64();
            if (isSigned) {
                if (width == 32) {
                    cc->sxtw(wideLeft, left);
                    cc->sxtw(wideRight, right);
                } else if (width == 16) {
                    cc->sxth(wideLeft, left);
                    cc->sxth(wideRight, right);
                } else {
                    cc->sxtb(wideLeft, left);
                    cc->sxtb(wideRight, right);
                }
            } else {
                cc->mov(wideLeft.r32(), left);
                cc->mov(wideRight.r32(), right);
            }
            Gp full = cc->new_gp64();
            cc->mul(full, wideLeft, wideRight);
            // Truncate to the operand width, widen back, and compare: any
            // difference means the full product doesn't fit.
            Gp trunc = cc->new_gp32();
            cc->mov(trunc, full.r32());
            canonicalize(trunc, type);
            Gp back = cc->new_gp64();
            if (isSigned) {
                if (width == 8)
                    cc->sxtb(back, trunc);
                else if (width == 16)
                    cc->sxth(back, trunc);
                else
                    cc->sxtw(back, trunc);
            } else {
                cc->mov(back.r32(), trunc);
            }
            cc->cmp(full, back);
            cc->cset(overflow, CondCode::kNE);
            out = trunc;
        }
    }
    canonicalize(out, type);
    gpValues.emplace(inst, out);
    checkedOverflow.emplace(inst, overflow);
}

void AsmJitGenerator::emitArrayOp(const ArrayOpInst* inst) {
    auto* arrayType = llvm::cast<IRArrayType>(inst->arrayType);
    int size = arrayType->size;
    IRType* elemType = arrayType->elementType;
    bool isComparison = inst->op == Token::Equal || inst->op == Token::NotEqual;
    bool foldAnd = inst->op == Token::Equal;
    bool isFloat = elemType->isFloatingPoint();
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
    Label cond = cc->new_label();
    Label body = cc->new_label();
    Label end = cc->new_label();
    cc->b(cond);
    cc->bind(body);
    auto loadElem = [&](Gp ptr) -> std::pair<Gp, Vec> {
        Gp addr = cc->new_gp64();
        cc->add(addr, ptr, off);
        if (isFloat) {
            Vec v = isDouble ? cc->new_vec_d() : cc->new_vec_s();
            a64::Mem mem = a64::ptr(addr);
            cc->ldr(v, mem);
            return {Gp(), v};
        }
        Gp g = elemSize > 4 ? cc->new_gp64() : cc->new_gp32();
        if (elemSize == 8) {
            a64::Mem mem = a64::ptr(addr);
            cc->ldr(g, mem);
        } else if (elemSize == 4) {
            a64::Mem mem = a64::ptr(addr);
            cc->ldr(g, mem);
        } else if (elemSize == 2) {
            a64::Mem mem = a64::ptr(addr);
            if (elemType->isSignedInteger())
                cc->ldrsh(g, mem);
            else
                cc->ldrh(g, mem);
        } else {
            a64::Mem mem = a64::ptr(addr);
            if (elemType->isSignedInteger())
                cc->ldrsb(g, mem);
            else
                cc->ldrb(g, mem);
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
        Gp bit = isFloat ? emitFloatCompare(inst->op, lVec, rVec) : emitIntCompare(inst->op, lGp, rGp, elemType);
        if (foldAnd)
            cc->and_(acc, acc, bit);
        else
            cc->orr(acc, acc, bit);
    } else if (isFloat) {
        Vec elem = emitFloatBinary(inst->op, lVec, rVec, isDouble);
        Gp addr = cc->new_gp64();
        cc->add(addr, dst, off);
        a64::Mem mem = a64::ptr(addr);
        cc->str(elem, mem);
    } else {
        Gp elem = emitIntBinary(inst->op, lGp, rGp, elemType, elemType);
        Gp addr = cc->new_gp64();
        cc->add(addr, dst, off);
        a64::Mem mem = a64::ptr(addr);
        if (elemSize == 8) {
            cc->str(elem, mem);
        } else if (elemSize == 4) {
            cc->str(elem, mem);
        } else if (elemSize == 2) {
            cc->strh(elem, mem);
        } else {
            cc->strb(elem, mem);
        }
    }
    if (elemSize <= 4095) {
        cc->add(off, off, (int64_t)elemSize);
    } else {
        Gp step = cc->new_gp64();
        cc->mov(step, elemSize);
        cc->add(off, off, step);
    }
    cc->bind(cond);
    cc->cmp(off, bound);
    cc->b_lo(body);
    cc->bind(end);
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

void* AsmJitGenerator::codegenFunction(const Function* function) {
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
    a64::Compiler compiler(&code);
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
    } else if (currentRetClass.kind == AbiClass::Kind::IntChunks) {
        sig.set_ret(currentRetClass.chunkIs64 ? TypeId::kUInt64 : TypeId::kUInt32);
    } else if (currentRetClass.kind == AbiClass::Kind::HFA) {
        sig.set_ret(currentRetClass.hfaIsDouble ? TypeId::kFloat64 : TypeId::kFloat32);
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
        } else if (cls.kind == AbiClass::Kind::IntChunks) {
            if (!cls.chunkIs64)
                sig.add_arg(TypeId::kUInt32);
            else
                for (unsigned c = 0; c < cls.chunkCount; ++c)
                    sig.add_arg(TypeId::kUInt64);
        } else if (cls.kind == AbiClass::Kind::HFA) {
            for (unsigned c = 0; c < cls.hfaCount; ++c)
                sig.add_arg(cls.hfaIsDouble ? TypeId::kFloat64 : TypeId::kFloat32);
        }
    }
    if (function->isVariadic) sig.set_va_index(sig.arg_count());

    FuncNode* funcNode = cc->add_func(sig);
    sideReserved = estimateFrame(function);
    bigFrame = sideReserved > 8192;
    frameUsed = 0;
    sideOffset = 0;
    if (bigFrame) {
        sideSaved = callHelper0((void*)&cxJitSideSave);
        Gp bytes = cc->new_gp64();
        cc->mov(bytes, sideReserved);
        sideBase = callHelper1((void*)&cxJitSideAlloc, bytes);
    }
    // Multi-register returns ride the pack (caller side does the same).
    if (currentRetClass.kind == AbiClass::Kind::IntChunks && currentRetClass.chunkCount == 2)
        funcNode->detail().ret(1).init_reg(RegType::kGp64, 1, TypeId::kUInt64);
    if (currentRetClass.kind == AbiClass::Kind::HFA) {
        for (unsigned c = 1; c < currentRetClass.hfaCount; ++c) {
            funcNode->detail().ret(c).init_reg(currentRetClass.hfaIsDouble ? RegType::kVec64 : RegType::kVec32, c,
                                               currentRetClass.hfaIsDouble ? TypeId::kFloat64 : TypeId::kFloat32);
        }
    }

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
                Vec v = isDoubleType(param->type) ? cc->new_vec_d() : cc->new_vec_s();
                funcNode->set_arg(argNo++, v);
                vecValues.emplace(param, v);
            } else {
                Gp r = newIntReg(param->type);
                funcNode->set_arg(argNo++, r);
                gpValues.emplace(param, r);
            }
            continue;
        }
        // Chunks and HFAs materialize into a home; aggregates live in memory.
        uint64_t size = typeSize(param->type);
        Gp home = homeAddr(size, typeAlign(param->type));
        if (cls.kind == AbiClass::Kind::IntChunks) {
            if (!cls.chunkIs64) {
                Gp chunk = cc->new_gp32();
                funcNode->set_arg(argNo++, chunk);
                cc->str(chunk, memAt(home, 0, 4));
            } else {
                for (unsigned c = 0; c < cls.chunkCount; ++c) {
                    Gp chunk = cc->new_gp64();
                    funcNode->set_arg(argNo++, chunk);
                    cc->str(chunk, memAt(home, c * 8, 8));
                }
            }
        } else {
            ASSERT(cls.kind == AbiClass::Kind::HFA);
            std::vector<HFALeaf> leaves;
            collectHFALeaves(param->type, 0, leaves);
            for (unsigned c = 0; c < cls.hfaCount; ++c) {
                Vec v = cls.hfaIsDouble ? cc->new_vec_d() : cc->new_vec_s();
                funcNode->set_arg(argNo++, v);
                cc->str(v, memAt(home, leaves[c].offset, cls.hfaIsDouble ? 8 : 4));
            }
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
            if (type->isFloatingPoint())
                vecValues.emplace(block->parameter, isDoubleType(type) ? cc->new_vec_d() : cc->new_vec_s());
            else if (type->isStruct() || type->isUnion() || type->isArrayType())
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

llvm::APFloat AsmJitGenerator::evalConstFloat(const Value* value) {
    switch (value->kind) {
    case ValueKind::ConstantFP:
        return llvm::cast<ConstantFP>(value)->value;
    case ValueKind::BinaryInst: {
        auto* inst = llvm::cast<BinaryInst>(value);
        llvm::APFloat left = evalConstFloat(inst->left);
        llvm::APFloat right = evalConstFloat(inst->right);
        bool ignored = false;
        // Operands share the result semantics; convert defensively.
        left.convert(isDoubleType(inst->getType()) ? llvm::APFloat::IEEEdouble() : llvm::APFloat::IEEEsingle(), llvm::APFloat::rmNearestTiesToEven, &ignored);
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
            operand.convert(isDoubleType(inst->type) ? llvm::APFloat::IEEEdouble() : llvm::APFloat::IEEEsingle(), llvm::APFloat::rmNearestTiesToEven, &ignored);
            return operand;
        }
        llvm::APFloat result = isDoubleType(inst->type) ? llvm::APFloat(0.0) : llvm::APFloat(0.0f);
        bool isSigned = inst->value->getType()->isSignedInteger();
        llvm::APInt bits(64, evalConstInt(inst->value));
        result.convertFromAPInt(bits, isSigned, llvm::APFloat::rmNearestTiesToEven);
        return result;
    }
    default:
        llvm_unreachable("not a constant float expression");
    }
}

uint64_t AsmJitGenerator::evalConstInt(const Value* value) {
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

void AsmJitGenerator::storeConstToAddr(const Value* value, char* addr) {
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
            if (isDoubleType(value->getType())) {
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
        if (isDoubleType(inst->type)) {
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

void AsmJitGenerator::initGlobal(const GlobalVariable* global) {
    if (!global->value) return; // extern
    // Initialize every defined global, not just address-taken ones: a global
    // can be observed indirectly through another global's initializer.
    storeConstToAddr(global->value, (char*)getGlobalAddr(global));
}

void AsmJitGenerator::codegenModules(const std::vector<IRModule*>& modules) {
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

} // namespace

static const Function* findMain(const std::vector<IRModule*>& modules) {
    for (auto* module : modules) {
        for (auto* function : module->functions) {
            if (function->mangledName == "main" && !function->isExtern && !function->body.empty()) return function;
        }
    }
    return nullptr;
}

bool AsmJitSession::eligible(const std::vector<IRModule*>& modules) {
#if !defined(__aarch64__) && !defined(_M_ARM64)
    return false;
#else
    const Function* main = findMain(modules);
    if (!main || (main->params.size() != 0 && main->params.size() != 2)) return false;
    llvm::sys::DynamicLibrary::getPermanentLibrary(nullptr);
    auto resolvable = [](llvm::StringRef name) {
        return !name.empty() && llvm::sys::DynamicLibrary::SearchForAddressOfSymbol(stripAsmLabelMarker(name).str()) != nullptr;
    };
    for (auto* module : modules) {
        for (auto* function : module->functions) {
            if ((function->isExtern || function->body.empty()) && !resolvable(function->mangledName)) return false;
        }
        for (auto* global : module->globalVariables) {
            if (!global->value && !resolvable(global->name)) return false;
        }
    }
    return true;
#endif
}

int AsmJitSession::run(const std::vector<IRModule*>& modules, const std::string& argv0, const std::vector<std::string>& programArgs) {
    // Process lifetime: JIT code and globals must outlive main's return so
    // atexit handlers and static destructors can still call into them. The
    // destructor runs after user handlers by atexit LIFO order.
    static AsmJitGenerator generator;
    generator.codegenModules(modules);
    const Function* main = findMain(modules);
    ASSERT(main);
    void* mainAddr = generator.funcTable[generator.funcIndex.at(main)];
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

Gp AsmJitGenerator::newIntReg(IRType* type) {
    if (type->isPointerType() || (type->isInteger() && getIntegerBitWidth(type) > 32)) return cc->new_gp64();
    return cc->new_gp32();
}

// Narrow integers live extended in 32-bit registers; re-extend after every
// 32-bit operation so the canonical form holds for all later uses.
void AsmJitGenerator::canonicalize(Gp reg, IRType* type) {
    if (type->isBool()) return; // produced 0/1 already
    if (type->isChar()) {
        cc->uxtb(reg, reg);
        return;
    }
    if (!type->isInteger()) return;
    int width = getIntegerBitWidth(type);
    if (width >= 32) return;
    bool isSigned = type->isSignedInteger();
    if (width == 8) {
        if (isSigned)
            cc->sxtb(reg, reg);
        else
            cc->uxtb(reg, reg);
    } else {
        ASSERT(width == 16);
        if (isSigned)
            cc->sxth(reg, reg);
        else
            cc->uxth(reg, reg);
    }
}

Gp AsmJitGenerator::emitIntCompare(Token::Kind op, Gp left, Gp right, IRType* type) {
    Gp out = cc->new_gp32();
    cc->cmp(left, right);
    bool isSigned = type->isSignedInteger();
    // char compares unsigned (it zero-extends); bool is 0/1 either way.
    CondCode cond;
    switch (op) {
    case Token::Equal:
        cond = CondCode::kEQ;
        break;
    case Token::NotEqual:
        cond = CondCode::kNE;
        break;
    case Token::Less:
        cond = isSigned ? CondCode::kLT : CondCode::kLO;
        break;
    case Token::LessOrEqual:
        cond = isSigned ? CondCode::kLE : CondCode::kLS;
        break;
    case Token::Greater:
        cond = isSigned ? CondCode::kGT : CondCode::kHI;
        break;
    case Token::GreaterOrEqual:
        cond = isSigned ? CondCode::kGE : CondCode::kHS;
        break;
    default:
        llvm_unreachable("not a comparison");
    }
    cc->cset(out, cond);
    return out;
}

Gp AsmJitGenerator::emitFloatCompare(Token::Kind op, Vec left, Vec right) {
    Gp out = cc->new_gp32();
    cc->fcmp(left, right);
    // Ordered predicates (NaN compares false except !=), matching clang.
    CondCode cond;
    switch (op) {
    case Token::Equal:
        cond = CondCode::kEQ;
        break;
    case Token::NotEqual:
        cond = CondCode::kNE;
        break;
    case Token::Less:
        cond = CondCode::kMI;
        break;
    case Token::LessOrEqual:
        cond = CondCode::kLS;
        break;
    case Token::Greater:
        cond = CondCode::kGT;
        break;
    case Token::GreaterOrEqual:
        cond = CondCode::kGE;
        break;
    default:
        llvm_unreachable("not a comparison");
    }
    cc->cset(out, cond);
    return out;
}

Gp AsmJitGenerator::emitIntBinary(Token::Kind op, Gp left, Gp right, IRType* type, IRType* rightType) {
    // Pointer arithmetic (char* + int) lowers on the raw addresses.
    Gp out = type->isPointerType() ? cc->new_gp64() : newIntReg(type);
    bool isSigned = type->isSignedInteger();
    if (type->isPointerType() && (rightType->isChar() || (rightType->isInteger() && getIntegerBitWidth(rightType) <= 32))) {
        // Extend a 32-bit index to 64; 32-bit writes already zero the top,
        // so only signed indices need an instruction.
        if (rightType->isSignedInteger()) {
            Gp wide = cc->new_gp64();
            cc->sxtw(wide, right);
            right = wide;
        } else {
            right = right.r64();
        }
    }
    switch (op) {
    case Token::Plus:
        cc->add(out, left, right);
        break;
    case Token::Minus:
        cc->sub(out, left, right);
        break;
    case Token::Star:
        cc->mul(out, left, right);
        break;
    case Token::Slash:
        if (isSigned)
            cc->sdiv(out, left, right);
        else
            cc->udiv(out, left, right);
        break;
    case Token::Modulo:
    case Token::PositiveModulo: {
        Gp quot = type->isPointerType() || getIntegerBitWidth(type) > 32 ? cc->new_gp64() : cc->new_gp32();
        if (isSigned)
            cc->sdiv(quot, left, right);
        else
            cc->udiv(quot, left, right);
        cc->msub(out, quot, right, left);
        if (op == Token::PositiveModulo && isSigned) {
            // ((a % b) + b) % b.
            Gp shifted = cc->new_gp32();
            if (getIntegerBitWidth(type) > 32) shifted = cc->new_gp64();
            cc->add(shifted, out, right);
            Gp quot2 = cc->new_gp32();
            if (getIntegerBitWidth(type) > 32) quot2 = cc->new_gp64();
            cc->sdiv(quot2, shifted, right);
            cc->msub(out, quot2, right, shifted);
        }
        break;
    }
    case Token::And:
    case Token::AndAnd:
        cc->and_(out, left, right);
        break;
    case Token::Or:
    case Token::OrOr:
        cc->orr(out, left, right);
        break;
    case Token::Xor:
        cc->eor(out, left, right);
        break;
    case Token::LeftShift:
        cc->lslv(out, left, right);
        break;
    case Token::RightShift:
        if (isSigned)
            cc->asrv(out, left, right);
        else
            cc->lsrv(out, left, right);
        break;
    default:
        llvm_unreachable("invalid integer binary operation");
    }
    if (!type->isPointerType()) canonicalize(out, type);
    return out;
}

Vec AsmJitGenerator::emitFloatBinary(Token::Kind op, Vec left, Vec right, bool isDouble) {
    if (op == Token::Modulo || op == Token::PositiveModulo) return callFmod(left, right, op, isDouble);
    Vec out = isDouble ? cc->new_vec_d() : cc->new_vec_s();
    switch (op) {
    case Token::Plus:
        cc->fadd(out, left, right);
        break;
    case Token::Minus:
        cc->fsub(out, left, right);
        break;
    case Token::Star:
        cc->fmul(out, left, right);
        break;
    case Token::Slash:
        cc->fdiv(out, left, right);
        break;
    default:
        llvm_unreachable("invalid float binary operation");
    }
    return out;
}

// Float remainder lowers to a fmod/fmodf libcall like LLVM's frem.
Vec AsmJitGenerator::callFmod(Vec left, Vec right, Token::Kind op, bool isDouble) {
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
        Vec out = isDouble ? cc->new_vec_d() : cc->new_vec_s();
        invoke->set_ret(0, out);
        return out;
    };
    Vec rem = callOnce(left, right);
    if (op == Token::PositiveModulo) {
        Vec shifted = isDouble ? cc->new_vec_d() : cc->new_vec_s();
        cc->fadd(shifted, rem, right);
        rem = callOnce(shifted, right);
    }
    return rem;
}

void AsmJitGenerator::emitCast(const CastInst* inst) {
    IRType* sourceType = inst->value->getType();
    IRType* type = inst->type;
    if (type->isFloatingPoint()) {
        ASSERT(type->getName() != "float80");
        ASSERT(!sourceType->isFloatingPoint() || sourceType->getName() != "float80");
        Vec out = isDoubleType(type) ? cc->new_vec_d() : cc->new_vec_s();
        if (sourceType->isFloatingPoint()) {
            if (isDoubleType(sourceType) == isDoubleType(type)) {
                vecValues.emplace(inst, getVec(inst->value));
                return;
            }
            cc->fcvt(out, getVec(inst->value));
        } else if (sourceType->isSignedInteger()) {
            cc->scvtf(out, getGp(inst->value));
        } else {
            // Unsigned integers and chars zero-extend; canonical form holds that.
            cc->ucvtf(out, getGp(inst->value));
        }
        vecValues.emplace(inst, out);
        return;
    }
    if (sourceType->isFloatingPoint()) {
        if (type->isBool()) {
            Gp tmp = cc->new_gp32();
            cc->fcvtzs(tmp, getVec(inst->value));
            Gp out = cc->new_gp32();
            cc->cmp(tmp, 0);
            cc->cset(out, CondCode::kNE);
            gpValues.emplace(inst, out);
            return;
        }
        Gp out = newIntReg(type);
        if (type->isSignedInteger())
            cc->fcvtzs(out, getVec(inst->value));
        else
            cc->fcvtzu(out, getVec(inst->value));
        canonicalize(out, type);
        gpValues.emplace(inst, out);
        return;
    }
    // Integer, bool, char, and pointer casts.
    Gp in = getGp(inst->value);
    if (type->isBool()) {
        Gp out = cc->new_gp32();
        cc->cmp(in, 0);
        cc->cset(out, CondCode::kNE);
        gpValues.emplace(inst, out);
        return;
    }
    if (type->isPointerType() || sourceType->isPointerType()) {
        // Pointer-int casts are no-ops on LP64; narrow ints extend canonically.
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
                cc->sxtw(out, in);
                gpValues.emplace(inst, out);
            } else {
                gpValues.emplace(inst, in.r64());
            }
            return;
        }
        // Pointer to integer: truncate to the canonical form.
        if (type->isInteger() && getIntegerBitWidth(type) > 32) {
            gpValues.emplace(inst, in);
        } else {
            cc->mov(out.r32(), in.r32());
            canonicalize(out.r32(), type);
            gpValues.emplace(inst, out.r32());
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
            cc->sxtw(out, in);
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

void AsmJitGenerator::emitCall(const CallInst* inst) {
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
    } else if (retClass.kind == AbiClass::Kind::IntChunks) {
        sig.set_ret(retClass.chunkIs64 ? TypeId::kUInt64 : TypeId::kUInt32);
        // Extra chunks ride in x1 via FuncDetail surgery below.
    } else if (retClass.kind == AbiClass::Kind::HFA) {
        sig.set_ret(retClass.hfaIsDouble ? TypeId::kFloat64 : TypeId::kFloat32);
        // Extra members ride in v1+ via FuncDetail surgery below.
    } else if (retClass.kind != AbiClass::Kind::Empty) {
        sig.set_ret(retClass.typeId);
    }

    // Fixed params.
    for (size_t i = 0; i < functionType->paramTypes.size(); ++i) {
        const AbiClass& cls = fixedClasses[i];
        if (cls.kind == AbiClass::Kind::Empty) continue;
        if (cls.kind == AbiClass::Kind::Direct || cls.kind == AbiClass::Kind::Indirect) {
            sig.add_arg(cls.kind == AbiClass::Kind::Indirect ? TypeId::kUInt64 : cls.typeId);
        } else if (cls.kind == AbiClass::Kind::IntChunks) {
            if (!cls.chunkIs64)
                sig.add_arg(TypeId::kUInt32);
            else
                for (unsigned c = 0; c < cls.chunkCount; ++c)
                    sig.add_arg(TypeId::kUInt64);
        } else if (cls.kind == AbiClass::Kind::HFA) {
            for (unsigned c = 0; c < cls.hfaCount; ++c)
                sig.add_arg(cls.hfaIsDouble ? TypeId::kFloat64 : TypeId::kFloat32);
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
            // Large HFA varargs cross as a pointer to a caller copy, like
            // other large aggregates; HFA-ness only affects register passing.
            if (cls.kind == AbiClass::Kind::HFA && typeSize(argType) > 16) cls.kind = AbiClass::Kind::Indirect;
            if (cls.kind == AbiClass::Kind::Empty) continue;
            if (cls.kind == AbiClass::Kind::Direct) {
                sig.add_arg(cls.typeId);
            } else if (cls.kind == AbiClass::Kind::IntChunks) {
                if (!cls.chunkIs64)
                    sig.add_arg(TypeId::kUInt32);
                else
                    for (unsigned c = 0; c < cls.chunkCount; ++c)
                        sig.add_arg(TypeId::kUInt64);
            } else if (cls.kind == AbiClass::Kind::HFA) {
                // Varargs never use vector registers on Apple; pass the bytes
                // in 8-byte slots like small structs.
                uint64_t slots = (typeSize(argType) + 7) / 8;
                for (uint64_t c = 0; c < slots; ++c)
                    sig.add_arg(TypeId::kUInt64);
            } else {
                // Large struct varargs cross as a pointer to a caller copy.
                sig.add_arg(TypeId::kUInt64);
            }
        }
    }
    if (vaIndex != FuncSignature::kNoVarArgs) sig.set_va_index(vaIndex);

    // Materialize every operand BEFORE creating the invoke node: nodes
    // append at the cursor, so anything emitted after would land after the call.
    Gp target = cc->new_gp64();
    if (callee) {
        Gp slot = cc->new_gp64();
        cc->mov(slot, (uint64_t)&funcTable[funcIndex.at(callee)]);
        a64::Mem mem = a64::ptr(slot);
        cc->ldr(target, mem);
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
    auto passChunks = [&](Gp home, const AbiClass& cls) {
        if (!cls.chunkIs64) {
            Gp chunk = cc->new_gp32();
            cc->ldr(chunk, memAt(home, 0, 4));
            callArgs.push_back({chunk});
            return;
        }
        for (unsigned c = 0; c < cls.chunkCount; ++c) {
            Gp chunk = cc->new_gp64();
            cc->ldr(chunk, memAt(home, c * 8, 8));
            callArgs.push_back({chunk});
        }
    };
    auto passHFA = [&](Gp home, const AbiClass& cls, IRType* argType, bool asVararg) {
        if (asVararg) {
            uint64_t slots = (typeSize(argType) + 7) / 8;
            for (uint64_t c = 0; c < slots; ++c) {
                Gp chunk = cc->new_gp64();
                cc->ldr(chunk, memAt(home, c * 8, 8));
                callArgs.push_back({chunk});
            }
            return;
        }
        std::vector<HFALeaf> leaves;
        collectHFALeaves(argType, 0, leaves);
        ASSERT(leaves.size() == cls.hfaCount);
        for (unsigned c = 0; c < cls.hfaCount; ++c) {
            Vec v = cls.hfaIsDouble ? cc->new_vec_d() : cc->new_vec_s();
            if (cls.hfaIsDouble)
                cc->ldr(v, memAt(home, leaves[c].offset, 8));
            else
                cc->ldr(v, memAt(home, leaves[c].offset, 4));
            callArgs.push_back({v, true});
        }
    };
    for (size_t i = 0; i < inst->args.size(); ++i) {
        bool isExtra = i >= functionType->paramTypes.size();
        IRType* argType = isExtra ? inst->args[i]->getType() : functionType->paramTypes[i];
        AbiClass cls = isExtra ? classifyType(argType) : fixedClasses[i];
        if (isExtra && cls.kind == AbiClass::Kind::HFA && typeSize(argType) > 16) cls.kind = AbiClass::Kind::Indirect;
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
        if (cls.kind == AbiClass::Kind::IntChunks) {
            passChunks(home, cls);
        } else if (cls.kind == AbiClass::Kind::HFA) {
            passHFA(home, cls, argType, isExtra);
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
            Vec out = isDoubleType(returnType) ? cc->new_vec_d() : cc->new_vec_s();
            invoke->set_ret(0, out);
            vecValues.emplace(inst, out);
        } else {
            Gp out = newIntReg(returnType);
            invoke->set_ret(0, out);
            gpValues.emplace(inst, out);
        }
        return;
    }
    uint64_t size = typeSize(returnType);
    Gp home = homeAddr(size, typeAlign(returnType));
    if (retClass.kind == AbiClass::Kind::IntChunks) {
        if (!retClass.chunkIs64) {
            Gp chunk = cc->new_gp32();
            invoke->set_ret(0, chunk);
            cc->str(chunk, memAt(home, 0, 4));
        } else {
            for (unsigned c = 0; c < retClass.chunkCount; ++c) {
                if (c > 0) invoke->detail().ret(c).init_reg(RegType::kGp64, c, TypeId::kUInt64);
                Gp chunk = cc->new_gp64();
                invoke->set_ret(c, chunk);
                cc->str(chunk, memAt(home, c * 8, 8));
            }
        }
        gpValues.emplace(inst, home);
        return;
    }
    ASSERT(retClass.kind == AbiClass::Kind::HFA);
    std::vector<HFALeaf> leaves;
    collectHFALeaves(returnType, 0, leaves);
    ASSERT(leaves.size() == retClass.hfaCount);
    for (unsigned c = 0; c < retClass.hfaCount; ++c) {
        if (c > 0) {
            invoke->detail().ret(c).init_reg(retClass.hfaIsDouble ? RegType::kVec64 : RegType::kVec32, c,
                                             retClass.hfaIsDouble ? TypeId::kFloat64 : TypeId::kFloat32);
        }
        Vec v = retClass.hfaIsDouble ? cc->new_vec_d() : cc->new_vec_s();
        invoke->set_ret(c, v);
        cc->str(v, memAt(home, leaves[c].offset, retClass.hfaIsDouble ? 8 : 4));
    }
    gpValues.emplace(inst, home);
}
