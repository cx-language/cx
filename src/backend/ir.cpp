#include "ir.h"
#pragma warning(push, 0)
#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/StringSet.h>
#include <llvm/ADT/StringSwitch.h>
#pragma warning(pop)
#include "../ast/decl.h"
#include "../ast/mangle.h"
#include "../ast/module.h"

using namespace cx;

BasicBlock::BasicBlock(std::string name, cx::Function* parent) : Value{ValueKind::BasicBlock}, name(std::move(name)), parent(parent) {
    if (parent) {
        parent->body.push_back(this);
    }
}

static std::unordered_map<TypeBase*, IRType*> irTypes = {{nullptr, nullptr}};

// Every allocated IR value and type, for bulk freeing below.
static std::vector<Value*> allIRValues;
static std::vector<IRType*> allIRTypes;

void* Value::operator new(size_t size) {
    void* ptr = ::operator new(size);
    allIRValues.push_back(static_cast<Value*>(ptr));
    return ptr;
}

// The failed object is always the most recently registered one: its braced
// initializer (whose evaluation threw) runs after its own allocation but
// before any later one.
void Value::operator delete(void* ptr) noexcept {
    ASSERT(!allIRValues.empty() && allIRValues.back() == ptr);
    if (!allIRValues.empty() && allIRValues.back() == ptr) allIRValues.pop_back();
    ::operator delete(ptr);
}

void* IRType::operator new(size_t size) {
    void* ptr = ::operator new(size);
    allIRTypes.push_back(static_cast<IRType*>(ptr));
    return ptr;
}

void IRType::operator delete(void* ptr) noexcept {
    ASSERT(!allIRTypes.empty() && allIRTypes.back() == ptr);
    if (!allIRTypes.empty() && allIRTypes.back() == ptr) allIRTypes.pop_back();
    ::operator delete(ptr);
}

// IR nodes have no virtual destructors (they must stay aggregates for braced
// initialization), so deletion dispatches on the kind instead. New node kinds
// must be added here or their members leak across resetIRState() calls.
static void deleteIRValue(Value* value) {
    switch (value->kind) {
    case ValueKind::AllocaInst:
        delete static_cast<AllocaInst*>(value);
        break;
    case ValueKind::ReturnInst:
        delete static_cast<ReturnInst*>(value);
        break;
    case ValueKind::BranchInst:
        delete static_cast<BranchInst*>(value);
        break;
    case ValueKind::CondBranchInst:
        delete static_cast<CondBranchInst*>(value);
        break;
    case ValueKind::SwitchInst:
        delete static_cast<SwitchInst*>(value);
        break;
    case ValueKind::LoadInst:
        delete static_cast<LoadInst*>(value);
        break;
    case ValueKind::StoreInst:
        delete static_cast<StoreInst*>(value);
        break;
    case ValueKind::InsertInst:
        delete static_cast<InsertInst*>(value);
        break;
    case ValueKind::ExtractInst:
        delete static_cast<ExtractInst*>(value);
        break;
    case ValueKind::CallInst:
        delete static_cast<CallInst*>(value);
        break;
    case ValueKind::BinaryInst:
        delete static_cast<BinaryInst*>(value);
        break;
    case ValueKind::UnaryInst:
        delete static_cast<UnaryInst*>(value);
        break;
    case ValueKind::GEPInst:
        delete static_cast<GEPInst*>(value);
        break;
    case ValueKind::ConstGEPInst:
        delete static_cast<ConstGEPInst*>(value);
        break;
    case ValueKind::CastInst:
        delete static_cast<CastInst*>(value);
        break;
    case ValueKind::UnreachableInst:
        delete static_cast<UnreachableInst*>(value);
        break;
    case ValueKind::ArrayOpInst:
        delete static_cast<ArrayOpInst*>(value);
        break;
    case ValueKind::SizeofInst:
        delete static_cast<SizeofInst*>(value);
        break;
    case ValueKind::CheckedArithInst:
        delete static_cast<CheckedArithInst*>(value);
        break;
    case ValueKind::ArithOverflowInst:
        delete static_cast<ArithOverflowInst*>(value);
        break;
    case ValueKind::SaturatingArithInst:
        delete static_cast<SaturatingArithInst*>(value);
        break;
    case ValueKind::BasicBlock:
        delete static_cast<BasicBlock*>(value);
        break;
    case ValueKind::Function:
        delete static_cast<Function*>(value);
        break;
    case ValueKind::Parameter:
        delete static_cast<Parameter*>(value);
        break;
    case ValueKind::GlobalVariable:
        delete static_cast<GlobalVariable*>(value);
        break;
    case ValueKind::ConstantString:
        delete static_cast<ConstantString*>(value);
        break;
    case ValueKind::ConstantInt:
        delete static_cast<ConstantInt*>(value);
        break;
    case ValueKind::ConstantFP:
        delete static_cast<ConstantFP*>(value);
        break;
    case ValueKind::ConstantBool:
        delete static_cast<ConstantBool*>(value);
        break;
    case ValueKind::ConstantNull:
        delete static_cast<ConstantNull*>(value);
        break;
    case ValueKind::ConstantIntToPtr:
        delete static_cast<ConstantIntToPtr*>(value);
        break;
    case ValueKind::Undefined:
        delete static_cast<Undefined*>(value);
        break;
    default:
        ABORT("unhandled IR value kind in deleteIRValue");
    }
}

