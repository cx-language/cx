#include "c-backend.h"
#include <algorithm>
#include <cctype>
#include <cstdint>
#pragma warning(push, 0)
#include <llvm/ADT/SmallString.h>
#include <llvm/Support/Path.h>
#pragma warning(pop)

using namespace cx;
using namespace llvm::sys;

namespace {

bool hasReturnValue(const CallInst* inst) {
    auto* returnType = inst->function->getType()->getPointee()->getReturnType();
    return !returnType->isVoid() && !returnType->isNever();
}

// Pure instructions without side effects: safe to drop when their result is unread.
// Safety checks are separate instructions, so dropping these cannot remove a check.
bool isPureTemp(ValueKind kind) {
    switch (kind) {
    case ValueKind::LoadInst:
    case ValueKind::ExtractInst:
    case ValueKind::BinaryInst:
    case ValueKind::UnaryInst:
    case ValueKind::CastInst:
    case ValueKind::GEPInst:
    case ValueKind::ConstGEPInst:
        return true;
    default:
        return false;
    }
}

// Invokes fn for each value operand of inst. Branch destinations are blocks, not values.
template<typename Fn> void forEachOperand(const Instruction* inst, Fn&& fn) {
    auto mark = [&](const Value* value) {
        if (value) fn(value);
    };
    switch (inst->kind) {
    case ValueKind::ReturnInst:
        mark(llvm::cast<ReturnInst>(inst)->value);
        break;
    case ValueKind::BranchInst:
        mark(llvm::cast<BranchInst>(inst)->argument);
        break;
    case ValueKind::CondBranchInst: {
        auto* branch = llvm::cast<CondBranchInst>(inst);
        mark(branch->condition);
        mark(branch->argument);
        break;
    }
    case ValueKind::SwitchInst: {
        auto* switchInst = llvm::cast<SwitchInst>(inst);
        mark(switchInst->condition);
        for (auto& c : switchInst->cases)
            mark(c.first);
        break;
    }
    case ValueKind::LoadInst:
        mark(llvm::cast<LoadInst>(inst)->value);
        break;
    case ValueKind::StoreInst: {
        auto* store = llvm::cast<StoreInst>(inst);
        mark(store->value);
        mark(store->pointer);
        break;
    }
    case ValueKind::InsertInst: {
        auto* insert = llvm::cast<InsertInst>(inst);
        mark(insert->aggregate);
        mark(insert->value);
        break;
    }
    case ValueKind::ExtractInst:
        mark(llvm::cast<ExtractInst>(inst)->aggregate);
        break;
    case ValueKind::CallInst: {
        auto* call = llvm::cast<CallInst>(inst);
        mark(call->function);
        for (auto* arg : call->args)
            mark(arg);
        break;
    }
    case ValueKind::BinaryInst: {
        auto* binary = llvm::cast<BinaryInst>(inst);
        mark(binary->left);
        mark(binary->right);
        break;
    }
    case ValueKind::UnaryInst:
        mark(llvm::cast<UnaryInst>(inst)->operand);
        break;
    case ValueKind::GEPInst: {
        auto* gep = llvm::cast<GEPInst>(inst);
        mark(gep->pointer);
        for (auto* index : gep->indexes)
            mark(index);
        break;
    }
    case ValueKind::ConstGEPInst:
        mark(llvm::cast<ConstGEPInst>(inst)->pointer);
        break;
    case ValueKind::CastInst:
        mark(llvm::cast<CastInst>(inst)->value);
        break;
    case ValueKind::AllocaInst:
    case ValueKind::UnreachableInst:
    case ValueKind::SizeofInst:
    case ValueKind::BasicBlock:
    case ValueKind::Function:
    case ValueKind::Parameter:
    case ValueKind::GlobalVariable:
    case ValueKind::ConstantString:
    case ValueKind::ConstantInt:
    case ValueKind::ConstantFP:
    case ValueKind::ConstantBool:
    case ValueKind::ConstantNull:
    case ValueKind::Undefined:
        break;
    }
}

// Named structs use field names (see codegenTypeDefinition); anonymous structs fall back to indices.
std::string getFieldName(IRType* type, int index) {
    ASSERT(index < (int)type->getFields().size());
    const auto& fieldName = type->getFields()[index].name;
    return fieldName.empty() ? "_" + std::to_string(index) : fieldName;
}

// Extern C functions with asm labels mangle to '\01' + label for LLVM, where
// the marker suppresses mangling. C has no such marker, so emit the declared
// name instead; the declaration comes from the included header.
llvm::StringRef getCFunctionName(const Function* function) {
    if (!function->mangledName.empty() && function->mangledName[0] == '\01') {
        return function->name;
    }
    return function->mangledName;
}

// The stdlib's opaque 'struct FILE {}' (std/libc.cx) denotes C's FILE. Every use of the
// name resolves to it, so an empty struct named FILE can only be that type (a C-imported
// FILE means the same thing). Emit the FILE typedef from stdio.h instead of a conflicting
// mangled 'struct _M...' definition, which would warn on every conversion to and from real FILE*.
bool isCFileType(const IRStructType* type) {
    return type->name == "FILE" && type->fields.empty();
}

} // namespace

void CGenerator::codegenModule(const IRModule& module) {
    stream << "\n";
    for (auto& includedHeader : module.includedHeaders) {
        stream << "#include \"" << includedHeader << "\"\n";
    }
    stream << "// Module '" << module.name << "' forward declarations\n";
    for (auto* function : module.functions) {
        // Extern functions from C headers get their declarations from the
        // included headers, avoiding duplicate or conflicting definitions.
        // Anything else (including functions defined in CX in libc.cx, such
        // as the Windows fdopen wrapper) needs a forward declaration, since
        // use-before-definition is an error in strict C dialects and MSVC.
        llvm::StringRef filePath = function->location.file;
        bool fromCHeader = path::filename(path::parent_path(filePath)) == "std" && path::filename(filePath) == "libc.cx";
        fromCHeader = fromCHeader || path::extension(filePath) == ".h";
        if (function->isExtern && fromCHeader) {
            continue;
        }
        codegenFunctionPrototype(function);
        stream << ";\n";
    }

    stream << "\n// Module '" << module.name << "' definitions\n";
    for (auto* globalVariable : module.globalVariables) {
        codegenGlobalVariable(globalVariable);
    }
    for (auto* function : module.functions) {
        if (function->isExtern) continue;
        if (alreadyDefinedFunctions.contains(function->mangledName)) continue;
        alreadyDefinedFunctions.insert(function->mangledName);
        codegenFunction(function);
    }
}

void CGenerator::codegenAlloca(const AllocaInst* inst) {
    auto name = claimSuffixedName(!inst->name.empty() ? inst->name : "_alloca");
    if (dispatchMode) {
        // The declaration is hoisted (see codegenFunctionDispatch); only register the name.
        emittedValues.insert({inst, "(&" + name + ")"});
        return;
    }
    stream.indent(4);
    codegenDeclaration(stream, inst->allocatedType, name, true);
    stream << ";\n";
    emittedValues.insert({inst, "(&" + std::move(name) + ")"});
}

void CGenerator::codegenReturn(const ReturnInst* inst) {
    auto* function = inst->parent && inst->parent->parent ? inst->parent->parent : nullptr;
    if (function && function->returnType->isArrayType()) {
        stream.indent(4) << "memcpy(_cx_return, &";
        codegenInst(inst->value);
        stream << ", sizeof(*_cx_return));\n";
        stream.indent(4) << "return _cx_return;\n";
        return;
    }

    stream.indent(4);
    stream << "return";
    if (inst->value) {
        stream << ' ';
        codegenArgument(inst->value);
    }
    stream << ";\n";
}

void CGenerator::codegenArgument(const Value* value) {
    if (value->kind == ValueKind::Undefined) {
        stream << "(";
        codegenTypeExpression(stream, value->getType(), true);
        stream << "){0}";
    } else {
        codegenInst(value);
    }
}

