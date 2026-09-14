#include "c-backend.h"
#include <algorithm>
#include <cctype>
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
    auto name = (!inst->name.empty() ? inst->name : "_alloca") + std::to_string(valueSuffixCounter++);
    if (dispatchMode) {
        // The declaration is hoisted (see codegenFunctionDispatch); only register the name.
        emittedValues.insert({inst, "(&" + name + ")"});
        return;
    }
    stream.indent(4);
    codegenType(stream, inst->allocatedType, true);
    stream << ' ';
    stream << name;
    codegenTypeSuffix(stream, inst->allocatedType, true);
    stream << ";\n";
    emittedValues.insert({inst, "(&" + std::move(name) + ")"});
}

void CGenerator::codegenReturn(const ReturnInst* inst) {
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
        codegenType(stream, value->getType(), true);
        codegenTypeSuffix(stream, value->getType(), true);
        stream << "){0}";
    } else {
        codegenInst(value);
    }
}

void CGenerator::codegenBranch(const BranchInst* inst) {
    if (inst->argument && inst->destination->parameter) {
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
    if (block->parameter && argument) {
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
    stream.indent(4);
    const std::string& name = getOrCreateTempName(inst, "_load");
    // Emit an explicit type instead of the '__auto_type' GNU extension,
    // so that the generated code can also be compiled with small,
    // strictly conforming C compilers (e.g. the one used by the web playground).
    codegenTempDeclaration(inst, name);
    stream << " = *";
    codegenInst(inst->value);
    stream << ";\n";
}

void CGenerator::codegenStore(const StoreInst* inst) {
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
        codegenType(stream, type, true);
        stream << " " << name;
        codegenTypeSuffix(stream, type, true);
        stream << "; ";
    }
    if (inst->aggregate->kind != ValueKind::Undefined) {
        stream << "memcpy(&" << name << ", &";
        codegenInst(inst->aggregate);
        stream << ", sizeof(" << name << ")); ";
    }
    stream << name;
    if (type->isArrayType()) {
        stream << "[" << inst->index << "] = ";
    } else {
        // Named structs use field names (see codegenTypeDefinition); anonymous tuples fall back to indices.
        const auto& fieldName = type->getFields()[inst->index].name;
        ASSERT(inst->index < (int)type->getFields().size());
        stream << "." << (fieldName.empty() ? "_" + std::to_string(inst->index) : fieldName) << " = ";
    }
    codegenInst(inst->value);
    stream << ";\n";
}

void CGenerator::codegenExtract(const ExtractInst* inst) {
    stream.indent(4);
    const std::string& name = getOrCreateTempName(inst, "_extract");
    codegenTempDeclaration(inst, name);
    stream << " = ";
    codegenInst(inst->aggregate);
    stream << "." << inst->name;
    stream << ";\n";
}

void CGenerator::codegenCall(const CallInst* inst) {
    stream.indent(4);
    if (hasReturnValue(inst)) {
        const std::string& name = getOrCreateTempName(inst, "_call");
        if (dispatchMode) {
            // The declaration is hoisted (see codegenFunctionDispatch).
            stream << name << " = ";
        } else {
            codegenTempDeclarationForType(inst->function->getType()->getPointee()->getReturnType(), name);
            stream << " = ";
        }
    }
    codegenInst(inst->function);
    stream << '(';
    for (size_t i = 0; i < inst->args.size(); ++i) {
        codegenArgument(inst->args[i]);
        if (i + 1 < inst->args.size()) stream << ", ";
    }
    stream << ");\n";
}

void CGenerator::codegenBinary(const BinaryInst* inst) {
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
        llvm_unreachable("all cases handled");
    }
    stream << ' ';
    codegenInst(inst->right);
    stream << ";\n";
}

void CGenerator::codegenUnary(const UnaryInst* inst) {
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
    stream.indent(4);
    const std::string& name = getOrCreateTempName(inst, "_get_element_ptr");
    if (dispatchMode) {
        // The declaration is hoisted (see codegenFunctionDispatch).
        stream << name << " = &(";
    } else {
        codegenType(stream, inst->getType(), true);
        stream << " " << name << " = &(";
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
        stream << "->" << inst->name << ";\n";
    }
}