static void deleteIRType(IRType* type) {
    switch (type->kind) {
    case IRTypeKind::IRBasicType:
        delete static_cast<IRBasicType*>(type);
        break;
    case IRTypeKind::IRPointerType:
        delete static_cast<IRPointerType*>(type);
        break;
    case IRTypeKind::IRFunctionType:
        delete static_cast<IRFunctionType*>(type);
        break;
    case IRTypeKind::IRArrayType:
        delete static_cast<IRArrayType*>(type);
        break;
    case IRTypeKind::IRStructType:
        delete static_cast<IRStructType*>(type);
        break;
    case IRTypeKind::IRUnionType:
        delete static_cast<IRUnionType*>(type);
        break;
    default:
        ABORT("unhandled IR type kind in deleteIRType");
    }
}

Type cx::getContextStructType() {
    auto* stdlib = Module::getStdlibModule();
    if (!stdlib) return Type();
    auto* decl = llvm::dyn_cast_or_null<TypeDecl>(stdlib->symbolTable.findOne("Context"));
    if (!decl) return Type();
    return decl->getType();
}

IRType* cx::getIRType(Type astType) {
    // Spelling twins are distinct bases sharing one identity; normalize so the
    // type cache below yields a single IR type per structure.
    astType = astType.canonicalTwin();
    auto it = irTypes.find(astType.typeBase);
    if (it != irTypes.end()) return it->second;

    IRType* irType = nullptr;

    switch (astType.getKind()) {
    case TypeKind::BasicType: {
        // Fixed-size arrays ("T[N]") lower to LLVM array types, not to the
        // fieldless stdlib Array struct layout.
        if (astType.isFixedArray()) {
            auto elementType = getIRType(astType.getElementType());
            if (astType.hasSizeofArraySize()) {
                irType = new IRArrayType{IRTypeKind::IRArrayType, elementType, -1, astType.getSizeofArrayOperand()};
            } else {
                ASSERT(astType.isConcreteArray());
                irType = new IRArrayType{IRTypeKind::IRArrayType, elementType, static_cast<int>(astType.getArraySize()), Type()};
            }
            break;
        }
        if (astType.isVoid() || Type::isBuiltinScalar(astType.getName())) {
            irType = new IRBasicType{IRTypeKind::IRBasicType, astType.getName().str()};
        } else if (astType.isOptionalType() && astType.isImplementedAsPointer()) {
            irType = getIRType(astType.getWrappedType());
        } else if (astType.isEnumType()) {
            auto enumDecl = llvm::cast<EnumDecl>(astType.getDecl());
            auto tagType = getIRType(enumDecl->getTagType());

            if (enumDecl->hasAssociatedValues()) {
                auto unionType = new IRUnionType{IRTypeKind::IRUnionType, {}, "", ""};
                irType = new IRStructType{IRTypeKind::IRStructType,
                                          {IRField{tagType, "tag"}, IRField{unionType, "payload"}},
                                          astType.getQualifiedTypeName(),
                                          '_' + mangleType(astType),
                                          false,
                                          false};
                irTypes.emplace(astType.typeBase, irType);
                // Cases without associated values carry no payload, so only cases with
                // associated types become union fields. Payload access casts the whole
                // union, so field indices don't matter.
                std::vector<IRField> associatedTypes;
                for (auto& enumCase : enumDecl->cases) {
                    if (enumCase.associatedType) {
                        associatedTypes.push_back(IRField{getIRType(enumCase.associatedType), enumCase.name.str()});
                    }
                }
                unionType->fields = std::move(associatedTypes);
                return irType;
            } else {
                irType = tagType;
            }
        } else if (auto decl = astType.getDecl()) {
            // C++ headers aren't included in generated code, so their records are emitted like cx records.
            // Generated anonymous records are defined by no header either.
            bool isImportedFromC = decl->module.isCHeaderImport && !decl->module.isCxxHeaderImport && !decl->isAnonymousRecord;
            if (decl->isUnion()) {
                auto unionType = new IRUnionType{IRTypeKind::IRUnionType,
                                                 {},
                                                 astType.getQualifiedTypeName(),
                                                 isImportedFromC ? astType.getName().str() : ('_' + mangleType(astType)),
                                                 isImportedFromC};
                irTypes.emplace(astType.typeBase, unionType);
                // Fields are set late to handle recursive types.
                unionType->fields = map(decl->fields, [](const FieldDecl& f) { return IRField{getIRType(f.type), f.name.str(), f.isAnonymousMember}; });
                return unionType;
            } else {
                auto structType = new IRStructType{IRTypeKind::IRStructType,
                                                   {},
                                                   astType.getQualifiedTypeName(),
                                                   isImportedFromC ? astType.getName().str() : ('_' + mangleType(astType)),
                                                   decl->packed,
                                                   isImportedFromC};
                irTypes.emplace(astType.typeBase, structType);
                // Fields are set late to handle recursive types.
                structType->fields = map(decl->fields, [](const FieldDecl& f) { return IRField{getIRType(f.type), f.name.str(), f.isAnonymousMember}; });
                return structType;
            }
        } else {
            llvm_unreachable(StringBuilder() << "unknown type '" << astType << "'");
        }
        break;
    }
    case TypeKind::ArrayPointerType:
        ASSERT(astType.isArrayPointer());
        irType = getIRType(PointerType::get(astType.getElementType(), PointerKind::Pointer));
        break;
    case TypeKind::AnonymousStructType: {
        auto fields = map(astType.getAnonymousStructElements(), [](const AnonymousStructElement& e) { return IRField{getIRType(e.type), e.name.str()}; });
        irType = new IRStructType{IRTypeKind::IRStructType, std::move(fields), std::string(), std::string(), false, false};
        break;
    }
    case TypeKind::FunctionType: {
        auto returnType = getIRType(astType.getReturnType());
        auto paramTypes = map(astType.getParamTypes(), [](Type t) { return getIRType(t); });
        // cx functions take the ambient context as a hidden first parameter;
        // extern ones use the plain C ABI. No stdlib (bare-env builds) means
        // no Context type, so functions stay contextless there.
        bool hasContextParam = false;
        if (!llvm::cast<FunctionType>(astType.typeBase)->isExtern) {
            if (Type contextType = getContextStructType()) {
                paramTypes.insert(paramTypes.begin(), getIRType(contextType)->getPointerTo());
                hasContextParam = true;
            }
        }
        auto functionType = new IRFunctionType{
            IRTypeKind::IRFunctionType, returnType, std::move(paramTypes), llvm::cast<FunctionType>(astType.typeBase)->isVariadic, hasContextParam,
        };
        irType = new IRPointerType{IRTypeKind::IRPointerType, functionType};
        break;
    }
    case TypeKind::PointerType: {
        auto pointeeType = getIRType(astType.getPointee());
        irType = new IRPointerType{IRTypeKind::IRPointerType, pointeeType};
        break;
    }
    case TypeKind::UnresolvedType:
        llvm_unreachable("cannot convert unresolved type to IR");
    }

    irTypes.emplace(astType.typeBase, irType);
    return irType;
}