void CGenerator::codegenBranch(const BranchInst* inst) {
    // Assignments to unread block parameters are dead: the argument value is
    // already evaluated where it is defined, so dropping the store is safe.
    if (inst->argument && inst->destination->parameter && !deadValues.contains(inst->destination->parameter)) {
        stream.indent(4) << getBlockParamName(inst->destination->parameter) << " = ";
        codegenArgument(inst->argument);
        stream << "; // branch argument\n";
    }
    if (dispatchMode) {
        stream.indent(4) << "_cx_pc = " << dispatchBlockIds[inst->destination] << ";\n";
        stream.indent(4) << "break;\n";
    } else {
        stream.indent(4) << "goto " << getBlockLabel(inst->destination) << ";\n";
    }
}
static void codegenCondBranchAssignment(CGenerator& generator, llvm::raw_string_ostream& stream, const BasicBlock* block, const Value* argument, int indent) {
    if (block->parameter && argument && !generator.deadValues.contains(block->parameter)) {
        stream.indent(indent) << generator.getBlockParamName(block->parameter) << " = ";
        generator.codegenArgument(argument);
        stream << ";\n";
    }
}

void CGenerator::codegenCondBranch(const CondBranchInst* inst) {
    stream.indent(4) << "if (";
    codegenInst(inst->condition);
    stream << ") {\n";
    codegenCondBranchAssignment(*this, stream, inst->trueBlock, inst->argument, 8);
    if (dispatchMode) {
        stream.indent(8) << "_cx_pc = " << dispatchBlockIds[inst->trueBlock] << ";\n";
    } else {
        stream.indent(8) << "goto " << getBlockLabel(inst->trueBlock) << ";\n";
    }
    stream.indent(4) << "} else {\n";
    codegenCondBranchAssignment(*this, stream, inst->falseBlock, inst->argument, 8);
    if (dispatchMode) {
        stream.indent(8) << "_cx_pc = " << dispatchBlockIds[inst->falseBlock] << ";\n";
    } else {
        stream.indent(8) << "goto " << getBlockLabel(inst->falseBlock) << ";\n";
    }
    stream.indent(4) << "}\n";
    if (dispatchMode) {
        stream.indent(4) << "break;\n";
    }
}

void CGenerator::codegenSwitch(const SwitchInst* inst) {
    stream.indent(4) << "switch (";
    codegenInst(inst->condition);
    stream << ") {\n";
    for (auto [value, block] : inst->cases) {
        stream.indent(8);
        stream << "case ";
        codegenInst(value);
        if (dispatchMode) {
            stream << ": _cx_pc = " << dispatchBlockIds[block] << "; break;\n";
        } else {
            stream << ": goto " << getBlockLabel(block) << ";\n";
        }
    }
    if (dispatchMode) {
        stream.indent(8) << "default: _cx_pc = " << dispatchBlockIds[inst->defaultBlock] << "; break;\n";
        stream.indent(4) << "}\n";
        stream.indent(4) << "break;\n";
    } else {
        stream.indent(4) << "}\n";
        // A matching case always jumps away above, so reaching here means no
        // case matched: continue with the default block.
        stream.indent(4) << "goto " << getBlockLabel(inst->defaultBlock) << ";\n";
    }
}

void CGenerator::codegenLoad(const LoadInst* inst) {
    // Pure and side-effect free (safety checks are separate instructions), so skip when unread.
    if (deadValues.contains(inst)) return;
    stream.indent(4);
    const std::string& name = getOrCreateTempName(inst, "_load");
    if (inst->getType()->isArrayType()) {
        // C arrays are not assignable; declare the temp and copy into it like codegenStore.
        if (!dispatchMode) {
            codegenTempDeclaration(inst, name);
            stream << ";\n";
            stream.indent(4);
        }
        stream << "memcpy(" << name << ", ";
        codegenInst(inst->value);
        stream << ", sizeof(" << name << "));\n";
        return;
    }
    // Emit an explicit type instead of the '__auto_type' GNU extension,
    // so that the generated code can also be compiled with small,
    // strictly conforming C compilers (e.g. the one used by the web playground).
    codegenTempDeclaration(inst, name);
    stream << " = *";
    codegenInst(inst->value);
    stream << ";\n";
}

void CGenerator::codegenStore(const StoreInst* inst) {
    if (inst->value->kind == ValueKind::Undefined) {
        return;
    }
    stream.indent(4);
    if (inst->value->getType()->isArrayType()) {
        stream << "memcpy(";
        codegenInst(inst->pointer);
        stream << ", &";
        codegenInst(inst->value);
        stream << ", sizeof(";
        codegenInst(inst->value);
        stream << "));\n";
    } else {
        stream << "*";
        codegenInst(inst->pointer);
        stream << " = ";
        codegenInst(inst->value);
        stream << ";\n";
    }
}

void CGenerator::codegenInsert(const InsertInst* inst) {
    stream.indent(4);
    auto type = inst->aggregate->getType();
    ASSERT(type->isStruct() || type->isArrayType());
    const std::string& name = getOrCreateTempName(inst, "_insert");
    if (dispatchMode) {
        // The declaration is hoisted (see codegenFunctionDispatch).
    } else {
        codegenDeclaration(stream, type, name, true);
        stream << "; ";
    }
    if (inst->aggregate->kind != ValueKind::Undefined) {
        stream << "memcpy(&" << name << ", &";
        codegenInst(inst->aggregate);
        stream << ", sizeof(" << name << ")); ";
    }
    if (inst->value->getType()->isArrayType()) {
        // C arrays are not assignable; copy element-wise like codegenStore.
        stream << "memcpy(&" << name;
        if (type->isArrayType()) {
            stream << "[" << inst->index << "]";
        } else {
            stream << "." << getFieldName(type, inst->index);
        }
        stream << ", &";
        codegenInst(inst->value);
        stream << ", sizeof(" << name;
        if (type->isArrayType()) {
            stream << "[" << inst->index << "]";
        } else {
            stream << "." << getFieldName(type, inst->index);
        }
        stream << "));\n";
        return;
    }
    stream << name;
    if (type->isArrayType()) {
        stream << "[" << inst->index << "] = ";
    } else {
        stream << "." << getFieldName(type, inst->index) << " = ";
    }
    codegenInst(inst->value);
    stream << ";\n";
}

void CGenerator::codegenExtract(const ExtractInst* inst) {
    if (deadValues.contains(inst)) return;
    stream.indent(4);
    const std::string& name = getOrCreateTempName(inst, "_extract");
    codegenTempDeclaration(inst, name);
    stream << " = ";
    codegenInst(inst->aggregate);
    stream << "." << getFieldName(inst->aggregate->getType(), inst->index);
    stream << ";\n";
}

void CGenerator::codegenCall(const CallInst* inst) {
    stream.indent(4);
    auto* returnType = inst->function->getType()->getPointee()->getReturnType();
    bool returnsArray = returnType->isArrayType();
    // A discarded result needs no temporary, except array results, which need
    // storage for the hidden result pointer.
    bool needsTemp = hasReturnValue(inst) && (returnsArray || useCounts[inst] > 0);
    std::string returnName;
    if (needsTemp) {
        const std::string& name = getOrCreateTempName(inst, "_call");
        if (returnsArray) returnName = name;
        if (returnsArray) {
            if (!dispatchMode) {
                codegenTempDeclarationForType(returnType, name);
                stream << ";\n";
            }
        } else if (dispatchMode) {
            // The declaration is hoisted (see codegenFunctionDispatch).
            stream << name << " = ";
        } else {
            codegenTempDeclarationForType(returnType, name);
            stream << " = ";
        }
    }
    codegenInst(inst->function);
    stream << '(';
    if (returnsArray) {
        // Array-returning functions use the hidden result pointer declared by codegenFunctionPrototype.
        ASSERT(!returnName.empty());
        stream << "(&" << returnName << ")";
        if (!inst->args.empty()) stream << ", ";
    }
    for (size_t i = 0; i < inst->args.size(); ++i) {
        codegenArgument(inst->args[i]);
        if (i + 1 < inst->args.size()) stream << ", ";
    }
    stream << ");\n";
    if (returnType->isNever()) {
        // 'never' erases to 'void', so tell the C compiler the call doesn't
        // return; otherwise it warns about values unset on the fallthrough path.
        stream.indent(4) << "abort();\n";
    }
}

