#include <iostream>
#include <limits>
#include <unordered_map>
#include <boost/utility/string_ref.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/optional.hpp>
#include "typecheck.h"
#include "../ast/type.h"
#include "../ast/expr.h"
#include "../ast/decl.h"
#include "../parser/parser.hpp"

static std::unordered_map<std::string, /*owned*/ Decl*> symbolTable;
static const Type* funcReturnType = nullptr;
static bool inInitializer = false;

template<typename... Args>
[[noreturn]] static void error(Args&&... args) {
    std::cout << "error: ";
    using expander = int[];
    (void)expander{0, (void(std::cout << std::forward<Args>(args)), 0)...};
    std::cout << '\n';
    exit(1);
}

const Type& typecheck(Expr& expr);
void typecheck(Stmt& stmt);

Type typecheck(VariableExpr& expr) {
    auto it = symbolTable.find(expr.identifier);
    if (it == symbolTable.end()) {
        error("unknown identifier '", expr.identifier, "'");
    }
    switch (it->second->getKind()) {
        case DeclKind::VarDecl: return it->second->getVarDecl().getType();
        case DeclKind::ParamDecl: return it->second->getParamDecl().type;
        case DeclKind::FuncDecl: return it->second->getFuncDecl().getFuncType();
        case DeclKind::InitDecl: assert(false && "cannot refer to initializers yet");
        case DeclKind::TypeDecl: return it->second->getTypeDecl().getType();
        case DeclKind::FieldDecl: return it->second->getFieldDecl().type;
        case DeclKind::ImportDecl: assert(false);
    }
}

Type typecheck(StrLiteralExpr& expr) {
    return Type(PtrType{std::unique_ptr<Type>(new Type(BasicType{"char"}))});
}

Type typecheck(IntLiteralExpr& expr) {
    if (expr.value >= std::numeric_limits<int32_t>::min()
    &&  expr.value <= std::numeric_limits<int32_t>::max())
        return Type(BasicType{"int"});
    else
    if (expr.value >= std::numeric_limits<int64_t>::min()
    &&  expr.value <= std::numeric_limits<int64_t>::max())
        return Type(BasicType{"int64"});
    else
        error("integer literal is too large");
}

Type typecheck(BoolLiteralExpr&) {
    return Type(BasicType{"bool"});
}

Type typecheck(PrefixExpr& expr) {
    if (expr.op.rawValue == STAR) { // Dereference operation
        auto operandType = typecheck(*expr.operand);
        if (operandType.getKind() != TypeKind::PtrType) {
            error("cannot dereference non-pointer type '", operandType, "'");
        }
        return *operandType.getPtrType().pointeeType;
    }
    if (expr.op.rawValue == AND) { // Address-of operation
        return Type(PtrType{std::unique_ptr<Type>(new Type(typecheck(*expr.operand)))});
    }
    return typecheck(*expr.operand);
}

static bool isValidConversion(const Expr&, const Type&, const Type&);

Type typecheck(BinaryExpr& expr) {
    Type leftType = typecheck(*expr.left);
    Type rightType = typecheck(*expr.right);
    if (!isValidConversion(*expr.left, leftType, rightType)
    &&  !isValidConversion(*expr.right, rightType, leftType)) {
        error("operands to binary expression must have same type");
    }
    return expr.op.isComparisonOperator() ? Type(BasicType{"bool"}) : leftType;
}

template<typename IntType>
bool checkRange(int64_t value, boost::string_ref param) {
    try {
        boost::numeric_cast<IntType>(value);
        return true;
    } catch (...) {
        error(value, " is out of range for parameter of type '", param, "'");
    }
}