IRType* Value::getType() const {
    switch (kind) {
    case ValueKind::AllocaInst:
        return llvm::cast<AllocaInst>(this)->allocatedType->getPointerTo();
    case ValueKind::ReturnInst:
        llvm_unreachable("unhandled ReturnInst");
    case ValueKind::BranchInst:
        llvm_unreachable("unhandled BranchInst");
    case ValueKind::CondBranchInst:
        llvm_unreachable("unhandled CondBranchInst");
    case ValueKind::SwitchInst:
        llvm_unreachable("unhandled SwitchInst");
    case ValueKind::LoadInst:
        return llvm::cast<LoadInst>(this)->value->getType()->getPointee();
    case ValueKind::StoreInst:
        llvm_unreachable("unhandled StoreInst");
    case ValueKind::InsertInst:
        return llvm::cast<InsertInst>(this)->aggregate->getType();
    case ValueKind::ExtractInst: {
        auto extract = llvm::cast<ExtractInst>(this);
        return extract->aggregate->getType()->getFields()[extract->index].type;
    }
    case ValueKind::CallInst: {
        auto functionType = llvm::cast<CallInst>(this)->function->getType();
        if (functionType->isPointerType()) {
            return functionType->getPointee()->getReturnType();
        } else {
            return functionType->getReturnType();
        }
    }
    case ValueKind::BinaryInst: {
        auto binary = llvm::cast<BinaryInst>(this);
        switch (binary->op) {
        case Token::AndAnd:
        case Token::OrOr:
        case Token::Equal:
        case Token::NotEqual:
        case Token::Less:
        case Token::LessOrEqual:
        case Token::Greater:
        case Token::GreaterOrEqual:
            return getIRType(Type::getBool());
        case Token::DotDot:
        case Token::DotDotDot:
            llvm_unreachable("range operators should be lowered");
        default:
            return binary->left->getType();
        }
    }
    case ValueKind::UnaryInst: {
        auto unary = llvm::cast<UnaryInst>(this);
        switch (unary->op) {
        case Token::Not:
            return getIRType(Type::getBool());
        default:
            return unary->operand->getType();
        }
    }
    case ValueKind::GEPInst: {
        auto gep = llvm::cast<GEPInst>(this);
        auto baseType = gep->pointer->getType();
        for (size_t i = 1; i < gep->indexes.size(); ++i) {
            switch (baseType->getPointee()->kind) {
            case IRTypeKind::IRArrayType:
                baseType = baseType->getPointee()->getElementType()->getPointerTo();
                break;
            default:
                llvm_unreachable("invalid non-const GEP target type");
            }
        }
        return baseType;
    }
    case ValueKind::ConstGEPInst: {
        auto gep = llvm::cast<ConstGEPInst>(this);
        auto baseType = gep->pointer->getType()->getPointee();
        switch (baseType->kind) {
        case IRTypeKind::IRStructType:
            ASSERT(gep->index < baseType->getFields().size());
            return baseType->getFields()[gep->index].type->getPointerTo();
        case IRTypeKind::IRArrayType:
            if (!llvm::cast<IRArrayType>(baseType)->hasSymbolicSize()) ASSERT(gep->index < baseType->getArraySize());
            return baseType->getElementType()->getPointerTo();
        default:
            llvm_unreachable("invalid const GEP target type");
        }
    }
    case ValueKind::CastInst:
        return llvm::cast<CastInst>(this)->type;
    case ValueKind::UnreachableInst:
        llvm_unreachable("unhandled UnreachableInst");
    case ValueKind::ArrayOpInst: {
        auto arrayOp = llvm::cast<ArrayOpInst>(this);
        if (arrayOp->op == Token::Equal || arrayOp->op == Token::NotEqual) {
            return getIRType(Type::getBool());
        }
        return arrayOp->arrayType->getPointerTo();
    }
    case ValueKind::SizeofInst:
        return llvm::cast<SizeofInst>(this)->resultType;
    case ValueKind::CheckedArithInst:
        return llvm::cast<CheckedArithInst>(this)->left->getType();
    case ValueKind::ArithOverflowInst:
        return getIRType(Type::getBool());
    case ValueKind::SaturatingArithInst:
        return llvm::cast<SaturatingArithInst>(this)->left->getType();
    case ValueKind::BasicBlock:
        llvm_unreachable("unhandled BasicBlock");
    case ValueKind::Function: {
        auto function = llvm::cast<Function>(this);
        auto paramTypes = map(function->params, [](auto& p) { return p.type; });
        return (new IRFunctionType{
                    IRTypeKind::IRFunctionType,
                    function->returnType,
                    std::move(paramTypes),
                    function->isVariadic,
                    function->hasContextParam,
                })
            ->getPointerTo();
    }
    case ValueKind::Parameter:
        return llvm::cast<Parameter>(this)->type;
    case ValueKind::GlobalVariable:
        return llvm::cast<GlobalVariable>(this)->type->getPointerTo();
    case ValueKind::ConstantString:
        return getIRType(PointerType::get(Type::getChar(), PointerKind::Pointer));
    case ValueKind::ConstantInt:
        return llvm::cast<ConstantInt>(this)->type;
    case ValueKind::ConstantFP:
        return llvm::cast<ConstantFP>(this)->type;
    case ValueKind::ConstantBool:
        return getIRType(Type::getBool());
    case ValueKind::ConstantNull:
        return llvm::cast<ConstantNull>(this)->type;
    case ValueKind::ConstantIntToPtr:
        return llvm::cast<ConstantIntToPtr>(this)->type;
    case ValueKind::Undefined:
        return llvm::cast<Undefined>(this)->type;
    }

    llvm_unreachable("unhandled instruction kind");
}