void CGenerator::codegenBinary(const BinaryInst* inst) {
    if (deadValues.contains(inst)) return;
    stream.indent(4);
    const std::string& name = getOrCreateTempName(inst, "_binary_op");
    codegenTempDeclaration(inst, name);
    stream << " = ";
    codegenInst(inst->left);
    stream << ' ';
    switch (inst->op.kind) {
    case Token::Plus:
        stream << '+';
        break;
    case Token::Minus:
        stream << '-';
        break;
    case Token::Star:
        stream << '*';
        break;
    case Token::Slash: {
        // MSVC rejects division with a literal zero divisor (C2124) even for
        // floats, where it is well-defined IEEE arithmetic. x * (±INFINITY)
        // computes the same result for every x, so spell it that way. GCC and
        // xcc accept the division form, but the product form works for them too.
        auto* divisor = llvm::dyn_cast<ConstantFP>(inst->right);
        if (divisor && divisor->value.isZero()) {
            // The dividend was already emitted before the switch.
            stream << (divisor->value.isNegative() ? "* (-INFINITY);\n" : "* INFINITY;\n");
            return;
        }
        stream << '/';
        break;
    }
    case Token::Modulo:
        stream << '%';
        break;
    case Token::LeftShift:
        stream << "<<";
        break;
    case Token::RightShift:
        stream << ">>";
        break;
    case Token::And:
        stream << '&';
        break;
    case Token::Or:
        stream << '|';
        break;
    case Token::Xor:
        stream << '^';
        break;
    case Token::Equal:
        stream << "==";
        break;
    case Token::NotEqual:
        stream << "!=";
        break;
    case Token::Less:
        stream << '<';
        break;
    case Token::LessOrEqual:
        stream << "<=";
        break;
    case Token::Greater:
        stream << '>';
        break;
    case Token::GreaterOrEqual:
        stream << ">=";
        break;
    default:
        llvm_unreachable("all cases handled");
    }
    stream << ' ';
    codegenInst(inst->right);
    stream << ";\n";
}

void CGenerator::codegenUnary(const UnaryInst* inst) {
    if (deadValues.contains(inst)) return;
    stream.indent(4);
    const std::string& name = getOrCreateTempName(inst, "_unary_op");
    codegenTempDeclaration(inst, name);
    stream << " = ";
    switch (inst->op.kind) {
    case Token::Plus:
        stream << '+';
        break;
    case Token::Minus:
        stream << '-';
        break;
    case Token::Not:
        stream << '!';
        break;
    case Token::Tilde:
        stream << '~';
        break;
    default:
        llvm_unreachable("all cases handled");
    }
    codegenInst(inst->operand);
    stream << ";\n";
}

void CGenerator::codegenGEP(const GEPInst* inst) {
    if (deadValues.contains(inst)) return;
    stream.indent(4);
    const std::string& name = getOrCreateTempName(inst, "_get_element_ptr");
    if (dispatchMode) {
        // The declaration is hoisted (see codegenFunctionDispatch).
        stream << name << " = &(";
    } else {
        codegenTempDeclarationForType(inst->getType(), name);
        stream << " = &(";
    }
    codegenInst(inst->pointer);
    for (auto* index : inst->indexes) {
        stream << "[";
        codegenInst(index);
        stream << ']';
    }
    stream << ");\n";
}

void CGenerator::codegenConstGEP(const ConstGEPInst* inst) {
    if (deadValues.contains(inst)) return;
    stream.indent(4);
    const std::string& name = getOrCreateTempName(inst, "_const_get_element_ptr");
    if (dispatchMode) {
        // The declaration is hoisted (see codegenFunctionDispatch).
        stream << name << " = &";
    } else {
        codegenTempDeclarationForType(inst->getType(), name);
        stream << " = &";
    }
    codegenInst(inst->pointer);
    if (inst->pointer->getType()->getPointee()->isArrayType()) {
        stream << "[0][" << inst->index << "];\n";
    } else {
        stream << "->" << getFieldName(inst->pointer->getType()->getPointee(), inst->index) << ";\n";
    }
}

void CGenerator::codegenCast(const CastInst* inst) {
    if (deadValues.contains(inst)) return;
    stream.indent(4);
    const std::string& name = getOrCreateTempName(inst, "_cast");
    codegenTempDeclaration(inst, name);
    stream << " = ";
    stream << "(";
    codegenTypeExpression(stream, inst->type, true);
    stream << ") ";
    codegenInst(inst->value);
    stream << ";\n";
}

void CGenerator::codegenUnreachable(const UnreachableInst* inst) {
    if (!dispatchMode) {
        // Nothing for now. unreachable() is a C23 extension.
        return;
    }
    // Mirror the default mode, where execution falls through to the next block
    // in layout order.
    const BasicBlock* block = inst->parent;
    const Function* function = block ? block->parent : nullptr;
    if (function) {
        auto it = std::find(function->body.begin(), function->body.end(), block);
        if (it != function->body.end() && ++it != function->body.end()) {
            stream.indent(4) << "_cx_pc = " << dispatchBlockIds[*it] << ";\n";
            stream.indent(4) << "break;\n";
            return;
        }
    }
    stream.indent(4) << "abort();\n";
}

void CGenerator::codegenSizeof(const SizeofInst* inst) {
    stream << "sizeof(";
    codegenTypeExpression(stream, inst->type, true);
    stream << ")";
}

void CGenerator::codegenTempDeclaration(const Value* value, const std::string& name) {
    if (dispatchMode) {
        // The declaration is hoisted (see codegenFunctionDispatch); emit just
        // the name, the caller appends the assignment.
        stream << name;
        return;
    }
    codegenTempDeclarationForType(value->getType(), name);
}

// The split prefix/suffix type printer cannot express a pointer to an array. Keep
// the array dimensions after a parenthesized pointer declarator instead.
void CGenerator::codegenTypeExpression(llvm::raw_string_ostream& stream, IRType* type, bool needsTypeDefinition) {
    std::vector<IRPointerType*> pointers;
    IRType* arrayType = type;
    while (auto* pointerType = llvm::dyn_cast<IRPointerType>(arrayType)) {
        pointers.push_back(pointerType);
        arrayType = pointerType->pointee;
    }
    if (!pointers.empty()) {
        if (auto* array = llvm::dyn_cast<IRArrayType>(arrayType)) {
            codegenType(stream, array->elementType, needsTypeDefinition);
            stream << " (";
            for (auto* pointerType : llvm::reverse(pointers)) {
                stream << '*';
                if (!pointerType->mutablePointee) stream << " const";
            }
            stream << ")";
            codegenTypeSuffix(stream, array, needsTypeDefinition);
            return;
        }
    }

    codegenType(stream, type, needsTypeDefinition);
    codegenTypeSuffix(stream, type, needsTypeDefinition);
}