static bool isValidConversion(const Expr& expr, const Type& source, const Type& target) {
    if (source.isImplicitlyConvertibleTo(target)) return true;

    // Autocast integer literals to parameter type if within range, error out if not within range.
    if (expr.getKind() == ExprKind::IntLiteralExpr && target.getKind() == TypeKind::BasicType) {
        int64_t value{expr.getIntLiteralExpr().value};
        boost::string_ref targetTypeName = target.getBasicType().name;
        if (targetTypeName == "int") return checkRange<int>(value, targetTypeName);
        if (targetTypeName == "uint") return checkRange<unsigned>(value, targetTypeName);
        if (targetTypeName == "int8") return checkRange<int8_t>(value, targetTypeName);
        if (targetTypeName == "int16") return checkRange<int16_t>(value, targetTypeName);
        if (targetTypeName == "int32") return checkRange<int32_t>(value, targetTypeName);
        if (targetTypeName == "int64") return checkRange<int64_t>(value, targetTypeName);
        if (targetTypeName == "uint8") return checkRange<uint8_t>(value, targetTypeName);
        if (targetTypeName == "uint16") return checkRange<uint16_t>(value, targetTypeName);
        if (targetTypeName == "uint32") return checkRange<uint32_t>(value, targetTypeName);
        if (targetTypeName == "uint64") return checkRange<uint64_t>(value, targetTypeName);
        if (targetTypeName == "size_t") return checkRange<size_t>(value, targetTypeName);
    }

    return false;
}

static void validateArgs(const std::vector<Arg>& args, const std::vector<ParamDecl>& params,
                         const std::string& funcName);

Type typecheckInitExpr(const TypeDecl& type, const std::vector<Arg>& args) {
    auto it = symbolTable.find("__init_" + type.name);
    if (it == symbolTable.end()) {
        error("no matching initializer for '", type.name, "'");
    }
    validateArgs(args, it->second->getInitDecl().params, "'" + type.name + "' initializer");
    return type.getType();
}

Type typecheck(CallExpr& expr) {
    auto it = symbolTable.find(expr.funcName);
    if (it == symbolTable.end()) {
        error("unknown function '", expr.funcName, "'");
    }
    expr.isInitializerCall = it->second->getKind() == DeclKind::TypeDecl;
    if (expr.isInitializerCall) {
        return typecheckInitExpr(it->second->getTypeDecl(), expr.args);
    } else if (expr.isMemberFuncCall()) {
        typecheck(*expr.receiver);
    }
    if (it->second->getKind() != DeclKind::FuncDecl) {
        error("'", expr.funcName, "' is not a function");
    }
    validateArgs(expr.args, it->second->getFuncDecl().params, "'" + expr.funcName + "'");
    return Type(TupleType{it->second->getFuncDecl().getFuncType().returnTypes});
}

static void validateArgs(const std::vector<Arg>& args, const std::vector<ParamDecl>& params,
                         const std::string& funcName) {
    if (args.size() < params.size()) {
        error("too few arguments to ", funcName, ", expected ", params.size());
    }
    if (args.size() > params.size()) {
        error("too many arguments to ", funcName, ", expected ", params.size());
    }
    for (int i = 0; i < params.size(); ++i) {
        if (args[i].label != params[i].label) {
            if (params[i].label.empty()) {
                error("excess argument label '", args[i].label, "' for argument #", i + 1,
                    ", expected no label");
            }
            error("invalid label '", args[i].label, "' for argument #", i + 1,
                ", expected '", params[i].label, "'");
        }
        auto argType = typecheck(*args[i].value);
        if (!isValidConversion(*args[i].value, argType, params[i].type)) {
            error("invalid argument #", i + 1, " type '", argType, "' to ",
                funcName, ", expected '", params[i].type, "'");
        }
    }
}

const Type& typecheck(CastExpr& expr) {
    const Type& sourceType = typecheck(*expr.expr);
    const Type& targetType = expr.type;

    switch (sourceType.getKind()) {
        case TypeKind::BasicType:
            if (sourceType.getBasicType().name == "bool") {
                if (targetType.getKind() == TypeKind::BasicType
                && targetType.getBasicType().name == "int") {
                    return targetType; // bool -> int
                }
            }
        case TypeKind::ArrayType:
        case TypeKind::TupleType:
        case TypeKind::FuncType:
            break;
        case TypeKind::PtrType:
            const Type& sourcePointee = *sourceType.getPtrType().pointeeType;
            if (targetType.getKind() == TypeKind::PtrType) {
                const Type& targetPointee = *targetType.getPtrType().pointeeType;
                if (sourcePointee.getKind() == TypeKind::BasicType
                && sourcePointee.getBasicType().name == "void"
                && (!targetPointee.isMutable() || sourcePointee.isMutable())) {
                    return targetType; // (mutable) void* -> T* / mutable void* -> mutable T*
                }
                if (targetPointee.getKind() == TypeKind::BasicType
                && targetPointee.getBasicType().name == "void"
                && (!targetPointee.isMutable() || sourcePointee.isMutable())) {
                    return targetType; // (mutable) T* -> void* / mutable T* -> mutable void*
                }
            }
            break;
    }
    error("illegal cast from '", sourceType, "' to '", targetType, "'");
}