const Expr* Value::getExpr() const {
    switch (kind) {
    case ValueKind::CallInst:
        return llvm::cast<CallInst>(this)->expr;
    case ValueKind::BinaryInst:
        return llvm::cast<BinaryInst>(this)->expr;
    case ValueKind::UnaryInst:
        return llvm::cast<UnaryInst>(this)->expr;
    case ValueKind::ConstGEPInst:
        return llvm::cast<ConstGEPInst>(this)->expr;
    case ValueKind::ArrayOpInst:
        return llvm::cast<ArrayOpInst>(this)->expr;
    case ValueKind::CheckedArithInst:
        return llvm::cast<CheckedArithInst>(this)->expr;
    case ValueKind::SaturatingArithInst:
        return llvm::cast<SaturatingArithInst>(this)->expr;
    default:
        return nullptr;
    }
}

std::string Value::getName() const {
    switch (kind) {
    case ValueKind::AllocaInst:
        return llvm::cast<AllocaInst>(this)->name;
    case ValueKind::ReturnInst:
        llvm_unreachable("unhandled ReturnInst");
    case ValueKind::BranchInst:
        llvm_unreachable("unhandled BranchInst");
    case ValueKind::CondBranchInst:
        llvm_unreachable("unhandled CondBranchInst");
    case ValueKind::SwitchInst:
        llvm_unreachable("unhandled SwitchInst");
    case ValueKind::LoadInst:
        return llvm::cast<LoadInst>(this)->name;
    case ValueKind::StoreInst:
        llvm_unreachable("unhandled StoreInst");
    case ValueKind::InsertInst:
        return llvm::cast<InsertInst>(this)->name;
    case ValueKind::ExtractInst:
        return llvm::cast<ExtractInst>(this)->name;
    case ValueKind::CallInst:
        return llvm::cast<CallInst>(this)->name;
    case ValueKind::BinaryInst:
        return llvm::cast<BinaryInst>(this)->name;
    case ValueKind::UnaryInst:
        return llvm::cast<UnaryInst>(this)->name;
    case ValueKind::GEPInst:
        return llvm::cast<GEPInst>(this)->name;
    case ValueKind::ConstGEPInst:
        return llvm::cast<ConstGEPInst>(this)->name;
    case ValueKind::CastInst:
        return llvm::cast<CastInst>(this)->name;
    case ValueKind::UnreachableInst:
        llvm_unreachable("unhandled UnreachableInst");
    case ValueKind::ArrayOpInst:
        return llvm::cast<ArrayOpInst>(this)->name;
    case ValueKind::SizeofInst:
        return ("sizeof(" + llvm::cast<SizeofInst>(this)->type->getName() + ")").str();
    case ValueKind::CheckedArithInst:
        return llvm::cast<CheckedArithInst>(this)->name;
    case ValueKind::ArithOverflowInst:
        return llvm::cast<ArithOverflowInst>(this)->name;
    case ValueKind::SaturatingArithInst:
        return llvm::cast<SaturatingArithInst>(this)->name;
    case ValueKind::BasicBlock:
        return llvm::cast<BasicBlock>(this)->name;
    case ValueKind::Function:
        return llvm::cast<Function>(this)->mangledName;
    case ValueKind::Parameter:
        return llvm::cast<Parameter>(this)->name;
    case ValueKind::GlobalVariable:
        return llvm::cast<GlobalVariable>(this)->name;
    case ValueKind::ConstantString:
        return '"' + llvm::cast<ConstantString>(this)->value + '"';
    case ValueKind::ConstantInt: {
        llvm::SmallString<128> buffer;
        llvm::cast<ConstantInt>(this)->value.toString(buffer, 10);
        return std::string(buffer);
    }
    case ValueKind::ConstantFP: {
        llvm::SmallString<128> buffer;
        llvm::cast<ConstantFP>(this)->value.toString(buffer);
        return std::string(buffer);
    }
    case ValueKind::ConstantBool:
        return llvm::cast<ConstantBool>(this)->value ? "true" : "false";
    case ValueKind::ConstantNull:
        return "null";
    case ValueKind::ConstantIntToPtr: {
        llvm::SmallString<128> buffer;
        llvm::cast<ConstantIntToPtr>(this)->value.toString(buffer, 10);
        return "inttoptr(" + std::string(buffer) + ")";
    }
    case ValueKind::Undefined:
        return "undefined";
    }

    llvm_unreachable("unhandled instruction kind");
}