void CGenerator::codegenDeclaration(llvm::raw_string_ostream& stream, IRType* type, llvm::StringRef name, bool needsTypeDefinition) {
    std::vector<IRPointerType*> pointers;
    IRType* arrayType = type;
    while (auto* pointerType = llvm::dyn_cast<IRPointerType>(arrayType)) {
        pointers.push_back(pointerType);
        arrayType = pointerType->pointee;
    }
    if (!pointers.empty()) {
        if (auto* array = llvm::dyn_cast<IRArrayType>(arrayType)) {
            codegenType(stream, array->elementType, needsTypeDefinition);
            stream << " (";
            for (auto* pointerType : llvm::reverse(pointers)) {
                stream << '*';
                if (!pointerType->mutablePointee) stream << " const";
            }
            stream << name << ")";
            codegenTypeSuffix(stream, array, needsTypeDefinition);
            return;
        }
    }

    codegenType(stream, type, needsTypeDefinition);
    stream << ' ' << name;
    codegenTypeSuffix(stream, type, needsTypeDefinition);
}

void CGenerator::codegenTempDeclarationForType(IRType* type, const std::string& name) {
    codegenDeclaration(stream, type, name, true);
}

void CGenerator::codegenBasicBlock(const BasicBlock* block) {
    if (!dispatchMode && !block->name.empty() && gotoTargets.contains(block)) {
        // Extra semicolon to work around "label followed by a declaration is a C23 extension".
        stream << '\n' << getBlockLabel(block) << ": ;\n";
    }
    for (auto* inst : block->body) {
        codegenInst(inst);
    }
}

void CGenerator::codegenGlobalVariable(const GlobalVariable* inst) {
    codegenDeclaration(stream, inst->type, inst->name, true);
    if (inst->value) {
        stream << " = ";
        codegenGlobalInitializer(inst->value);
    }
    stream << ";\n";
    emittedValues.insert({inst, "(&" + inst->name + ")"});
}

void CGenerator::codegenGlobalInitializer(const Value* value) {
    switch (value->kind) {
    case ValueKind::ConstantInt:
        return codegenConstantInt(llvm::cast<ConstantInt>(value));
    case ValueKind::ConstantFP:
        return codegenConstantFP(llvm::cast<ConstantFP>(value));
    case ValueKind::ConstantBool:
        return codegenConstantBool(llvm::cast<ConstantBool>(value));
    case ValueKind::ConstantString:
        return codegenConstantString(llvm::cast<ConstantString>(value));
    case ValueKind::ConstantNull:
        return codegenConstantNull(llvm::cast<ConstantNull>(value));
    case ValueKind::Function:
        stream << getCFunctionName(llvm::cast<Function>(value));
        return;
    case ValueKind::GlobalVariable:
        stream << "(&" << llvm::cast<GlobalVariable>(value)->name << ")";
        return;
    case ValueKind::SizeofInst:
        return codegenSizeof(llvm::cast<SizeofInst>(value));
    case ValueKind::Undefined:
        stream << "{0}";
        return;
    case ValueKind::InsertInst: {
        auto* insert = llvm::cast<InsertInst>(value);
        IRType* aggregateType = insert->aggregate->getType();
        int count = aggregateType->isArrayType() ? aggregateType->getArraySize() : (int)aggregateType->getFields().size();
        std::vector<const Value*> elements(count, nullptr);
        for (auto* current = insert; current; current = llvm::dyn_cast<InsertInst>(current->aggregate)) {
            if (!elements[current->index]) elements[current->index] = current->value;
        }
        stream << "{";
        for (int i = 0; i < count; ++i) {
            if (i > 0) stream << ", ";
            if (elements[i]) {
                codegenGlobalInitializer(elements[i]);
            } else {
                stream << "{0}";
            }
        }
        stream << "}";
        return;
    }
    case ValueKind::BinaryInst: {
        auto* binary = llvm::cast<BinaryInst>(value);
        stream << "(";
        codegenGlobalInitializer(binary->left);
        stream << ' ';
        switch (binary->op.kind) {
        case Token::Plus:
            stream << '+';
            break;
        case Token::Minus:
            stream << '-';
            break;
        case Token::Star:
            stream << '*';
            break;
        case Token::Slash:
            stream << '/';
            break;
        case Token::Modulo:
            stream << '%';
            break;
        case Token::LeftShift:
            stream << "<<";
            break;
        case Token::RightShift:
            stream << ">>";
            break;
        case Token::And:
            stream << '&';
            break;
        case Token::Or:
            stream << '|';
            break;
        case Token::Xor:
            stream << '^';
            break;
        case Token::Equal:
            stream << "==";
            break;
        case Token::NotEqual:
            stream << "!=";
            break;
        case Token::Less:
            stream << '<';
            break;
        case Token::LessOrEqual:
            stream << "<=";
            break;
        case Token::Greater:
            stream << '>';
            break;
        case Token::GreaterOrEqual:
            stream << ">=";
            break;
        default:
            llvm_unreachable("unexpected binary operator in global initializer");
        }
        stream << ' ';
        codegenGlobalInitializer(binary->right);
        stream << ")";
        return;
    }
    case ValueKind::UnaryInst: {
        auto* unary = llvm::cast<UnaryInst>(value);
        // `~` lowers to a Not instruction (see emitNot), so Not with an integer operand is a bitwise not.
        switch (unary->op.kind) {
        case Token::Plus:
            stream << "(+";
            break;
        case Token::Minus:
            stream << "(-";
            break;
        case Token::Tilde:
            stream << "(~";
            break;
        case Token::Not:
            stream << (unary->operand->getType()->isBool() ? "(!" : "(~");
            break;
        default:
            llvm_unreachable("unexpected unary operator in global initializer");
        }
        codegenGlobalInitializer(unary->operand);
        stream << ")";
        return;
    }
    default:
        llvm_unreachable("unexpected value in global initializer");
    }
}

void CGenerator::codegenConstantString(const ConstantString* inst) {
    stream << '"';
    stream.write_escaped(inst->value);
    stream << '"';
}

void CGenerator::codegenConstantInt(const ConstantInt* inst) {
    // -9223372036854775808 parses as unary minus applied to 9223372036854775808,
    // which doesn't fit a signed 64-bit int; spell it to avoid the warning.
    if (inst->value.isSigned() && inst->value.getSignificantBits() <= 64 && inst->value.getSExtValue() == INT64_MIN) {
        stream << "(-9223372036854775807 - 1)";
        return;
    }
    stream << inst->value;
    // Non-negative magnitudes above INT64_MAX don't fit a signed 64-bit literal; spell them
    // unsigned. Wider magnitudes are still unrepresentable as C literals, as before.
    if (!inst->value.isNegative() && inst->value.getActiveBits() > 63) stream << "ULL";
}

void CGenerator::codegenConstantFP(const ConstantFP* inst) {
    if (inst->value.isNaN()) {
        stream << "NAN";
        return;
    }
    if (inst->value.isInfinity()) {
        if (inst->value.isNegative()) stream << "-";
        stream << "INFINITY";
        return;
    }
    // Give narrow constants their own type so arithmetic and casts round through it
    // like the LLVM backend; a bare decimal would compute in double precision.
    auto name = llvm::cast<IRBasicType>(inst->type)->name;
    if (name == "float32" || name == "float80" || name == "c_float") {
        stream << "(";
        codegenType(stream, inst->type, true);
        stream << ") ";
    }
    llvm::SmallString<128> str;
    inst->value.toString(str);
    // Integral values have no decimal point; spell them as floating literals
    // so C doesn't parse them as integers.
    if (llvm::StringRef(str).find_first_of(".eE") == llvm::StringRef::npos) str += ".0";
    stream << str;
}

void CGenerator::codegenConstantBool(const ConstantBool* inst) {
    stream << (inst->value ? "true" : "false");
}

void CGenerator::codegenConstantNull(const ConstantNull*) {
    stream << "NULL";
}

void CGenerator::codegenUndefined(const Undefined*) {
    llvm_unreachable("undefined instructions should be handled in parent instruction");
}

const std::string& CGenerator::getOrCreateTempName(const Value* inst, llvm::StringRef prefix) {
    auto it = emittedValues.find(inst);
    if (it != emittedValues.end()) {
        return it->second;
    }
    return emittedValues.insert({inst, claimSuffixedName(prefix)}).first->second;
}