void CGenerator::codegenCast(const CastInst* inst) {
    stream.indent(4);
    const std::string& name = getOrCreateTempName(inst, "_cast");
    codegenTempDeclaration(inst, name);
    stream << " = ";
    stream << "(";
    codegenType(stream, inst->type, true);
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
    codegenType(stream, inst->type, true);
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

void CGenerator::codegenTempDeclarationForType(IRType* type, const std::string& name) {
    if (type->isPointerType() && type->getPointee()->isArrayType()) {
        // A pointer to an array needs parenthesized declarator syntax:
        // `int* name[4]` would declare an array of pointers instead.
        IRType* elementType = type->getPointee();
        std::vector<int> dimensions;
        while (auto* arrayType = llvm::dyn_cast<IRArrayType>(elementType)) {
            dimensions.push_back(arrayType->getArraySize());
            elementType = arrayType->getElementType();
        }
        codegenType(stream, elementType, true);
        stream << " (*" << name << ")";
        for (int size : dimensions) {
            stream << "[" << size << "]";
        }
        return;
    }
    codegenType(stream, type, true);
    stream << ' ' << name;
    codegenTypeSuffix(stream, type, true);
}

void CGenerator::codegenBasicBlock(const BasicBlock* block) {
    if (!dispatchMode && !block->name.empty()) {
        // Extra semicolon to work around "label followed by a declaration is a C23 extension".
        stream << '\n' << getBlockLabel(block) << ": ;\n";
    }
    for (auto* inst : block->body) {
        codegenInst(inst);
    }
}

void CGenerator::codegenGlobalVariable(const GlobalVariable* inst) {
    codegenType(stream, inst->type, true);
    stream << ' ' << inst->name;
    codegenTypeSuffix(stream, inst->type, true);
    stream << ";\n";
    emittedValues.insert({inst, "(&" + inst->name + ")"});
}

void CGenerator::codegenConstantString(const ConstantString* inst) {
    stream << '"';
    stream.write_escaped(inst->value);
    stream << '"';
}

void CGenerator::codegenConstantInt(const ConstantInt* inst) {
    stream << inst->value;
}

void CGenerator::codegenConstantFP(const ConstantFP* inst) {
    llvm::SmallString<128> str;
    inst->value.toString(str);
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
    return emittedValues.insert({inst, prefix.str() + std::to_string(valueSuffixCounter++)}).first->second;
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
        stream << llvm::cast<Function>(value)->mangledName;
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

void CGenerator::codegenFunctionPrototype(const Function* function) {
    codegenType(stream, function->returnType, !function->isExtern);
    stream << ' ' << function->mangledName << '(';
    if (function->params.empty() && !function->isVariadic) {
        // An empty parameter list means "unspecified arguments" in C, so spell out 'void' instead.
        stream << "void";
    }
    for (auto& param : function->params) {
        codegenType(stream, param.type, !function->isExtern);
        stream << ' ' << param.name;
        codegenTypeSuffix(stream, param.type, !function->isExtern);
        if (&param != &function->params.back()) stream << ", ";
    }
    if (function->isVariadic) stream << ", ...";
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
        valueSuffixCounter = 0;
        collectBlockParams(function);
        copyArrayParams(function);
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
    return emittedValues.insert({param, sanitizeBlockParamName(param->name)}).first->second;
}

void CGenerator::collectBlockParams(const Function* function) {
    for (auto* block : function->body) {
        if (block->parameter && !emittedValues.contains(block->parameter)) {
            const std::string& name = getBlockParamName(block->parameter);
            stream.indent(4);
            codegenType(stream, block->parameter->type, true);
            stream << ' ' << name;
            codegenTypeSuffix(stream, block->parameter->type, true);
            stream << ";\n";
        }
    }
}

void CGenerator::copyArrayParams(const Function* function) {
    for (auto& param : function->params) {
        if (!param.type->isArrayType()) continue;
        std::string copyName = "_cx_arg_" + param.name;
        stream.indent(4);
        codegenType(stream, param.type, true);
        stream << ' ' << copyName;
        codegenTypeSuffix(stream, param.type, true);
        stream << ";\n";
        stream.indent(4) << "memcpy(" << copyName << ", " << param.name << ", sizeof(" << copyName << "));\n";
        emittedValues[&param] = copyName;
    }
}

void CGenerator::codegenFunctionDispatch(const Function* function) {
    stream << " {\n";
    valueSuffixCounter = 0;
    copyArrayParams(function);
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
                auto name = (!alloca->name.empty() ? alloca->name : "_alloca") + std::to_string(valueSuffixCounter++);
                stream.indent(4);
                codegenType(stream, alloca->allocatedType, true);
                stream << ' ' << name;
                codegenTypeSuffix(stream, alloca->allocatedType, true);
                stream << ";\n";
                emittedValues.insert({inst, "(&" + std::move(name) + ")"});
                break;
            }
            case ValueKind::CallInst: {
                auto* call = llvm::cast<CallInst>(inst);
                if (!hasReturnValue(call)) break;
                auto name = "_call" + std::to_string(valueSuffixCounter++);
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
                auto name = (inst->kind == ValueKind::LoadInst      ? "_load"
                             : inst->kind == ValueKind::ExtractInst ? "_extract"
                             : inst->kind == ValueKind::BinaryInst  ? "_binary_op"
                             : inst->kind == ValueKind::UnaryInst   ? "_unary_op"
                                                                    : "_cast")
                          + std::to_string(valueSuffixCounter++);
                stream.indent(4);
                codegenTempDeclarationForType(inst->getType(), name);
                stream << ";\n";
                emittedValues.insert({inst, std::move(name)});
                break;
            }
            case ValueKind::InsertInst: {
                auto* insert = llvm::cast<InsertInst>(inst);
                auto* type = insert->aggregate->getType();
                auto name = "_insert" + std::to_string(valueSuffixCounter++);
                stream.indent(4);
                codegenType(stream, type, true);
                stream << ' ' << name;
                codegenTypeSuffix(stream, type, true);
                stream << ";\n";
                emittedValues.insert({inst, std::move(name)});
                break;
            }
            case ValueKind::GEPInst:
            case ValueKind::ConstGEPInst: {
                auto prefix = inst->kind == ValueKind::GEPInst ? "_get_element_ptr" : "_const_get_element_ptr";
                auto name = prefix + std::to_string(valueSuffixCounter++);
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
                                    .Case("int", "int")
                                    .Case("int8", "int8_t")
                                    .Case("int16", "int16_t")
                                    .Case("int32", "int32_t")
                                    .Case("int64", "int64_t")
                                    .Case("int128", "__int128")
                                    .Case("uint", "unsigned")
                                    .Case("uint8", "uint8_t")
                                    .Case("uint16", "uint16_t")
                                    .Case("uint32", "uint32_t")
                                    .Case("uint64", "uint64_t")
                                    .Case("uint128", "unsigned __int128")
                                    .Case("float", "float")
                                    .Case("float16", "_Float16")
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
        } else {
            if (needsTypeDefinition) codegenTypeDefinition(preludeStream, type);
            stream << "struct " << irStruct->mangledName;
        }
        break;
    }
    case IRTypeKind::IRUnionType:
        stream << "union " << llvm::cast<IRUnionType>(type)->name;
        break;
    }
}