Value* Value::getBranchArgument() const {
    if (auto branch = llvm::dyn_cast<BranchInst>(this)) {
        return branch->argument;
    } else if (auto condBranch = llvm::dyn_cast<CondBranchInst>(this)) {
        return condBranch->argument;
    } else {
        llvm_unreachable("value has no branch argument");
    }
}

static bool isConstant(const Value* inst) {
    return inst->kind == ValueKind::ConstantInt || inst->kind == ValueKind::ConstantFP || inst->kind == ValueKind::ConstantString
        || inst->kind == ValueKind::Undefined || inst->kind == ValueKind::ConstantNull || inst->kind == ValueKind::ConstantBool
        || inst->kind == ValueKind::ConstantIntToPtr;
}

static std::unordered_map<const Value*, std::string> valuesNames;
static llvm::StringSet usedNames;

void cx::resetIRState() {
    // Reverse order: each `delete` routes through the tracking operator
    // delete above, which unregisters the most recent entry.
    for (auto it = allIRValues.rbegin(); it != allIRValues.rend(); ++it)
        deleteIRValue(*it);
    allIRValues.clear();
    for (auto it = allIRTypes.rbegin(); it != allIRTypes.rend(); ++it)
        deleteIRType(*it);
    allIRTypes.clear();
    irTypes.clear();
    irTypes.emplace(nullptr, nullptr);
    valuesNames.clear();
    usedNames.clear();
}

static std::string formatName(const Value* inst) {
    std::string str;
    llvm::raw_string_ostream s(str);

    if (isConstant(inst)) {
        s << inst->getType() << " " << inst->getName(); // Always print type for inline constants.
        return s.str();
    }

    std::string name;
    auto it = valuesNames.find(inst);
    if (it != valuesNames.end()) {
        name = it->second;
    } else {
        auto instName = inst->getName();
        name = instName;
        int i = 0;

        while (name.empty() || usedNames.count(name)) {
            name = instName + '_' + std::to_string(i++);
        }

        usedNames.insert(name);
        valuesNames.emplace(inst, name);
    }

    s << name;
    return std::move(s.str());
}

static std::string formatTypeAndName(const Value* inst) {
    std::string str;
    llvm::raw_string_ostream s(str);
    if (!isConstant(inst) && inst->kind != ValueKind::BasicBlock) {
        s << inst->getType() << " ";
    }
    s << formatName(inst);
    return std::move(s.str());
}