std::string CGenerator::claimSuffixedName(llvm::StringRef base) {
    std::string name;
    do {
        name = base.str() + std::to_string(valueSuffixCounter++);
    } while (!usedValueNames.insert(name).second);
    return name;
}

void CGenerator::resetValueNaming(const Function* function) {
    valueSuffixCounter = 0;
    usedValueNames.clear();
    for (auto& param : function->params) {
        usedValueNames.insert(param.name);
    }
    if (function->returnType->isArrayType()) usedValueNames.insert("_cx_return");
}

const std::string& CGenerator::getOrCreateTypeName(IRType* type, const std::string& name, llvm::StringRef prefix) {
    if (!name.empty()) return name;
    auto it = generatedTypeNames.find(type);
    if (it != generatedTypeNames.end()) return it->second;
    return generatedTypeNames.insert({type, prefix.str() + std::to_string(generatedTypeNames.size())}).first->second;
}

const std::string& CGenerator::getBlockLabel(const BasicBlock* block) {
    auto it = emittedValues.find(block);
    if (it != emittedValues.end()) {
        return it->second;
    } else {
        auto name = block->name;
        llvm::replace(name, '.', '_');
        llvm::raw_string_ostream os(name);
        os << valueSuffixCounter++;
        return emittedValues.insert({block, std::move(os.str())}).first->second;
    }
}

void CGenerator::codegenInst(const Value* value) {
    auto it = emittedValues.find(value);
    if (it != emittedValues.end()) {
        stream << it->second;
    } else {
        codegenInstImpl(value);
    }
}

void CGenerator::codegenInstImpl(const Value* value) {
    switch (value->kind) {
    case ValueKind::AllocaInst:
        return codegenAlloca(llvm::cast<AllocaInst>(value));
    case ValueKind::ReturnInst:
        return codegenReturn(llvm::cast<ReturnInst>(value));
    case ValueKind::BranchInst:
        return codegenBranch(llvm::cast<BranchInst>(value));
    case ValueKind::CondBranchInst:
        return codegenCondBranch(llvm::cast<CondBranchInst>(value));
    case ValueKind::SwitchInst:
        return codegenSwitch(llvm::cast<SwitchInst>(value));
    case ValueKind::LoadInst:
        return codegenLoad(llvm::cast<LoadInst>(value));
    case ValueKind::StoreInst:
        return codegenStore(llvm::cast<StoreInst>(value));
    case ValueKind::InsertInst:
        return codegenInsert(llvm::cast<InsertInst>(value));
    case ValueKind::ExtractInst:
        return codegenExtract(llvm::cast<ExtractInst>(value));
    case ValueKind::CallInst:
        return codegenCall(llvm::cast<CallInst>(value));
    case ValueKind::BinaryInst:
        return codegenBinary(llvm::cast<BinaryInst>(value));
    case ValueKind::UnaryInst:
        return codegenUnary(llvm::cast<UnaryInst>(value));
    case ValueKind::GEPInst:
        return codegenGEP(llvm::cast<GEPInst>(value));
    case ValueKind::ConstGEPInst:
        return codegenConstGEP(llvm::cast<ConstGEPInst>(value));
    case ValueKind::CastInst:
        return codegenCast(llvm::cast<CastInst>(value));
    case ValueKind::UnreachableInst:
        return codegenUnreachable(llvm::cast<UnreachableInst>(value));
    case ValueKind::SizeofInst:
        return codegenSizeof(llvm::cast<SizeofInst>(value));
    case ValueKind::BasicBlock:
        return codegenBasicBlock(llvm::cast<BasicBlock>(value));
    case ValueKind::Function:
        stream << getCFunctionName(llvm::cast<Function>(value));
        break;
    case ValueKind::Parameter: {
        auto* param = llvm::cast<Parameter>(value);
        // Basic block parameters are registered under their sanitized C names
        // (see getBlockParamName); anything else is a function parameter whose
        // C name is the source name.
        auto it = emittedValues.find(param);
        stream << (it != emittedValues.end() ? it->second : param->name);
        break;
    }
    case ValueKind::GlobalVariable:
        return codegenGlobalVariable(llvm::cast<GlobalVariable>(value));
    case ValueKind::ConstantString:
        return codegenConstantString(llvm::cast<ConstantString>(value));
    case ValueKind::ConstantInt:
        return codegenConstantInt(llvm::cast<ConstantInt>(value));
    case ValueKind::ConstantFP:
        return codegenConstantFP(llvm::cast<ConstantFP>(value));
    case ValueKind::ConstantBool:
        return codegenConstantBool(llvm::cast<ConstantBool>(value));
    case ValueKind::ConstantNull:
        return codegenConstantNull(llvm::cast<ConstantNull>(value));
    case ValueKind::Undefined:
        return codegenUndefined(llvm::cast<Undefined>(value));
    }
}

void CGenerator::collectUsedValues(const Function* function) {
    useCounts.clear();
    deadValues.clear();
    gotoTargets.clear();
    // Branch arguments feeding each block parameter. A dead parameter drops its
    // assignments, so its arguments lose those uses (see the worklist below).
    std::unordered_map<const Value*, std::vector<const Value*>> paramArgs;
    for (auto* block : function->body) {
        for (auto* inst : block->body) {
            forEachOperand(inst, [&](const Value* value) { ++useCounts[value]; });
            switch (inst->kind) {
            case ValueKind::BranchInst: {
                auto* branch = llvm::cast<BranchInst>(inst);
                gotoTargets.insert(branch->destination);
                if (branch->destination->parameter && branch->argument) {
                    paramArgs[branch->destination->parameter].push_back(branch->argument);
                } else if (branch->argument) {
                    --useCounts[branch->argument]; // counted above, but no assignment exists
                }
                break;
            }
            case ValueKind::CondBranchInst: {
                auto* branch = llvm::cast<CondBranchInst>(inst);
                gotoTargets.insert(branch->trueBlock);
                gotoTargets.insert(branch->falseBlock);
                if (branch->argument) {
                    bool hasTrue = branch->trueBlock->parameter != nullptr;
                    bool hasFalse = branch->falseBlock->parameter != nullptr;
                    if (hasTrue) paramArgs[branch->trueBlock->parameter].push_back(branch->argument);
                    if (hasFalse) paramArgs[branch->falseBlock->parameter].push_back(branch->argument);
                    // forEachOperand counts the shared argument once; each emitted
                    // assignment is a separate use.
                    useCounts[branch->argument] += (hasTrue ? 1 : 0) + (hasFalse ? 1 : 0) - 1;
                }
                break;
            }
            case ValueKind::SwitchInst: {
                auto* switchInst = llvm::cast<SwitchInst>(inst);
                for (auto& c : switchInst->cases)
                    gotoTargets.insert(c.second);
                gotoTargets.insert(switchInst->defaultBlock);
                break;
            }
            default:
                break;
            }
        }
    }
    // Dropping a dead value orphans its operands, so iterate to a fixpoint.
    // Function parameters may be marked dead too; nothing consults deadValues
    // for them (silencing uses counts), so it is harmless.
    std::vector<const Value*> worklist;
    auto killIfUnused = [&](const Value* value) {
        if (value && useCounts[value] == 0 && (isPureTemp(value->kind) || value->kind == ValueKind::Parameter)) worklist.push_back(value);
    };
    for (auto* block : function->body) {
        for (auto* inst : block->body)
            killIfUnused(inst);
        killIfUnused(block->parameter);
    }
    while (!worklist.empty()) {
        const Value* dead = worklist.back();
        worklist.pop_back();
        if (!deadValues.insert(dead).second) continue;
        if (dead->kind == ValueKind::Parameter) {
            for (const Value* arg : paramArgs[dead]) {
                if (--useCounts[arg] == 0) killIfUnused(arg);
            }
            continue;
        }
        forEachOperand(llvm::cast<Instruction>(dead), [&](const Value* operand) {
            if (--useCounts[operand] == 0) killIfUnused(operand);
        });
    }
}