Type typecheck(MemberExpr& expr) {
    auto it = symbolTable.find(expr.base);
    if (it == symbolTable.end()) {
        error("unknown identifier '", expr.base, "'");
    }

    decltype(symbolTable)::iterator typeIt;
    switch (it->second->getKind()) {
        case DeclKind::VarDecl:
            typeIt = symbolTable.find(it->second->getVarDecl().getType().getBasicType().name);
            break;
        case DeclKind::ParamDecl:
            typeIt = symbolTable.find(it->second->getParamDecl().type.getBasicType().name);
            break;
        default:
            error("'", expr.base, "' doesn't support member access");
    }
    assert(typeIt != symbolTable.end());
    for (const FieldDecl& field : typeIt->second->getTypeDecl().fields) {
        if (field.name == expr.member) {
            return field.type;
        }
    }
    error("no member named '", expr.member, "' in '", expr.base, "'");
}

Type typecheck(SubscriptExpr& expr) {
    const Type& lhsType = typecheck(*expr.array);
    const ArrayType* arrayType;

    if (lhsType.getKind() == TypeKind::ArrayType) {
        arrayType = &lhsType.getArrayType();
    } else if (lhsType.getKind() == TypeKind::PtrType
    && lhsType.getPtrType().pointeeType->getKind() == TypeKind::ArrayType) {
        arrayType = &lhsType.getPtrType().pointeeType->getArrayType();
    } else {
        error("cannot subscript '", lhsType, "', expected array or pointer-to-array");
    }

    const Type& indexType = typecheck(*expr.index);
    if (!isValidConversion(*expr.index, indexType, Type(BasicType{"int"}))) {
        error("illegal subscript index type '", indexType, "', expected 'int'");
    }

    if (expr.index->getKind() == ExprKind::IntLiteralExpr
    && expr.index->getIntLiteralExpr().value >= arrayType->size) {
        error("accessing array out-of-bounds with index ", expr.index->getIntLiteralExpr().value,
            ", array size is ", arrayType->size);
    }

    return *arrayType->elementType;
}

const Type& typecheck(Expr& expr) {
    boost::optional<Type> type;
    switch (expr.getKind()) {
        case ExprKind::VariableExpr:    type = typecheck(expr.getVariableExpr()); break;
        case ExprKind::StrLiteralExpr:  type = typecheck(expr.getStrLiteralExpr()); break;
        case ExprKind::IntLiteralExpr:  type = typecheck(expr.getIntLiteralExpr()); break;
        case ExprKind::BoolLiteralExpr: type = typecheck(expr.getBoolLiteralExpr()); break;
        case ExprKind::PrefixExpr:      type = typecheck(expr.getPrefixExpr()); break;
        case ExprKind::BinaryExpr:      type = typecheck(expr.getBinaryExpr()); break;
        case ExprKind::CallExpr:        type = typecheck(expr.getCallExpr()); break;
        case ExprKind::CastExpr:        type = typecheck(expr.getCastExpr()); break;
        case ExprKind::MemberExpr:      type = typecheck(expr.getMemberExpr()); break;
        case ExprKind::SubscriptExpr:   type = typecheck(expr.getSubscriptExpr()); break;
    }
    expr.setType(std::move(*type));
    return expr.getType();
}

static bool isValidConversion(const std::vector<Expr>& exprs, const Type& source, const Type& target) {
    if (source.getKind() != TypeKind::TupleType) {
        assert(target.getKind() != TypeKind::TupleType);
        assert(exprs.size() == 1);
        return isValidConversion(exprs[0], source, target);
    }
    assert(target.getKind() == TypeKind::TupleType);

    const TupleType& sourceTuple = source.getTupleType();
    const TupleType& targetTuple = source.getTupleType();

    for (int i = 0; i < exprs.size(); ++i) {
        if (!isValidConversion(exprs[i], sourceTuple.subtypes.at(i), targetTuple.subtypes.at(i))) {
            return false;
        }
    }
    return true;
}