void Value::print(llvm::raw_ostream& stream) const {
    const auto indent = "    ";

    switch (this->kind) {
    case ValueKind::AllocaInst: {
        auto alloca = llvm::cast<AllocaInst>(this);
        stream << indent << formatTypeAndName(alloca) << " = alloca " << alloca->allocatedType;
        break;
    }
    case ValueKind::ReturnInst: {
        auto returnInst = llvm::cast<ReturnInst>(this);
        stream << indent << "return " << (returnInst->value ? formatName(returnInst->value) : "void");
        break;
    }
    case ValueKind::BranchInst: {
        auto branch = llvm::cast<BranchInst>(this);
        stream << indent << "br " << formatName(branch->destination);
        if (branch->argument) stream << "(" << formatName(branch->argument) << ")";
        break;
    }
    case ValueKind::CondBranchInst: {
        auto condBranch = llvm::cast<CondBranchInst>(this);
        stream << indent << "br " << formatName(condBranch->condition) << ", " << condBranch->trueBlock->name;
        if (condBranch->argument) stream << "(" << formatName(condBranch->argument) << ")";
        stream << ", " << condBranch->falseBlock->name;
        if (condBranch->argument) stream << "(" << formatName(condBranch->argument) << ")";
        break;
    }
    case ValueKind::SwitchInst: {
        auto switchInst = llvm::cast<SwitchInst>(this);
        stream << indent << "switch " << formatName(switchInst->condition) << " {\n";
        for (auto& p : switchInst->cases) {
            stream << indent << indent << formatName(p.first) << " -> " << p.second->name << "\n";
        }
        stream << indent << "}";
        break;
    }
    case ValueKind::LoadInst: {
        auto load = llvm::cast<LoadInst>(this);
        stream << indent << formatTypeAndName(load) << " = load " << formatName(load->value);
        break;
    }
    case ValueKind::StoreInst: {
        auto store = llvm::cast<StoreInst>(this);
        stream << indent << "store " << formatName(store->value) << " to " << formatName(store->pointer);
        break;
    }
    case ValueKind::InsertInst: {
        auto insert = llvm::cast<InsertInst>(this);
        stream << indent << formatTypeAndName(insert) << " = insertvalue " << formatName(insert->aggregate) << ", " << insert->index << ", "
               << formatName(insert->value);
        break;
    }
    case ValueKind::ExtractInst: {
        auto extract = llvm::cast<ExtractInst>(this);
        stream << indent << formatTypeAndName(extract) << " = extractvalue " << formatName(extract->aggregate) << ", " << extract->index;
        break;
    }
    case ValueKind::CallInst: {
        auto call = llvm::cast<CallInst>(this);
        stream << indent << formatTypeAndName(call) << " = call " << formatName(call->function) << "(";
        for (auto& arg : call->args) {
            stream << formatTypeAndName(arg);
            if (&arg != &call->args.back()) stream << ", ";
        }
        stream << ")";
        break;
    }
    case ValueKind::BinaryInst: {
        auto binaryOp = llvm::cast<BinaryInst>(this);
        stream << indent << formatTypeAndName(binaryOp) << " = " << formatName(binaryOp->left) << " " << binaryOp->op << " " << formatName(binaryOp->right);
        break;
    }
    case ValueKind::UnaryInst: {
        auto unaryOp = llvm::cast<UnaryInst>(this);
        stream << indent << formatTypeAndName(unaryOp) << " = " << unaryOp->op << formatName(unaryOp->operand);
        break;
    }
    case ValueKind::GEPInst: {
        auto gep = llvm::cast<GEPInst>(this);
        stream << indent << formatTypeAndName(gep) << " = getelementptr " << formatName(gep->pointer);
        for (auto* index : gep->indexes) {
            stream << ", " << formatName(index);
        }
        break;
    }
    case ValueKind::ConstGEPInst: {
        auto gep = llvm::cast<ConstGEPInst>(this);
        stream << indent << formatTypeAndName(gep) << " = getelementptr " << formatName(gep->pointer) << ", " << gep->index;
        break;
    }
    case ValueKind::CastInst: {
        auto cast = llvm::cast<CastInst>(this);
        stream << indent << formatTypeAndName(cast) << " = cast " << formatName(cast->value) << " to " << cast->type;
        break;
    }
    case ValueKind::UnreachableInst: {
        stream << indent << "unreachable";
        break;
    }
    case ValueKind::ArrayOpInst: {
        auto arrayOp = llvm::cast<ArrayOpInst>(this);
        stream << indent << formatTypeAndName(arrayOp) << " = arrayop " << arrayOp->op << " " << formatName(arrayOp->left) << ", "
               << formatName(arrayOp->right);
        break;
    }
    case ValueKind::SizeofInst:
        llvm_unreachable("unhandled SizeofInst");
    case ValueKind::CheckedArithInst: {
        auto checked = llvm::cast<CheckedArithInst>(this);
        stream << indent << formatTypeAndName(checked) << " = checkedarith " << checked->op << " " << formatName(checked->left) << ", "
               << formatName(checked->right);
        break;
    }
    case ValueKind::ArithOverflowInst: {
        auto overflow = llvm::cast<ArithOverflowInst>(this);
        stream << indent << formatTypeAndName(overflow) << " = overflow " << formatName(overflow->checked);
        break;
    }
    case ValueKind::SaturatingArithInst: {
        auto sat = llvm::cast<SaturatingArithInst>(this);
        stream << indent << formatTypeAndName(sat) << " = saturatingarith " << sat->op << " " << formatName(sat->left) << ", " << formatName(sat->right);
        break;
    }
    case ValueKind::BasicBlock:
        llvm_unreachable("handled via Function");
    case ValueKind::Function: {
        for (auto it = valuesNames.begin(); it != valuesNames.end();) {
            if (!it->first->isGlobal()) {
                usedNames.erase(it->second);
                it = valuesNames.erase(it);
            } else {
                it++;
            }
        }

        auto function = llvm::cast<Function>(this);
        stream << "\n";
        if (function->isExtern) stream << "extern ";
        stream << function->returnType << " " << function->mangledName << "(";
        for (auto& param : function->params) {
            if (function->isExtern) {
                stream << param.type;
            } else {
                stream << formatTypeAndName(&param);
            }
            if (&param != &function->params.back()) stream << ", ";
        }
        stream << ")";
        if (!function->isExtern) {
            stream << " {\n";
            for (auto& block : function->body) {
                if (&block != &function->body.front()) {
                    stream << "\n" << formatName(block);
                    if (block->parameter) stream << "(" << formatTypeAndName(block->parameter) << ")";
                    stream << ":\n";
                }
                for (auto* i : block->body) {
                    i->print(stream);
                }
            }
            stream << "}";
        }
        break;
    }
    case ValueKind::Parameter:
        stream << formatTypeAndName(this);
        break;
    case ValueKind::GlobalVariable: {
        auto globalVariable = llvm::cast<GlobalVariable>(this);
        if (globalVariable->value) {
            stream << "global " << formatName(globalVariable) << " = " << formatTypeAndName(globalVariable->value);
        } else {
            stream << "extern global " << globalVariable->type << " " << formatName(globalVariable);
        }
        break;
    }
    case ValueKind::ConstantString:
        llvm_unreachable("unhandled ConstantString");
    case ValueKind::ConstantInt:
        llvm_unreachable("unhandled ConstantInt");
    case ValueKind::ConstantFP:
        llvm_unreachable("unhandled ConstantFP");
    case ValueKind::ConstantBool:
        llvm_unreachable("unhandled ConstantBool");
    case ValueKind::ConstantNull:
        llvm_unreachable("unhandled ConstantNull");
    case ValueKind::ConstantIntToPtr:
        llvm_unreachable("unhandled ConstantIntToPtr");
    case ValueKind::Undefined:
        llvm_unreachable("unhandled Undefined");
    }

    stream << "\n";
}

bool Value::loads(Value* value, int gepIndex) {
    if (auto load = llvm::dyn_cast<LoadInst>(this)) {
        if (gepIndex == -1) {
            // Dereferencing through memory (e.g. `*p` where p is a spilled parameter or local)
            // lowers to a chain of loads; a null check on the end of the chain covers the base.
            return load->value == value || load->value->loads(value, gepIndex);
        } else {
            if (auto gep = llvm::dyn_cast<ConstGEPInst>(load->value)) {
                if (gep->pointer == value && gep->index == gepIndex) return true;
                if (gep->index == gepIndex) {
                    if (auto valueLoad = llvm::dyn_cast<LoadInst>(value)) {
                        if (auto gepPointerLoad = llvm::dyn_cast<LoadInst>(gep->pointer)) {
                            if (valueLoad->value == gepPointerLoad->value) {
                                return true;
                            }
                        }
                    }
                }
                return false;
            }
        }
    }
    return false;
}