void CGenerator::silenceUnusedParams(const Function* function) {
    for (auto& param : function->params) {
        if (!param.type->isArrayType() && useCounts[&param] == 0) {
            stream.indent(4) << "(void)" << param.name << ";\n";
        }
    }
}

void CGenerator::codegenFunctionPrototype(const Function* function) {
    collectUsedValues(function);
    bool returnsArray = function->returnType->isArrayType();
    auto emitParameters = [&] {
        if (returnsArray) {
            codegenTempDeclarationForType(function->returnType->getPointerTo(), "_cx_return");
            if (!function->params.empty() || function->isVariadic) stream << ", ";
        } else if (function->params.empty() && !function->isVariadic) {
            // An empty parameter list means "unspecified arguments" in C, so spell out 'void' instead.
            stream << "void";
        }
        for (auto& param : function->params) {
            codegenTempDeclarationForType(param.type, param.name);
            if (&param != &function->params.back()) stream << ", ";
        }
        if (function->isVariadic) stream << ", ...";
    };

    if (returnsArray) {
        // C has no array return type. Use the usual hidden result pointer ABI and
        // keep the cx-level return value as a normal array at call sites.
        auto* array = llvm::cast<IRArrayType>(function->returnType);
        codegenType(stream, array->elementType, !function->isExtern);
        stream << " (*" << getCFunctionName(function) << '(';
        emitParameters();
        stream << "))";
        codegenTypeSuffix(stream, array, !function->isExtern);
        return;
    }

    std::vector<IRPointerType*> returnPointers;
    IRType* returnArray = function->returnType;
    while (auto* pointerType = llvm::dyn_cast<IRPointerType>(returnArray)) {
        returnPointers.push_back(pointerType);
        returnArray = pointerType->pointee;
    }

    if (!returnPointers.empty()) {
        if (auto* array = llvm::dyn_cast<IRArrayType>(returnArray)) {
            codegenType(stream, array->elementType, !function->isExtern);
            stream << " (";
            for (auto* pointerType : llvm::reverse(returnPointers)) {
                stream << '*';
                if (!pointerType->mutablePointee) stream << " const";
            }
            stream << getCFunctionName(function) << '(';
            emitParameters();
            stream << "))";
            codegenTypeSuffix(stream, array, !function->isExtern);
            return;
        }
    }

    codegenType(stream, function->returnType, !function->isExtern);
    stream << ' ' << getCFunctionName(function) << '(';
    emitParameters();
    stream << ')';
    codegenTypeSuffix(stream, function->returnType, !function->isExtern);
}

void CGenerator::codegenFunction(const Function* function) {
    stream << '\n';
    codegenFunctionPrototype(function);
    if (function->isExtern) {
        stream << ';';
    } else if (dispatchMode) {
        codegenFunctionDispatch(function);
        stream << '\n';
        return;
    } else {
        stream << " {\n";
        resetValueNaming(function);
        collectBlockParams(function);
        copyArrayParams(function);
        silenceUnusedParams(function);
        for (auto* block : function->body) {
            codegenBasicBlock(block);
        }
        stream << '}';
    }
    stream << '\n';
}

static std::string sanitizeBlockParamName(llvm::StringRef name) {
    // Basic block parameters are compiler-generated (e.g. "and", "or",
    // "if.result"), and their raw names are not all valid C identifiers ("and"
    // and "or" are alternative tokens, '.' is not allowed in identifiers), so
    // give them a dedicated prefix as well as replacing invalid characters.
    std::string result = "_cxp_";
    for (char ch : name) {
        result += (std::isalnum(static_cast<unsigned char>(ch)) || ch == '_') ? ch : '_';
    }
    return result;
}

const std::string& CGenerator::getBlockParamName(const Parameter* param) {
    auto it = emittedValues.find(param);
    if (it != emittedValues.end()) {
        return it->second;
    }
    // Several block parameters in one function can share a name (every
    // ternary produces an "if.result" parameter), so disambiguate repeats
    // with a numeric suffix.
    std::string base = sanitizeBlockParamName(param->name);
    std::string name = base;
    for (int suffix = 0; usedValueNames.contains(name); ++suffix) {
        name = base + std::to_string(suffix);
    }
    usedValueNames.insert(name);
    return emittedValues.insert({param, std::move(name)}).first->second;
}

void CGenerator::collectBlockParams(const Function* function) {
    for (auto* block : function->body) {
        if (block->parameter && deadValues.contains(block->parameter)) continue;
        if (block->parameter && !emittedValues.contains(block->parameter)) {
            const std::string& name = getBlockParamName(block->parameter);
            stream.indent(4);
            codegenDeclaration(stream, block->parameter->type, name, true);
            stream << ";\n";
        }
    }
}

void CGenerator::copyArrayParams(const Function* function) {
    for (auto& param : function->params) {
        if (!param.type->isArrayType()) continue;
        std::string copyName = "_cx_arg_" + param.name;
        while (!usedValueNames.insert(copyName).second)
            copyName += "_";
        stream.indent(4);
        codegenDeclaration(stream, param.type, copyName, true);
        stream << ";\n";
        stream.indent(4) << "memcpy(" << copyName << ", " << param.name << ", sizeof(" << copyName << "));\n";
        emittedValues[&param] = copyName;
    }
}

void CGenerator::codegenFunctionDispatch(const Function* function) {
    stream << " {\n";
    resetValueNaming(function);
    copyArrayParams(function);
    silenceUnusedParams(function);
    dispatchBlockIds.clear();

    int id = 0;
    for (auto* block : function->body) {
        dispatchBlockIds[block] = id++;
    }

    // Hoist declarations for basic block parameters and temporaries, so that
    // the dispatch cases below contain only assignments and control flow.
    collectBlockParams(function);
    for (auto* block : function->body) {
        for (auto* inst : block->body) {
            switch (inst->kind) {
            case ValueKind::AllocaInst: {
                auto* alloca = llvm::cast<AllocaInst>(inst);
                auto name = claimSuffixedName(!alloca->name.empty() ? alloca->name : "_alloca");
                stream.indent(4);
                codegenDeclaration(stream, alloca->allocatedType, name, true);
                stream << ";\n";
                emittedValues.insert({inst, "(&" + std::move(name) + ")"});
                break;
            }
            case ValueKind::CallInst: {
                auto* call = llvm::cast<CallInst>(inst);
                if (!hasReturnValue(call)) break;
                if (!call->getType()->isArrayType() && useCounts[call] == 0) break;
                auto name = claimSuffixedName("_call");
                stream.indent(4);
                codegenTempDeclarationForType(call->getType(), name);
                stream << ";\n";
                emittedValues.insert({inst, std::move(name)});
                break;
            }
            case ValueKind::LoadInst:
            case ValueKind::ExtractInst:
            case ValueKind::BinaryInst:
            case ValueKind::UnaryInst:
            case ValueKind::CastInst: {
                if (deadValues.contains(inst)) break;
                auto name = claimSuffixedName(inst->kind == ValueKind::LoadInst      ? "_load"
                                              : inst->kind == ValueKind::ExtractInst ? "_extract"
                                              : inst->kind == ValueKind::BinaryInst  ? "_binary_op"
                                              : inst->kind == ValueKind::UnaryInst   ? "_unary_op"
                                                                                     : "_cast");
                stream.indent(4);
                codegenTempDeclarationForType(inst->getType(), name);
                stream << ";\n";
                emittedValues.insert({inst, std::move(name)});
                break;
            }
            case ValueKind::InsertInst: {
                auto* insert = llvm::cast<InsertInst>(inst);
                auto* type = insert->aggregate->getType();
                auto name = claimSuffixedName("_insert");
                stream.indent(4);
                codegenDeclaration(stream, type, name, true);
                stream << ";\n";
                emittedValues.insert({inst, std::move(name)});
                break;
            }
            case ValueKind::GEPInst:
            case ValueKind::ConstGEPInst: {
                if (deadValues.contains(inst)) break;
                auto prefix = inst->kind == ValueKind::GEPInst ? "_get_element_ptr" : "_const_get_element_ptr";
                auto name = claimSuffixedName(prefix);
                stream.indent(4);
                codegenTempDeclarationForType(inst->getType(), name);
                stream << ";\n";
                emittedValues.insert({inst, std::move(name)});
                break;
            }
            default:
                break;
            }
        }
    }

    stream.indent(4) << "int _cx_pc = 0;\n";
    stream.indent(4) << "while (1) {\n";
    stream.indent(8) << "switch (_cx_pc) {\n";
    for (auto* block : function->body) {
        stream.indent(8) << "case " << dispatchBlockIds[block] << ": {\n";
        for (auto* inst : block->body) {
            // Bypass the emittedValues short-circuit in codegenInst: every
            // name is already registered by the hoisting pass above, and here
            // we want the assignment, not just the name.
            codegenInstImpl(inst);
        }
        stream.indent(8) << "}\n";
    }
    stream.indent(8) << "default: abort();\n";
    stream.indent(8) << "}\n";
    stream.indent(4) << "}\n";
    stream << '}';
}