void typecheck(ReturnStmt& stmt) {
    std::vector<Type> returnValueTypes;
    for (Expr& expr : stmt.values) {
        returnValueTypes.push_back(typecheck(expr));
    }
    Type returnType = returnValueTypes.size() > 1
        ? Type(TupleType{std::move(returnValueTypes)}) : std::move(returnValueTypes[0]);
    if (!isValidConversion(stmt.values, returnType, *funcReturnType)) {
        error("mismatching return type '", returnType, "', expected '", *funcReturnType, "'");
    }
}

void typecheck(VarDecl& decl);

void typecheck(VariableStmt& stmt) {
    typecheck(*stmt.decl);
}

void typecheck(IncrementStmt& stmt) {
    auto type = typecheck(stmt.operand);
    if (!type.isMutable()) {
        error("cannot increment immutable value");
    }
    // TODO: check that operand supports increment operation.
}

void typecheck(DecrementStmt& stmt) {
    auto type = typecheck(stmt.operand);
    if (!type.isMutable()) {
        error("cannot decrement immutable value");
    }
    // TODO: check that operand supports decrement operation.
}

void typecheck(IfStmt& ifStmt) {
    const Type& conditionType = typecheck(ifStmt.condition);
    if (conditionType != Type(BasicType{"bool"})) {
        error("'if' condition must have type 'bool'");
    }
    for (Stmt& stmt : ifStmt.thenBody) typecheck(stmt);
    for (Stmt& stmt : ifStmt.elseBody) typecheck(stmt);
}

void typecheck(WhileStmt& whileStmt) {
    const Type& conditionType = typecheck(whileStmt.condition);
    if (conditionType != Type(BasicType{"bool"})) {
        error("'while' condition must have type 'bool'");
    }
    for (Stmt& stmt : whileStmt.body) typecheck(stmt);
}

void typecheck(AssignStmt& stmt) {
    const Type& lhsType = typecheck(stmt.lhs);
    if (lhsType.getKind() == TypeKind::FuncType) {
        error("cannot assign to function");
    }
    const Type& rhsType = typecheck(stmt.rhs);
    if (!isValidConversion(stmt.rhs, rhsType, lhsType)) {
        error("cannot assign '", rhsType, "' to variable of type '", lhsType, "'");
    }
    if (!lhsType.isMutable() && !inInitializer) {
        if (stmt.lhs.getKind() == ExprKind::VariableExpr) {
            error("cannot assign to immutable variable '", stmt.lhs.getVariableExpr().identifier, "'");
        } else {
            error("cannot assign to immutable expression");
        }
    }
}

void typecheck(Stmt& stmt) {
    switch (stmt.getKind()) {
        case StmtKind::ReturnStmt:    typecheck(stmt.getReturnStmt()); break;
        case StmtKind::VariableStmt:  typecheck(stmt.getVariableStmt()); break;
        case StmtKind::IncrementStmt: typecheck(stmt.getIncrementStmt()); break;
        case StmtKind::DecrementStmt: typecheck(stmt.getDecrementStmt()); break;
        case StmtKind::CallStmt:      typecheck(stmt.getCallStmt().expr); break;
        case StmtKind::IfStmt:        typecheck(stmt.getIfStmt()); break;
        case StmtKind::WhileStmt:     typecheck(stmt.getWhileStmt()); break;
        case StmtKind::AssignStmt:    typecheck(stmt.getAssignStmt()); break;
    }
}

void typecheck(ParamDecl& decl) {
    if (symbolTable.count(decl.name) > 0) {
        error("redefinition of '", decl.name, "'");
    }
    symbolTable.insert({decl.name, new Decl(ParamDecl(decl))});
}

void addToSymbolTable(const FuncDecl& decl) {
    if (symbolTable.count(decl.name) > 0) {
        error("redefinition of '", decl.name, "'");
    }
    symbolTable.insert({decl.name, new Decl(FuncDecl(decl))});
}

void addToSymbolTable(const InitDecl& decl) {
    if (symbolTable.count("__init_" + decl.getTypeName()) > 0) {
        error("redefinition of '", decl.getTypeName(), "' initializer");
    }

    InitDecl initDecl(decl);
    auto it = symbolTable.find(decl.getTypeName());
    if (it == symbolTable.end()) {
        error("unknown identifier '", decl.getTypeName(), "'");
    }
    if (it->second->getKind() != DeclKind::TypeDecl) {
        error("'", decl.getTypeName(), "' is not a class or struct");
    }
    initDecl.type = &it->second->getTypeDecl();

    symbolTable.insert({"__init_" + decl.getTypeName(), new Decl(std::move(initDecl))});
}