void IRModule::print(llvm::raw_ostream& stream) const {
    for (auto* globalVariable : globalVariables) {
        globalVariable->print(stream);
    }

    for (auto* function : functions) {
        function->print(stream);
    }
}

bool IRType::isInteger() {
    if (!isBasicType()) return false;
    return llvm::StringSwitch<bool>(llvm::cast<IRBasicType>(this)->name)
        .Cases({"int8", "int16", "int32", "int64", "uint8", "uint16", "uint32", "uint64", "c_size_t", "c_schar", "c_uchar", "c_short", "c_ushort", "c_int",
                "c_uint", "c_long", "c_ulong", "c_longlong", "c_ulonglong"},
               true)
        .Default(false);
}

bool IRType::isSignedInteger() {
    if (!isBasicType()) return false;
    return llvm::StringSwitch<bool>(llvm::cast<IRBasicType>(this)->name)
        .Cases({"int8", "int16", "int32", "int64", "c_schar", "c_short", "c_int", "c_long", "c_longlong"}, true)
        .Default(false);
}

bool IRType::isUnsignedInteger() {
    if (!isBasicType()) return false;
    return llvm::StringSwitch<bool>(llvm::cast<IRBasicType>(this)->name)
        .Cases({"uint8", "uint16", "uint32", "uint64", "c_size_t", "c_uchar", "c_ushort", "c_uint", "c_ulong", "c_ulonglong"}, true)
        .Default(false);
}

bool IRType::isFloatingPoint() {
    if (!isBasicType()) return false;
    return llvm::StringSwitch<bool>(llvm::cast<IRBasicType>(this)->name).Cases({"float32", "float64", "float80", "c_float", "c_double"}, true).Default(false);
}

bool IRType::isChar() {
    if (!isBasicType()) return false;
    return llvm::cast<IRBasicType>(this)->name == "char";
}

bool IRType::isBool() {
    if (!isBasicType()) return false;
    return llvm::cast<IRBasicType>(this)->name == "bool";
}

bool IRType::isVoid() {
    if (!isBasicType()) return false;
    return llvm::cast<IRBasicType>(this)->name == "void";
}

bool IRType::isNever() {
    if (!isStruct()) return false;
    return llvm::cast<IRStructType>(this)->name == "never";
}

IRType* IRType::getPointee() {
    return llvm::cast<IRPointerType>(this)->pointee;
}

llvm::ArrayRef<IRField> IRType::getFields() {
    if (isUnion()) return llvm::cast<IRUnionType>(this)->fields;
    return llvm::cast<IRStructType>(this)->fields;
}

llvm::StringRef IRType::getName() {
    if (isBasicType()) return llvm::cast<IRBasicType>(this)->name;
    if (isUnion()) return llvm::cast<IRUnionType>(this)->name;
    return llvm::cast<IRStructType>(this)->name;
}

IRType* IRType::getReturnType() {
    return llvm::cast<IRFunctionType>(this)->returnType;
}

llvm::ArrayRef<IRType*> IRType::getParamTypes() {
    return llvm::cast<IRFunctionType>(this)->paramTypes;
}

IRType* IRType::getElementType() {
    return llvm::cast<IRArrayType>(this)->elementType;
}

int IRType::getArraySize() {
    auto* arrayType = llvm::cast<IRArrayType>(this);
    ASSERT(!arrayType->hasSymbolicSize());
    return arrayType->size;
}

IRType* IRType::getPointerTo() {
    return new IRPointerType{IRTypeKind::IRPointerType, this};
}

llvm::raw_ostream& cx::operator<<(llvm::raw_ostream& stream, IRType* type) {
    switch (type->kind) {
    case IRTypeKind::IRBasicType:
        return stream << type->getName();

    case IRTypeKind::IRPointerType:
        return stream << type->getPointee() << "*";

    case IRTypeKind::IRFunctionType:
        stream << type->getReturnType() << "(";
        for (auto& paramType : type->getParamTypes()) {
            stream << paramType;
            if (&paramType != &type->getParamTypes().back()) stream << ", ";
        }
        return stream << ")";

    case IRTypeKind::IRArrayType: {
        auto* arrayType = llvm::cast<IRArrayType>(type);
        if (arrayType->hasSymbolicSize()) return stream << arrayType->elementType << "[sizeof(" << arrayType->sizeofOperand << ")]";
        return stream << arrayType->elementType << "[" << arrayType->size << "]";
    }

    case IRTypeKind::IRStructType:
    case IRTypeKind::IRUnionType:
        if (type->getName() != "") return stream << type->getName();
        stream << (type->isUnion() ? "union" : "struct") << " { ";
        for (auto& field : type->getFields()) {
            stream << field.type;
            if (&field != &type->getFields().back()) stream << ", ";
        }
        return stream << " }";
    }

    llvm_unreachable("all cases handled");
}