void CGenerator::codegenTypeSuffix(llvm::raw_string_ostream& stream, IRType* type, bool) {
    switch (type->kind) {
    case IRTypeKind::IRArrayType: {
        auto* arrayType = llvm::cast<IRArrayType>(type);
        stream << "[" << arrayType->size << "]";
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

void CGenerator::codegenTypeDefinition(llvm::raw_string_ostream& stream, IRType* type) {
    switch (type->kind) {
    case IRTypeKind::IRBasicType:
        break;
    case IRTypeKind::IRPointerType:
        codegenTypeDefinition(stream, llvm::cast<IRPointerType>(type)->pointee);
        break;
    case IRTypeKind::IRFunctionType:
        codegenTypeDefinition(stream, llvm::cast<IRFunctionType>(type)->returnType);
        for (auto paramType : llvm::cast<IRFunctionType>(type)->paramTypes) {
            codegenTypeDefinition(stream, paramType);
        }
        break;
    case IRTypeKind::IRArrayType:
        codegenTypeDefinition(stream, llvm::cast<IRArrayType>(type)->elementType);
        break;
    case IRTypeKind::IRStructType: {
        // Generate struct definition if this is the first time we encounter this type.
        auto* irStruct = llvm::cast<IRStructType>(type);
        if (!irStruct->isImportedFromC && !alreadyEmittedTypes.contains(type)) {
            alreadyEmittedTypes.insert(type);

            // Generate type dependencies first.
            for (auto& field : irStruct->fields) {
                codegenTypeDefinition(stream, field.type);
            }

            stream << "\nstruct " << irStruct->mangledName << " {\n";
            if (irStruct->fields.empty()) {
                // Empty structs are a GNU extension, so add a placeholder member to stay
                // compatible with strictly conforming C compilers. Empty structs carry
                // no data and are only ever used through pointers, so this is harmless.
                stream.indent(4);
                stream << "char _cx_empty;\n";
            }
            for (auto& field : irStruct->fields) {
                stream.indent(4);
                codegenType(stream, field.type, true);
                stream << " " << field.name;
                codegenTypeSuffix(stream, field.type, true);
                stream << ";\n";
            }
            stream << "};\n";
        }
        break;
    }
    case IRTypeKind::IRUnionType:
        for (auto& field : llvm::cast<IRUnionType>(type)->fields) {
            codegenTypeDefinition(stream, field.type);
        }
        break;
    }
}

std::string CGenerator::finish() {
    return "#include <stdio.h>\n"
           "#include <stdint.h>\n"
           "#include <stdlib.h>\n"
           "#include <string.h>\n"
           "#include <stdbool.h>\n"
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
           "#endif\n"
         + preludeStream.str() + stream.str();
}
