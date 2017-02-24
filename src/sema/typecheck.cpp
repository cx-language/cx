#include <iostream>
#include <limits>
#include <unordered_map>
#include <boost/utility/string_ref.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/optional.hpp>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/STLExtras.h>
#include "typecheck.h"
#include "c-import.h"
#include "../ast/type.h"
#include "../ast/expr.h"
#include "../ast/decl.h"
#include "../parser/parser.hpp"
#include "../driver/utility.h"

using namespace delta;

namespace {

std::unordered_map<std::string, /*owned*/ Decl*> symbolTable;
const Type* funcReturnType = nullptr;
bool inInitializer = false;
bool importingC = false;

const Type& typecheck(Expr& expr);
void typecheck(Stmt& stmt);

Type typecheck(VariableExpr& expr) {
    Decl& decl = findInSymbolTable(expr.identifier, expr.srcLoc);

    switch (decl.getKind()) {
        case DeclKind::VarDecl: return decl.getVarDecl().getType();
        case DeclKind::ParamDecl: return decl.getParamDecl().type;
        case DeclKind::FuncDecl: return decl.getFuncDecl().getFuncType();
        case DeclKind::InitDecl: assert(false && "cannot refer to initializers yet");
        case DeclKind::DeinitDecl: assert(false && "cannot refer to deinitializers yet");
        case DeclKind::TypeDecl: return decl.getTypeDecl().getType();
        case DeclKind::FieldDecl: return decl.getFieldDecl().type;
        case DeclKind::ImportDecl: assert(false);
    }
}

Type typecheck(StrLiteralExpr& expr) {
    ArrayType arrayType{llvm::make_unique<Type>(BasicType{"char"}), int64_t(expr.value.size() + 1)};
    return PtrType{llvm::make_unique<Type>(std::move(arrayType)), true};
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
        error(expr.srcLoc, "integer literal is too large");
}

Type typecheck(BoolLiteralExpr&) {
    return Type(BasicType{"bool"});
}

Type typecheck(NullLiteralExpr&) {
    return Type(BasicType{"null"});
}

Type typecheck(PrefixExpr& expr) {
    const Type& operandType = typecheck(*expr.operand);

    if (expr.op.rawValue == NOT) {
        if (!operandType.isBasicType() || operandType.getBasicType().name != "bool") {
            error(expr.operand->getSrcLoc(), "invalid operand type '", operandType, "' to logical not");
        }
        return operandType;
    }
    if (expr.op.rawValue == STAR) { // Dereference operation
        if (!operandType.isPtrType()) {
            error(expr.operand->getSrcLoc(), "cannot dereference non-pointer type '", operandType, "'");
        }
        return *operandType.getPtrType().pointeeType;
    }
    if (expr.op.rawValue == AND) { // Address-of operation
        return Type(PtrType{llvm::make_unique<Type>(operandType), true});
    }
    return operandType;
}

bool isValidConversion(Expr&, const Type&, const Type&);

Type typecheck(BinaryExpr& expr) {
    const Type& leftType = typecheck(*expr.left);
    const Type& rightType = typecheck(*expr.right);

    if (expr.op.rawValue == AND_AND || expr.op.rawValue == OR_OR) {
        if (leftType.isBasicType() && leftType.getBasicType().name == "bool"
        &&  rightType.isBasicType() && rightType.getBasicType().name == "bool") {
            return Type(BasicType{"bool"});
        }
        error(expr.srcLoc, "invalid operands to binary expression ('", leftType, "' and '", rightType, "')");
    }

    if (!isValidConversion(*expr.left, leftType, rightType)
    &&  !isValidConversion(*expr.right, rightType, leftType)) {
        error(expr.srcLoc, "invalid operands to binary expression ('", leftType, "' and '", rightType, "')");
    }
    return expr.op.isComparisonOperator() ? Type(BasicType{"bool"}) : leftType;
}

TypeDecl* getTypeDecl(const BasicType& type);

template<typename IntType>
bool checkRange(Expr& expr, int64_t value, boost::string_ref param) {
    try {
        boost::numeric_cast<IntType>(value);
    } catch (...) {
        error(expr.getSrcLoc(), value, " is out of range for parameter of type '", param, "'");
    }
    expr.setType(BasicType{param.to_string()});
    return true;
}

bool isValidConversion(Expr& expr, const Type& source, const Type& target) {
    if (expr.isLvalue() && source.isBasicType()) {
        TypeDecl* typeDecl = getTypeDecl(source.getBasicType());
        if (typeDecl && !typeDecl->passByValue() && !target.isPtrType()) {
            error(expr.getSrcLoc(), "implicit copying of class instances is disallowed");
        }
    }
    if (source.isImplicitlyConvertibleTo(target)) return true;

    // Autocast integer literals to parameter type if within range, error out if not within range.
    if (expr.isIntLiteralExpr() && target.isBasicType()) {
        int64_t value{expr.getIntLiteralExpr().value};
        boost::string_ref targetTypeName = target.getBasicType().name;
        if (targetTypeName == "int") return checkRange<int>(expr, value, targetTypeName);
        if (targetTypeName == "uint") return checkRange<unsigned>(expr, value, targetTypeName);
        if (targetTypeName == "int8") return checkRange<int8_t>(expr, value, targetTypeName);
        if (targetTypeName == "int16") return checkRange<int16_t>(expr, value, targetTypeName);
        if (targetTypeName == "int32") return checkRange<int32_t>(expr, value, targetTypeName);
        if (targetTypeName == "int64") return checkRange<int64_t>(expr, value, targetTypeName);
        if (targetTypeName == "uint8") return checkRange<uint8_t>(expr, value, targetTypeName);
        if (targetTypeName == "uint16") return checkRange<uint16_t>(expr, value, targetTypeName);
        if (targetTypeName == "uint32") return checkRange<uint32_t>(expr, value, targetTypeName);
        if (targetTypeName == "uint64") return checkRange<uint64_t>(expr, value, targetTypeName);
    } else if (expr.isNullLiteralExpr() && target.isPtrType() && !target.getPtrType().ref) {
        expr.setType(target);
        return true;
    } else if (expr.isStrLiteralExpr() && target.isPtrType()
               && target.getPtrType().pointeeType->isBasicType()
               && target.getPtrType().pointeeType->getBasicType().name == "char"
               && !target.getPtrType().pointeeType->isMutable()) {
        // Special case: allow passing string literals as C-strings (const char*).
        expr.setType(target);
        return true;
    } else if (expr.isLvalue() && source.isBasicType() && target.isPtrType()) {
        auto typeDecl = getTypeDecl(source.getBasicType());
        if (!typeDecl || typeDecl->passByValue()) {
            error(expr.getSrcLoc(), "cannot implicitly pass value types by reference, add explicit '&'");
        }
        if (source.isImplicitlyConvertibleTo(*target.getPtrType().pointeeType)) return true;
    }

    return false;
}

void validateArgs(const std::vector<Arg>& args, const std::vector<ParamDecl>& params,
                  const std::string& funcName, SrcLoc srcLoc);

Type typecheckInitExpr(const TypeDecl& type, const std::vector<Arg>& args, SrcLoc srcLoc) {
    auto it = symbolTable.find("__init_" + type.name);
    if (it == symbolTable.end()) {
        error(srcLoc, "no matching initializer for '", type.name, "'");
    }
    validateArgs(args, it->second->getInitDecl().params, "'" + type.name + "' initializer", srcLoc);
    return type.getType();
}

Type typecheck(CallExpr& expr) {
    Decl& decl = findInSymbolTable(expr.funcName, expr.srcLoc);
    expr.isInitializerCall = decl.isTypeDecl();
    if (expr.isInitializerCall) {
        return typecheckInitExpr(decl.getTypeDecl(), expr.args, expr.srcLoc);
    } else if (expr.isMemberFuncCall()) {
        typecheck(*expr.receiver);
    }
    if (!decl.isFuncDecl()) {
        error(expr.srcLoc, "'", expr.funcName, "' is not a function");
    }
    validateArgs(expr.args, decl.getFuncDecl().params, "'" + expr.funcName + "'", expr.srcLoc);
    return Type(TupleType{decl.getFuncDecl().getFuncType().returnTypes});
}

void validateArgs(const std::vector<Arg>& args, const std::vector<ParamDecl>& params,
                  const std::string& funcName, SrcLoc srcLoc) {
    if (args.size() < params.size()) {
        error(srcLoc, "too few arguments to ", funcName, ", expected ", params.size());
    }
    if (args.size() > params.size()) {
        error(srcLoc, "too many arguments to ", funcName, ", expected ", params.size());
    }
    for (int i = 0; i < params.size(); ++i) {
        if (args[i].label != params[i].label) {
            if (params[i].label.empty()) {
                error(args[i].srcLoc, "excess argument label '", args[i].label,
                      "' for argument #", i + 1, ", expected no label");
            }
            error(args[i].srcLoc, "invalid label '", args[i].label, "' for argument #",
                  i + 1, ", expected '", params[i].label, "'");
        }
        auto argType = typecheck(*args[i].value);
        if (!isValidConversion(*args[i].value, argType, params[i].type)) {
            error(args[i].srcLoc, "invalid argument #", i + 1, " type '", argType,
                  "' to ", funcName, ", expected '", params[i].type, "'");
        }
    }
}

const Type& typecheck(CastExpr& expr) {
    const Type& sourceType = typecheck(*expr.expr);
    const Type& targetType = expr.type;

    switch (sourceType.getKind()) {
        case TypeKind::BasicType:
            if (sourceType.getBasicType().name == "bool") {
                if (targetType.isBasicType() && targetType.getBasicType().name == "int") {
                    return targetType; // bool -> int
                }
            }
        case TypeKind::ArrayType:
        case TypeKind::TupleType:
        case TypeKind::FuncType:
            break;
        case TypeKind::PtrType:
            const Type& sourcePointee = *sourceType.getPtrType().pointeeType;
            if (targetType.isPtrType()) {
                const Type& targetPointee = *targetType.getPtrType().pointeeType;
                if (sourcePointee.isBasicType() && sourcePointee.getBasicType().name == "void"
                && (!targetPointee.isMutable() || sourcePointee.isMutable())) {
                    return targetType; // (mutable) void* -> T* / mutable void* -> mutable T*
                }
                if (targetPointee.isBasicType() && targetPointee.getBasicType().name == "void"
                && (!targetPointee.isMutable() || sourcePointee.isMutable())) {
                    return targetType; // (mutable) T* -> void* / mutable T* -> mutable void*
                }
            }
            break;
    }
    error(expr.srcLoc, "illegal cast from '", sourceType, "' to '", targetType, "'");
}

Type typecheck(MemberExpr& expr) {
    Decl& baseDecl = findInSymbolTable(expr.base, expr.baseSrcLoc);

    const Type* baseType;
    switch (baseDecl.getKind()) {
        case DeclKind::VarDecl:   baseType = &baseDecl.getVarDecl().getType(); break;
        case DeclKind::ParamDecl: baseType = &baseDecl.getParamDecl().type; break;
        default: error(expr.baseSrcLoc, "'", expr.base, "' doesn't support member access");
    }

    if (baseType->isPtrType()) baseType = baseType->getPtrType().pointeeType.get();
    Decl& typeDecl = findInSymbolTable(baseType->getBasicType().name, SrcLoc::invalid());

    for (const FieldDecl& field : typeDecl.getTypeDecl().fields) {
        if (field.name == expr.member) {
            return field.type;
        }
    }
    error(expr.memberSrcLoc, "no member named '", expr.member, "' in '", expr.base, "'");
}

Type typecheck(SubscriptExpr& expr) {
    const Type& lhsType = typecheck(*expr.array);
    const ArrayType* arrayType;

    if (lhsType.isArrayType()) {
        arrayType = &lhsType.getArrayType();
    } else if (lhsType.isPtrType() && lhsType.getPtrType().pointeeType->isArrayType()) {
        arrayType = &lhsType.getPtrType().pointeeType->getArrayType();
    } else {
        error(expr.array->getSrcLoc(), "cannot subscript '", lhsType, "', expected array or pointer-to-array");
    }

    const Type& indexType = typecheck(*expr.index);
    if (!isValidConversion(*expr.index, indexType, Type(BasicType{"int"}))) {
        error(expr.index->getSrcLoc(), "illegal subscript index type '", indexType, "', expected 'int'");
    }

    if (expr.index->isIntLiteralExpr() && expr.index->getIntLiteralExpr().value >= arrayType->size) {
        error(expr.index->getSrcLoc(), "accessing array out-of-bounds with index ",
              expr.index->getIntLiteralExpr().value, ", array size is ", arrayType->size);
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
        case ExprKind::NullLiteralExpr: type = typecheck(expr.getNullLiteralExpr()); break;
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

bool isValidConversion(std::vector<Expr>& exprs, const Type& source, const Type& target) {
    if (!source.isTupleType()) {
        assert(!target.isTupleType());
        assert(exprs.size() == 1);
        return isValidConversion(exprs[0], source, target);
    }
    assert(target.isTupleType());

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
    if (stmt.values.empty()) {
        if (!funcReturnType->isBasicType() || funcReturnType->getBasicType().name != "void") {
            error(stmt.srcLoc, "expected return statement to return a value of type '", *funcReturnType, "'");
        }
        return;
    }
    std::vector<Type> returnValueTypes;
    for (Expr& expr : stmt.values) {
        returnValueTypes.push_back(typecheck(expr));
    }
    Type returnType = returnValueTypes.size() > 1
        ? Type(TupleType{std::move(returnValueTypes)}) : std::move(returnValueTypes[0]);
    if (!isValidConversion(stmt.values, returnType, *funcReturnType)) {
        error(stmt.srcLoc, "mismatching return type '", returnType, "', expected '", *funcReturnType, "'");
    }
}

void typecheck(VarDecl& decl);

void typecheck(VariableStmt& stmt) {
    typecheck(*stmt.decl);
}

void typecheck(IncrementStmt& stmt) {
    auto type = typecheck(stmt.operand);
    if (!type.isMutable()) {
        error(stmt.srcLoc, "cannot increment immutable value");
    }
    // TODO: check that operand supports increment operation.
}

void typecheck(DecrementStmt& stmt) {
    auto type = typecheck(stmt.operand);
    if (!type.isMutable()) {
        error(stmt.srcLoc, "cannot decrement immutable value");
    }
    // TODO: check that operand supports decrement operation.
}

void typecheck(IfStmt& ifStmt) {
    const Type& conditionType = typecheck(ifStmt.condition);
    if (conditionType != Type(BasicType{"bool"})) {
        error(ifStmt.condition.getSrcLoc(), "'if' condition must have type 'bool'");
    }
    for (Stmt& stmt : ifStmt.thenBody) typecheck(stmt);
    for (Stmt& stmt : ifStmt.elseBody) typecheck(stmt);
}

void typecheck(SwitchStmt& stmt) {
    const Type& conditionType = typecheck(stmt.condition);
    for (SwitchCase& switchCase : stmt.cases) {
        const Type& caseType = typecheck(switchCase.value);
        if (caseType != conditionType) {
            error(switchCase.value.getSrcLoc(), "case value type '", caseType,
                  "' doesn't match switch condition type '", conditionType, "'");
        }
        for (Stmt& caseStmt : switchCase.stmts) typecheck(caseStmt);
    }
    for (Stmt& defaultStmt : stmt.defaultStmts) typecheck(defaultStmt);
}

void typecheck(WhileStmt& whileStmt) {
    const Type& conditionType = typecheck(whileStmt.condition);
    if (conditionType != Type(BasicType{"bool"})) {
        error(whileStmt.condition.getSrcLoc(), "'while' condition must have type 'bool'");
    }
    for (Stmt& stmt : whileStmt.body) typecheck(stmt);
}

void typecheck(AssignStmt& stmt) {
    const Type& lhsType = typecheck(stmt.lhs);
    if (lhsType.isFuncType()) error(stmt.srcLoc, "cannot assign to function");
    const Type& rhsType = typecheck(stmt.rhs);
    if (!isValidConversion(stmt.rhs, rhsType, lhsType)) {
        error(stmt.rhs.getSrcLoc(), "cannot assign '", rhsType, "' to variable of type '", lhsType, "'");
    }
    if (!lhsType.isMutable() && !inInitializer) {
        if (stmt.lhs.isVariableExpr()) {
            error(stmt.srcLoc, "cannot assign to immutable variable '",
                  stmt.lhs.getVariableExpr().identifier, "'");
        } else {
            error(stmt.srcLoc, "cannot assign to immutable expression");
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
        case StmtKind::DeferStmt:     typecheck(stmt.getDeferStmt().expr); break;
        case StmtKind::IfStmt:        typecheck(stmt.getIfStmt()); break;
        case StmtKind::SwitchStmt:    typecheck(stmt.getSwitchStmt()); break;
        case StmtKind::WhileStmt:     typecheck(stmt.getWhileStmt()); break;
        case StmtKind::AssignStmt:    typecheck(stmt.getAssignStmt()); break;
    }
}

void typecheck(ParamDecl& decl) {
    if (symbolTable.count(decl.name) > 0) {
        error(decl.srcLoc, "redefinition of '", decl.name, "'");
    }
    symbolTable.insert({decl.name, new Decl(ParamDecl(decl))});
}

} // anonymous namespace

void delta::addToSymbolTable(const FuncDecl& decl) {
    if (!importingC && symbolTable.count(decl.name) > 0) {
        error(decl.srcLoc, "redefinition of '", decl.name, "'");
    }
    symbolTable.insert({decl.name, new Decl(FuncDecl(decl))});
}

void delta::addToSymbolTable(const InitDecl& decl) {
    if (symbolTable.count("__init_" + decl.getTypeName()) > 0) {
        error(decl.srcLoc, "redefinition of '", decl.getTypeName(), "' initializer");
    }

    InitDecl initDecl(decl);
    Decl& typeDecl = findInSymbolTable(decl.getTypeName(), decl.srcLoc);
    if (!typeDecl.isTypeDecl()) error(decl.srcLoc, "'", decl.getTypeName(), "' is not a class or struct");
    initDecl.type = &typeDecl.getTypeDecl();

    symbolTable.insert({"__init_" + decl.getTypeName(), new Decl(std::move(initDecl))});
}

void delta::addToSymbolTable(const DeinitDecl& decl) {
    if (symbolTable.count("__deinit_" + decl.getTypeName()) > 0) {
        error(decl.srcLoc, "redefinition of '", decl.getTypeName(), "' deinitializer");
    }

    DeinitDecl deinitDecl(decl);
    Decl& typeDecl = findInSymbolTable(decl.getTypeName(), decl.srcLoc);
    if (!typeDecl.isTypeDecl()) error(decl.srcLoc, "'", decl.getTypeName(), "' is not a class or struct");
    deinitDecl.type = &typeDecl.getTypeDecl();

    symbolTable.insert({"__deinit_" + decl.getTypeName(), new Decl(std::move(deinitDecl))});
}

void delta::addToSymbolTable(const TypeDecl& decl) {
    if (!importingC && symbolTable.count(decl.name) > 0) {
        error(decl.srcLoc, "redefinition of '", decl.name, "'");
    }
    symbolTable.insert({decl.name, new Decl(TypeDecl(decl))});
}

void delta::addToSymbolTable(const VarDecl& decl) {
    if (!importingC && symbolTable.count(decl.name) > 0) {
        error(decl.srcLoc, "redefinition of '", decl.name, "'");
    }
    symbolTable.insert({decl.name, new Decl(VarDecl(decl))});
}

Decl& delta::findInSymbolTable(llvm::StringRef name, SrcLoc srcLoc) {
    auto it = symbolTable.find(name);
    if (it == symbolTable.end()) error(srcLoc, "unknown identifier '", name, "'");
    return *it->second;
}

namespace {

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
    Decl& receiverType = findInSymbolTable(decl.receiverType, decl.srcLoc);
    if (!receiverType.isTypeDecl()) error(decl.srcLoc, "'", decl.receiverType, "' is not a class or struct");
    symbolTable.emplace("this",
        new Decl(VarDecl{receiverType.getTypeDecl().getTypeForPassing(), "this", nullptr, SrcLoc::invalid()}));
    for (ParamDecl& param : decl.params) typecheck(param);
    funcReturnType = &decl.returnType;
    for (Stmt& stmt : *decl.body) typecheck(stmt);
    funcReturnType = nullptr;
    symbolTable = std::move(symbolTableBackup);
}

void typecheck(InitDecl& decl) {
    auto symbolTableBackup = symbolTable;
    Decl& typeDecl = findInSymbolTable(decl.getTypeName(), decl.srcLoc);
    if (!typeDecl.isTypeDecl()) error(decl.srcLoc, "'", decl.getTypeName(), "' is not a class or struct");
    decl.type = &typeDecl.getTypeDecl();
    symbolTable.insert({"this",
        new Decl(VarDecl{typeDecl.getTypeDecl().getType(), "this", nullptr, SrcLoc::invalid()})});
    for (ParamDecl& param : decl.params) typecheck(param);
    inInitializer = true;
    for (Stmt& stmt : *decl.body) typecheck(stmt);
    inInitializer = false;
    symbolTable = std::move(symbolTableBackup);
}

void typecheck(DeinitDecl& decl) {
    Decl& typeDecl = findInSymbolTable(decl.getTypeName(), decl.srcLoc);
    FuncDecl funcDecl{"__deinit_" + decl.getTypeName(), {}, typeDecl.getTypeDecl().getType(),
        decl.getTypeName(), decl.body, SrcLoc::invalid()};
    decl.type = &typeDecl.getTypeDecl();
    typecheckMemberFunc(funcDecl);
}

void typecheck(TypeDecl& decl) {
    // TODO
}

TypeDecl* getTypeDecl(const BasicType& type) {
    auto it = symbolTable.find(type.name);
    if (it == symbolTable.end()) return nullptr;
    return &it->second->getTypeDecl();
}

void typecheck(VarDecl& decl) {
    if (symbolTable.count(decl.name) > 0) {
        error(decl.srcLoc, "redefinition of '", decl.name, "'");
    }
    const Type* initType = nullptr;
    if (decl.initializer) {
        initType = &typecheck(*decl.initializer);
        if (initType->isFuncType()) {
            error(decl.initializer->getSrcLoc(), "function pointers not implemented yet");
        }
    }
    if (auto declaredType = decl.getDeclaredType()) {
        if (initType && !isValidConversion(*decl.initializer, *initType, *declaredType)) {
            error(decl.initializer->getSrcLoc(), "cannot initialize variable of type '", *declaredType,
                "' with '", *initType, "'");
        }
        symbolTable.insert({decl.name, new Decl(VarDecl(decl))});
    } else {
        if (initType->isBasicType() && initType->getBasicType().name == "null") {
            error(decl.srcLoc, "couldn't infer type of '", decl.name, "', add a type annotation");
        }

        auto initTypeCopy = *initType;
        initTypeCopy.setMutable(decl.isMutable());
        decl.type = std::move(initTypeCopy);
        symbolTable.insert({decl.name, new Decl(VarDecl(decl))});
    }
}

void typecheck(FieldDecl& decl) {
}

void typecheck(ImportDecl& decl) {
    importingC = true;
    importCHeader(decl.target);
    importingC = false;
}

void typecheck(Decl& decl) {
    switch (decl.getKind()) {
        case DeclKind::ParamDecl: typecheck(decl.getParamDecl()); break;
        case DeclKind::FuncDecl:  typecheck(decl.getFuncDecl()); break;
        case DeclKind::InitDecl:  typecheck(decl.getInitDecl()); break;
        case DeclKind::DeinitDecl:typecheck(decl.getDeinitDecl()); break;
        case DeclKind::TypeDecl:  typecheck(decl.getTypeDecl()); break;
        case DeclKind::VarDecl:   typecheck(decl.getVarDecl()); break;
        case DeclKind::FieldDecl: typecheck(decl.getFieldDecl()); break;
        case DeclKind::ImportDecl:typecheck(decl.getImportDecl()); break;
    }
}

} // anonymous namespace

void delta::typecheck(std::vector<Decl>& decls) {
    for (Decl& decl : decls) ::typecheck(decl);
}