bool IRType::equals(IRType* other) {
    switch (kind) {
    case IRTypeKind::IRBasicType:
        return other->isBasicType() && getName() == other->getName();

    case IRTypeKind::IRPointerType:
        return other->isPointerType() && getPointee()->equals(other->getPointee());

    case IRTypeKind::IRFunctionType:
        if (!other->isFunctionType()) return false;
        if (!getReturnType()->equals(other->getReturnType())) return false;
        if (getParamTypes().size() != other->getParamTypes().size()) return false;
        for (size_t i = 0; i < getParamTypes().size(); ++i) {
            if (!getParamTypes()[i]->equals(other->getParamTypes()[i])) return false;
        }
        return true;

    case IRTypeKind::IRArrayType: {
        if (!other->isArrayType()) return false;
        auto* a = llvm::cast<IRArrayType>(this);
        auto* b = llvm::cast<IRArrayType>(other);
        if (a->hasSymbolicSize() || b->hasSymbolicSize()) {
            if (!a->hasSymbolicSize() || !b->hasSymbolicSize() || a->sizeofOperand != b->sizeofOperand) return false;
        } else if (a->size != b->size) {
            return false;
        }
        return getElementType()->equals(other->getElementType());
    }

    case IRTypeKind::IRStructType:
        if (!other->isStruct()) return false;
        if (getName() != other->getName()) return false;
        if (getName().empty()) {
            if (getFields().size() != other->getFields().size()) return false;
            for (size_t i = 0; i < getFields().size(); ++i) {
                if (!getFields()[i].type->equals(other->getFields()[i].type)) return false;
            }
        }
        return true;

    case IRTypeKind::IRUnionType:
        if (!other->isUnion()) return false;
        if (getName() != other->getName()) return false;
        return true;
    }

    llvm_unreachable("all cases handled");
}

int cx::getIntegerBitWidth(IRType* type) {
    // c_size_t is pointer-sized like C's size_t; the host pointer width is
    // the target width (native host, or wasm32 under Emscripten).
    auto name = llvm::cast<IRBasicType>(type)->name;
    if (name == "c_size_t") return static_cast<int>(sizeof(void*) * 8);
    if (name == "c_long" || name == "c_ulong") return static_cast<int>(sizeof(long) * 8);
    return llvm::StringSwitch<int>(name)
        .Cases({"int8", "uint8", "c_schar", "c_uchar"}, 8)
        .Cases({"int16", "uint16", "c_short", "c_ushort"}, 16)
        .Cases({"int32", "uint32", "c_int", "c_uint"}, 32)
        .Cases({"int64", "uint64", "c_longlong", "c_ulonglong"}, 64)
        .Default(0);
}

Type cx::getUnsignedIntegerType(int width) {
    switch (width) {
    case 8:
        return Type::getUInt8();
    case 16:
        return Type::getUInt16();
    case 32:
        return Type::getUInt32();
    case 64:
        return Type::getUInt64();
    default:
        llvm_unreachable("invalid integer width");
    }
}

// Maps a builtin name to its (category, bits) calling-convention class,
// mirroring LLVMGenerator::getBuiltinType. Unknown names compare by name.
static std::pair<char, int> abiClass(llvm::StringRef name) {
    if (name == "void") return {'v', 0};
    if (name == "bool") return {'b', 1};
    if (name == "char" || name == "int8" || name == "uint8" || name == "c_schar" || name == "c_uchar") return {'i', 8};
    if (name == "int16" || name == "uint16" || name == "c_short" || name == "c_ushort") return {'i', 16};
    if (name == "int32" || name == "uint32" || name == "c_int" || name == "c_uint") return {'i', 32};
    if (name == "int64" || name == "uint64" || name == "c_longlong" || name == "c_ulonglong") return {'i', 64};
    if (name == "c_size_t") return {'i', static_cast<int>(sizeof(void*) * 8)};
    if (name == "c_long" || name == "c_ulong") return {'i', static_cast<int>(sizeof(long) * 8)};
    if (name == "float32" || name == "c_float") return {'f', 32};
    if (name == "float64" || name == "c_double") return {'f', 64};
    if (name == "float80") return {'f', 80};
    return {'x', 0};
}

bool IRType::abiEquals(IRType* other) {
    switch (kind) {
    case IRTypeKind::IRBasicType: {
        if (!other->isBasicType()) return false;
        auto a = abiClass(getName()), b = abiClass(other->getName());
        if (a.first == 'x' || b.first == 'x') return getName() == other->getName();
        return a == b;
    }
    case IRTypeKind::IRPointerType:
        return other->isPointerType();
    case IRTypeKind::IRFunctionType: {
        if (!other->isFunctionType()) return false;
        auto* a = llvm::cast<IRFunctionType>(this);
        auto* b = llvm::cast<IRFunctionType>(other);
        if (a->isVariadic != b->isVariadic) return false;
        if (!a->returnType->abiEquals(b->returnType)) return false;
        if (a->paramTypes.size() != b->paramTypes.size()) return false;
        for (size_t i = 0; i < a->paramTypes.size(); ++i) {
            if (!a->paramTypes[i]->abiEquals(b->paramTypes[i])) return false;
        }
        return true;
    }
    case IRTypeKind::IRArrayType: {
        if (!other->isArrayType() || !getElementType()->abiEquals(other->getElementType())) return false;
        auto* a = llvm::cast<IRArrayType>(this);
        auto* b = llvm::cast<IRArrayType>(other);
        if (a->hasSymbolicSize() || b->hasSymbolicSize()) return a->hasSymbolicSize() && b->hasSymbolicSize() && a->sizeofOperand == b->sizeofOperand;
        return a->size == b->size;
    }
    case IRTypeKind::IRStructType: {
        if (!other->isStruct()) return false;
        // Named structs lower to nominal LLVM types, so only identical objects
        // share one; anonymous ones lower structurally and compare by fields.
        if (this == other) return true;
        auto* a = llvm::cast<IRStructType>(this);
        auto* b = llvm::cast<IRStructType>(other);
        if (!a->name.empty() || !b->name.empty()) return false;
        if (a->packed != b->packed || a->fields.size() != b->fields.size()) return false;
        for (size_t i = 0; i < a->fields.size(); ++i) {
            if (!a->fields[i].type->abiEquals(b->fields[i].type)) return false;
        }
        return true;
    }
    case IRTypeKind::IRUnionType:
        // Unions lower to nominal LLVM types; only identical objects share one.
        return this == other;
    }

    llvm_unreachable("all cases handled");
}