void CGenerator::codegenType(llvm::raw_string_ostream& stream, IRType* type, bool needsTypeDefinition) {
    switch (type->kind) {
    case IRTypeKind::IRBasicType: {
        auto* basicType = llvm::cast<IRBasicType>(type);
        llvm::StringRef cType = llvm::StringSwitch<llvm::StringRef>(basicType->name)
                                    .Case("void", "void")
                                    .Case("bool", "_Bool")
                                    .Case("char", "char")
                                    .Case("int8", "int8_t")
                                    .Case("int16", "int16_t")
                                    .Case("int32", "int32_t")
                                    .Case("int64", "int64_t")
                                    .Case("int128", "__int128")
                                    .Case("uint8", "uint8_t")
                                    .Case("uint16", "uint16_t")
                                    .Case("uint32", "uint32_t")
                                    .Case("uint64", "uint64_t")
                                    .Case("uint128", "unsigned __int128")
                                    .Case("c_size_t", "size_t")
                                    .Case("c_schar", "signed char")
                                    .Case("c_uchar", "unsigned char")
                                    .Case("c_short", "short")
                                    .Case("c_ushort", "unsigned short")
                                    .Case("c_int", "int")
                                    .Case("c_uint", "unsigned int")
                                    .Case("c_long", "long")
                                    .Case("c_ulong", "unsigned long")
                                    .Case("c_longlong", "long long")
                                    .Case("c_ulonglong", "unsigned long long")
                                    .Case("c_float", "float")
                                    .Case("c_double", "double")
                                    .Case("float32", "float")
                                    .Case("float64", "double")
                                    .Case("float80", "__float80")
                                    .Default(llvm::StringRef());
        if (cType.empty()) {
            llvm_unreachable("unhandled type");
        } else {
            stream << cType;
        }
        break;
    }
    case IRTypeKind::IRPointerType: {
        auto* pointerType = llvm::cast<IRPointerType>(type);
        codegenType(stream, pointerType->pointee, needsTypeDefinition);
        if (!pointerType->mutablePointee) stream << " const";
        stream << '*';
        break;
    }
    case IRTypeKind::IRFunctionType: {
        auto* functionType = llvm::cast<IRFunctionType>(type);
        codegenType(stream, functionType->returnType, false);
        stream << "(";
        // Rest of the type will be generated by codegenTypeSuffix.
        break;
    }
    case IRTypeKind::IRArrayType: {
        auto* arrayType = llvm::cast<IRArrayType>(type);
        codegenType(stream, arrayType->elementType, needsTypeDefinition);
        // Array size will be generated by codegenTypeSuffix.
        break;
    }
    case IRTypeKind::IRStructType: {
        auto* irStruct = llvm::cast<IRStructType>(type);
        if (irStruct->name == "never") {
            stream << "void";
        } else if (isCFileType(irStruct)) {
            stream << "FILE";
        } else {
            codegenTypeDefinition(preludeStream, type, needsTypeDefinition);
            stream << "struct " << getOrCreateTypeName(type, irStruct->mangledName, "_cx_struct");
        }
        break;
    }
    case IRTypeKind::IRUnionType: {
        auto* unionType = llvm::cast<IRUnionType>(type);
        codegenTypeDefinition(preludeStream, type, needsTypeDefinition);
        stream << "union " << getOrCreateTypeName(type, unionType->name, "_cx_union");
        break;
    }
    }
}

void CGenerator::codegenTypeSuffix(llvm::raw_string_ostream& stream, IRType* type, bool) {
    switch (type->kind) {
    case IRTypeKind::IRArrayType: {
        auto* arrayType = llvm::cast<IRArrayType>(type);
        // MSVC rejects zero-size arrays (C2466); over-allocate one dummy
        // element instead. It is never accessed: indexing is bounds-checked.
        stream << "[" << (arrayType->size == 0 ? 1 : arrayType->size) << "]";
        codegenTypeSuffix(stream, arrayType->elementType, false);
        break;
    }
    case IRTypeKind::IRFunctionType: {
        auto* functionType = llvm::cast<IRFunctionType>(type);
        stream << ")(";
        if (functionType->paramTypes.empty() && !functionType->isVariadic) {
            stream << "void";
        }
        for (auto& paramType : functionType->paramTypes) {
            codegenType(stream, paramType, false);
            codegenTypeSuffix(stream, paramType, false);
            if (&paramType != &functionType->paramTypes.back()) stream << ", ";
        }
        stream << ")";
        break;
    }
    case IRTypeKind::IRPointerType: {
        auto* pointerType = llvm::cast<IRPointerType>(type);
        codegenTypeSuffix(stream, pointerType->pointee, false);
        break;
    }
    default:
        break;
    }
}