void addToSymbolTable(const TypeDecl& decl) {
    if (symbolTable.count(decl.name) > 0) {
        error("redefinition of '", decl.name, "'");
    }
    symbolTable.insert({decl.name, new Decl(TypeDecl(decl))});
}

void typecheckMemberFunc(FuncDecl& decl);

void typecheck(FuncDecl& decl) {
    if (!decl.receiverType.empty()) return typecheckMemberFunc(decl);
    if (decl.isExtern()) return;
    auto symbolTableBackup = symbolTable;
    for (ParamDecl& param : decl.params) typecheck(param);
    funcReturnType = &decl.returnType;
    for (Stmt& stmt : *decl.body) typecheck(stmt);
    funcReturnType = nullptr;
    symbolTable = std::move(symbolTableBackup);
}

void typecheckMemberFunc(FuncDecl& decl) {
    auto symbolTableBackup = symbolTable;
    auto it = symbolTable.find(decl.receiverType);
    if (it == symbolTable.end()) {
        error("unknown identifier '", decl.receiverType, "'");
    }
    if (it->second->getKind() != DeclKind::TypeDecl) {
        error("'", decl.receiverType, "' is not a class or struct");
    }
    symbolTable.insert({"this", new Decl(VarDecl{it->second->getTypeDecl().getType(), "this"})});
    for (ParamDecl& param : decl.params) typecheck(param);
    funcReturnType = &decl.returnType;
    for (Stmt& stmt : *decl.body) typecheck(stmt);
    funcReturnType = nullptr;
    symbolTable = std::move(symbolTableBackup);
}

void typecheck(InitDecl& decl) {
    auto symbolTableBackup = symbolTable;
    auto it = symbolTable.find(decl.getTypeName());
    if (it == symbolTable.end()) {
        error("unknown identifier '", decl.getTypeName(), "'");
    }
    if (it->second->getKind() != DeclKind::TypeDecl) {
        error("'", decl.getTypeName(), "' is not a class or struct");
    }
    decl.type = &it->second->getTypeDecl();
    symbolTable.insert({"this", new Decl(VarDecl{it->second->getTypeDecl().getType(), "this"})});
    for (ParamDecl& param : decl.params) typecheck(param);
    inInitializer = true;
    for (Stmt& stmt : *decl.body) typecheck(stmt);
    inInitializer = false;
    symbolTable = std::move(symbolTableBackup);
}

void typecheck(TypeDecl& decl) {
    // TODO
}

void typecheck(VarDecl& decl) {
    if (symbolTable.count(decl.name) > 0) {
        error("redefinition of '", decl.name, "'");
    }
    const Type* initType = nullptr;
    if (decl.initializer) {
        initType = &typecheck(*decl.initializer);
        if (initType->getKind() == TypeKind::FuncType) {
            error("function pointers not implemented yet");
        }
    }
    if (auto declaredType = decl.getDeclaredType()) {
        if (initType && !isValidConversion(*decl.initializer, *initType, *declaredType)) {
            error("cannot initialize variable of type '", *declaredType,
                "' with '", *initType, "'");
        }
        symbolTable.insert({decl.name, new Decl(VarDecl(decl))});
    } else {
        auto initTypeCopy = *initType;
        initTypeCopy.setMutable(decl.isMutable());
        decl.type = std::move(initTypeCopy);
        symbolTable.insert({decl.name, new Decl(VarDecl(decl))});
    }
}

void typecheck(FieldDecl& decl) {
}

void typecheck(ImportDecl& decl) {
}

void typecheck(Decl& decl) {
    switch (decl.getKind()) {
        case DeclKind::ParamDecl: typecheck(decl.getParamDecl()); break;
        case DeclKind::FuncDecl:  typecheck(decl.getFuncDecl()); break;
        case DeclKind::InitDecl:  typecheck(decl.getInitDecl()); break;
        case DeclKind::TypeDecl:  typecheck(decl.getTypeDecl()); break;
        case DeclKind::VarDecl:   typecheck(decl.getVarDecl()); break;
        case DeclKind::FieldDecl: typecheck(decl.getFieldDecl()); break;
        case DeclKind::ImportDecl:typecheck(decl.getImportDecl()); break;
    }
}

void typecheck(std::vector<Decl>& decls) {
    for (Decl& decl : decls) typecheck(decl);
}