void CGenerator::codegenTypeDefinition(llvm::raw_string_ostream& stream, IRType* type, bool define) {
    switch (type->kind) {
    case IRTypeKind::IRBasicType:
        break;
    case IRTypeKind::IRPointerType:
        // Pointers only need their pointee declared, which also cuts reference cycles.
        codegenTypeDefinition(stream, llvm::cast<IRPointerType>(type)->pointee, false);
        break;
    case IRTypeKind::IRFunctionType:
        codegenTypeDefinition(stream, llvm::cast<IRFunctionType>(type)->returnType, define);
        for (auto paramType : llvm::cast<IRFunctionType>(type)->paramTypes) {
            codegenTypeDefinition(stream, paramType, define);
        }
        break;
    case IRTypeKind::IRArrayType:
        codegenTypeDefinition(stream, llvm::cast<IRArrayType>(type)->elementType, define);
        break;
    case IRTypeKind::IRStructType: {
        auto* irStruct = llvm::cast<IRStructType>(type);
        if (irStruct->isImportedFromC || isCFileType(irStruct) || alreadyEmittedTypes.contains(type)) break;
        if (!forwardDeclaredTypes.contains(type)) {
            stream << "\nstruct " << getOrCreateTypeName(type, irStruct->mangledName, "_cx_struct") << ";\n";
            forwardDeclaredTypes.insert(type);
        }
        if (!define) break;
        // Mark as emitted before generating dependencies: re-entry happens only through pointers,
        // for which the forward declaration above suffices. By-value cycles are rejected during typechecking.
        alreadyEmittedTypes.insert(type);

        // Generate type dependencies first.
        for (auto& field : irStruct->fields) {
            codegenTypeDefinition(stream, field.type, true);
        }

        stream << "\nstruct " << getOrCreateTypeName(type, irStruct->mangledName, "_cx_struct") << " {\n";
        if (irStruct->fields.empty()) {
            // Empty structs are a GNU extension, so add a placeholder member to stay
            // compatible with strictly conforming C compilers. Empty structs carry
            // no data and are only ever used through pointers, so this is harmless.
            stream.indent(4);
            stream << "char _cx_empty;\n";
        }
        for (auto& field : irStruct->fields) {
            stream.indent(4);
            // Pointer members only need their pointee declared, which also keeps in-progress
            // ancestor types from being re-entered here; by-value members need full definitions.
            codegenDeclaration(stream, field.type, field.name, !field.type->isPointerType());
            stream << ";\n";
        }
        stream << "};\n";
        break;
    }
    case IRTypeKind::IRUnionType: {
        auto* unionType = llvm::cast<IRUnionType>(type);
        if (alreadyEmittedTypes.contains(type)) break;
        if (!forwardDeclaredTypes.contains(type)) {
            stream << "\nunion " << getOrCreateTypeName(type, unionType->name, "_cx_union") << ";\n";
            forwardDeclaredTypes.insert(type);
        }
        if (!define) break;
        alreadyEmittedTypes.insert(type);

        // Generate type dependencies first.
        for (auto& field : unionType->fields) {
            codegenTypeDefinition(stream, field.type, true);
        }

        // Named unions are defined in C headers; only anonymous enum payload unions need definitions here.
        if (unionType->name.empty()) {
            stream << "\nunion " << getOrCreateTypeName(type, unionType->name, "_cx_union") << " {\n";
            for (auto& field : unionType->fields) {
                stream.indent(4);
                codegenDeclaration(stream, field.type, field.name, !field.type->isPointerType());
                stream << ";\n";
            }
            stream << "};\n";
        }
        break;
    }
    }
}

std::string CGenerator::finish() {
    return "#include <stdio.h>\n"
           "#include <stdint.h>\n"
           "#include <stdlib.h>\n"
           "#include <string.h>\n"
           "#include <stdbool.h>\n"
           "#include <math.h>\n"
           "#ifdef __wasm\n"
           "// xcc's WebAssembly libc neither declares nor defines abort(), so map\n"
           "// it to exit() (which terminates the process via WASI). Other\n"
           "// toolchains are unaffected by this fallback.\n"
           "static void cx_wasm_abort(void) {\n"
           "    exit(1);\n"
           "}\n"
           "#define abort cx_wasm_abort\n"
           "// The glibc-specific backtrace API is unavailable to WebAssembly\n"
           "// toolchains, so provide no-op stubs (stack traces are simply empty).\n"
           "int backtrace(void** array, int size) {\n"
           "    (void)array;\n"
           "    (void)size;\n"
           "    return 0;\n"
           "}\n"
           "char** backtrace_symbols(void** array, int size) {\n"
           "    (void)array;\n"
           "    (void)size;\n"
           "    return 0;\n"
           "}\n"
           "// xcc's WebAssembly libc lacks single-precision math and double-precision\n"
           "// exp2/cbrt/hypot/trunc, so provide them here in terms of what it has.\n"
           "double sin(double x);\n"
           "double cos(double x);\n"
           "double tan(double x);\n"
           "double asin(double x);\n"
           "double acos(double x);\n"
           "double atan(double x);\n"
           "double atan2(double y, double x);\n"
           "double sinh(double x);\n"
           "double cosh(double x);\n"
           "double tanh(double x);\n"
           "double exp(double x);\n"
           "double log(double x);\n"
           "double log2(double x);\n"
           "double log10(double x);\n"
           "double pow(double x, double y);\n"
           "double sqrt(double x);\n"
           "double ceil(double x);\n"
           "double floor(double x);\n"
           "double round(double x);\n"
           "double fmod(double x, double y);\n"
           "double exp2(double x) { return pow(2.0, x); }\n"
           "double cbrt(double x) { return x < 0.0 ? -pow(-x, 1.0 / 3.0) : pow(x, 1.0 / 3.0); }\n"
           "double hypot(double x, double y) {\n"
           "    double ax = x < 0.0 ? -x : x;\n"
           "    double ay = y < 0.0 ? -y : y;\n"
           "    double hi = ax > ay ? ax : ay;\n"
           "    double lo = ax > ay ? ay : ax;\n"
           "    if (hi == 0.0) return 0.0;\n"
           "    double r = lo / hi;\n"
           "    return hi * sqrt(1.0 + r * r);\n"
           "}\n"
           "double trunc(double x) { return x >= 0.0 ? floor(x) : ceil(x); }\n"
           "float sinf(float x) { return (float)sin((double)x); }\n"
           "float cosf(float x) { return (float)cos((double)x); }\n"
           "float tanf(float x) { return (float)tan((double)x); }\n"
           "float asinf(float x) { return (float)asin((double)x); }\n"
           "float acosf(float x) { return (float)acos((double)x); }\n"
           "float atanf(float x) { return (float)atan((double)x); }\n"
           "float atan2f(float y, float x) { return (float)atan2((double)y, (double)x); }\n"
           "float sinhf(float x) { return (float)sinh((double)x); }\n"
           "float coshf(float x) { return (float)cosh((double)x); }\n"
           "float tanhf(float x) { return (float)tanh((double)x); }\n"
           "float expf(float x) { return (float)exp((double)x); }\n"
           "float exp2f(float x) { return (float)exp2((double)x); }\n"
           "float logf(float x) { return (float)log((double)x); }\n"
           "float log2f(float x) { return (float)log2((double)x); }\n"
           "float log10f(float x) { return (float)log10((double)x); }\n"
           "float powf(float x, float y) { return (float)pow((double)x, (double)y); }\n"
           "float sqrtf(float x) { return (float)sqrt((double)x); }\n"
           "float cbrtf(float x) { return (float)cbrt((double)x); }\n"
           "float hypotf(float x, float y) { return (float)hypot((double)x, (double)y); }\n"
           "float ceilf(float x) { return (float)ceil((double)x); }\n"
           "float floorf(float x) { return (float)floor((double)x); }\n"
           "float truncf(float x) { return (float)trunc((double)x); }\n"
           "float roundf(float x) { return (float)round((double)x); }\n"
           "float fmodf(float x, float y) { return (float)fmod((double)x, (double)y); }\n"
           "// xcc's WebAssembly libc lacks strtof, so provide it in terms of strtod.\n"
           "float strtof(const char* s, char** e) { return (float)strtod(s, e); }\n"
           "// WASI has no shell, so popen/pclose/system are unavailable (xcc's\n"
           "// headers don't declare them on __wasm either). Provide stubs so\n"
           "// programs that don't use them still compile and link; calling\n"
           "// them returns failure.\n"
           "FILE* popen(const char* command, const char* mode) {\n"
           "    (void)command;\n"
           "    (void)mode;\n"
           "    return 0;\n"
           "}\n"
           "int pclose(FILE* stream) {\n"
           "    (void)stream;\n"
           "    return -1;\n"
           "}\n"
           "int system(const char* command) {\n"
           "    (void)command;\n"
           "    return -1;\n"
           "}\n"
           "// WASI has no access/setenv either (xcc declares neither); stub\n"
           "// them the same way so the file-system and environment helpers\n"
           "// link even when unused.\n"
           "int access(const char* path, int mode) {\n"
           "    (void)path;\n"
           "    (void)mode;\n"
           "    return -1;\n"
           "}\n"
           "int setenv(const char* name, const char* value, int overwrite) {\n"
           "    (void)name;\n"
           "    (void)value;\n"
           "    (void)overwrite;\n"
           "    return -1;\n"
           "}\n"
           "#endif\n"
         + preludeStream.str() + stream.str();
}
