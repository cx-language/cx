#include <iostream>
#include <limits>
#include <unordered_map>
#include <vector>
#include <cstdlib>
#include <boost/numeric/conversion/cast.hpp>
#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/Optional.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/ADT/iterator_range.h>
#include <llvm/Support/ErrorHandling.h>
#include "typecheck.h"
#include "c-import.h"
#include "../ast/type.h"
#include "../ast/expr.h"
#include "../ast/decl.h"
#include "../ast/module.h"
#include "../parser/token.h"
#include "../driver/utility.h"
#include "../irgen/mangle.h"

using namespace delta;

namespace {

std::unordered_map<std::string, Decl*> symbolTable;

/// Storage for Decls that are not in the AST but are referenced by the symbol table.
std::vector<std::unique_ptr<Decl>> nonASTDecls;

std::vector<VarDecl*> globalVariables;
std::unordered_map<std::string, Type> currentGenericArgs;
Type funcReturnType = nullptr;
bool inInitializer = false;
int breakableBlocks = 0;
bool importingC = false;
llvm::ArrayRef<llvm::StringRef> includePaths;

Type typecheck(Expr& expr);
void typecheck(Stmt& stmt);

Type typecheck(VariableExpr& expr) {
    Decl& decl = findInSymbolTable(expr.identifier, expr.srcLoc);

    switch (decl.getKind()) {
        case DeclKind::VarDecl: return decl.getVarDecl().getType();
        case DeclKind::ParamDecl: return decl.getParamDecl().type;
        case DeclKind::FuncDecl: return decl.getFuncDecl().getFuncType();
        case DeclKind::GenericParamDecl: llvm_unreachable("cannot refer to generic parameters yet");
        case DeclKind::GenericFuncDecl: llvm_unreachable("cannot refer to generic functions yet");
        case DeclKind::InitDecl: llvm_unreachable("cannot refer to initializers yet");
        case DeclKind::DeinitDecl: llvm_unreachable("cannot refer to deinitializers yet");
        case DeclKind::TypeDecl: error(expr.srcLoc, "'", expr.identifier, "' is not a variable");
        case DeclKind::FieldDecl: return decl.getFieldDecl().type;
        case DeclKind::ImportDecl: llvm_unreachable("import statement validation not implemented yet");
    }
    abort();
}

Type typecheck(StrLiteralExpr& expr) {
    return PtrType::get(ArrayType::get(Type::getChar(), int64_t(expr.value.size() + 1)), true);
}

Type typecheck(IntLiteralExpr& expr) {
    if (expr.value >= std::numeric_limits<int32_t>::min()
    &&  expr.value <= std::numeric_limits<int32_t>::max())
        return Type::getInt();
    else
    if (expr.value >= std::numeric_limits<int64_t>::min()
    &&  expr.value <= std::numeric_limits<int64_t>::max())
        return Type::getInt64();
    else
        error(expr.srcLoc, "integer literal is too large");
}

Type typecheck(FloatLiteralExpr&) {
    return Type::getFloat64();
}

Type typecheck(BoolLiteralExpr&) {
    return Type::getBool();
}

Type typecheck(NullLiteralExpr&) {
    return Type::getNull();
}

Type typecheck(ArrayLiteralExpr& array) {
    Type firstType = typecheck(*array.elements[0]);
    for (auto& element : llvm::drop_begin(array.elements, 1)) {
        Type type = typecheck(*element);
        if (type != firstType) {
            error(element->getSrcLoc(), "mixed element types in array literal (expected '",
                  firstType, "', found '", type, "')");
        }
    }
    return ArrayType::get(firstType, int64_t(array.elements.size()));
}

Type typecheck(PrefixExpr& expr) {
    Type operandType = typecheck(*expr.operand);

    if (expr.op == NOT) {
        if (!operandType.isBool()) {
            error(expr.operand->getSrcLoc(), "invalid operand type '", operandType, "' to logical not");
        }
        return operandType;
    }
    if (expr.op == STAR) { // Dereference operation
        if (!operandType.isPtrType()) {
            error(expr.operand->getSrcLoc(), "cannot dereference non-pointer type '", operandType, "'");
        }
        return operandType.getPointee();
    }
    if (expr.op == AND) { // Address-of operation
        return PtrType::get(operandType, true);
    }
    return operandType;
}

bool isValidConversion(Expr&, Type, Type);

Type typecheck(BinaryExpr& expr) {
    Type leftType = typecheck(*expr.left);
    Type rightType = typecheck(*expr.right);

    if (expr.op == AND_AND || expr.op == OR_OR) {
        if (leftType.isBool() && rightType.isBool()) {
            return Type::getBool();
        }
        error(expr.srcLoc, "invalid operands to binary expression ('", leftType, "' and '", rightType, "')");
    }

    if (expr.op.isBitwiseOperator() && (leftType.isFloatingPoint() || rightType.isFloatingPoint())) {
        error(expr.srcLoc, "invalid operands to binary expression ('", leftType, "' and '", rightType, "')");
    }

    if (!isValidConversion(*expr.left, leftType, rightType)
    &&  !isValidConversion(*expr.right, rightType, leftType)) {
        error(expr.srcLoc, "invalid operands to binary expression ('", leftType, "' and '", rightType, "')");
    }
    return expr.op.isComparisonOperator() ? Type::getBool() : leftType;
}

TypeDecl* getTypeDecl(const BasicType& type);

template<typename IntType>
bool checkRange(Expr& expr, int64_t value, llvm::StringRef param) {
    try {
        boost::numeric_cast<IntType>(value);
    } catch (...) {
        error(expr.getSrcLoc(), value, " is out of range for type '", param, "'");
    }
    expr.setType(BasicType::get(param.str()));
    return true;
}

/// Resolves generic parameters to their corresponding types, returns other types as is.
Type resolve(Type type) {
    if (!type.isBasicType()) return type;
    auto it = currentGenericArgs.find(type.getName());
    if (it == currentGenericArgs.end()) return type;
    return it->second;
}

bool isValidConversion(Expr& expr, Type unresolvedSource, Type unresolvedTarget) {
    Type source = resolve(unresolvedSource);
    Type target = resolve(unresolvedTarget);

    if (expr.isLvalue() && source.isBasicType()) {
        TypeDecl* typeDecl = getTypeDecl(llvm::cast<BasicType>(*source));
        if (typeDecl && !typeDecl->passByValue() && !target.isPtrType()) {
            error(expr.getSrcLoc(), "implicit copying of class instances is disallowed");
        }
    }
    if (source.isImplicitlyConvertibleTo(target)) return true;

    // Autocast integer literals to parameter type if within range, error out if not within range.
    if (expr.isIntLiteralExpr() && target.isBasicType()) {
        int64_t value{expr.getIntLiteralExpr().value};
        llvm::StringRef targetTypeName = target.getName();
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
    } else if (expr.isNullLiteralExpr() && target.isPtrType() && !target.isRef()) {
        expr.setType(target);
        return true;
    } else if (expr.isStrLiteralExpr() && target.isPtrType()
               && target.getPointee().isChar()
               && !target.getPointee().isMutable()) {
        // Special case: allow passing string literals as C-strings (const char*).
        expr.setType(target);
        return true;
    } else if (expr.isLvalue() && source.isBasicType() && target.isPtrType()) {
        auto typeDecl = getTypeDecl(llvm::cast<BasicType>(*source));
        if (!typeDecl || typeDecl->passByValue()) {
            error(expr.getSrcLoc(), "cannot implicitly pass value types by reference, add explicit '&'");
        }
        if (source.isImplicitlyConvertibleTo(target.getPointee())) return true;
    }

    return false;
}

void validateArgs(const std::vector<Arg>& args, const std::vector<ParamDecl>& params,
                  const std::string& funcName, SrcLoc srcLoc);

Type typecheckInitExpr(const TypeDecl& type, const std::vector<Arg>& args, SrcLoc srcLoc) {
    auto it = symbolTable.find(mangleInitDecl(type.name));
    if (it == symbolTable.end()) {
        error(srcLoc, "no matching initializer for '", type.name, "'");
    }
    validateArgs(args, it->second->getInitDecl().params, "'" + type.name + "' initializer", srcLoc);
    return type.getType();
}

void setCurrentGenericArgs(GenericFuncDecl& decl, CallExpr& call) {
    if (call.genericArgs.empty()) {
        call.genericArgs.reserve(decl.genericParams.size());
        // FIXME: The args will also be typechecked by validateArgs()
        // after this function. Get rid of this duplicated typechecking.
        for (auto& arg : call.args) call.genericArgs.emplace_back(typecheck(*arg.value));
    }
    else if (call.genericArgs.size() < decl.genericParams.size()) {
        error(call.srcLoc, "too few generic arguments to '", call.getFuncName(),
              "', expected ", decl.genericParams.size());
    }
    else if (call.genericArgs.size() > decl.genericParams.size()) {
        error(call.srcLoc, "too many generic arguments to '", call.getFuncName(),
              "', expected ", decl.genericParams.size());
    }

    auto genericArg = call.genericArgs.begin();
    for (const GenericParamDecl& genericParam : decl.genericParams) {
        currentGenericArgs.insert({genericParam.name, *genericArg++});
    }
}

Type typecheckBuiltinConversion(CallExpr& expr) {
    if (expr.args.size() != 1) {
        error(expr.srcLoc, "expected single argument to converting initializer");
    }
    if (!expr.genericArgs.empty()) {
        error(expr.srcLoc, "expected no generic arguments to converting initializer");
    }
    if (!expr.args.front().label.empty()) {
        error(expr.srcLoc, "expected no argument label to converting initializer");
    }
    typecheck(*expr.args.front().value);
    expr.isInitializerCall = true;
    expr.setType(BasicType::get(expr.getFuncName()));
    return expr.getType();
}

Type typecheck(CallExpr& expr) {
    if (!expr.callsNamedFunc()) fatalError("anonymous function calls not implemented yet");

    if (Type::isBuiltinScalar(expr.getFuncName()))
        return typecheckBuiltinConversion(expr);

    if (expr.func->isMemberExpr())
        typecheck(*expr.getReceiver());

    std::string mangledName = expr.getMangledFuncName();
    Decl& decl = findInSymbolTable(mangledName, expr.func->getSrcLoc());
    expr.isInitializerCall = decl.isTypeDecl();
    if (expr.isInitializerCall) {
        return typecheckInitExpr(decl.getTypeDecl(), expr.args, expr.srcLoc);
    } else if (expr.isMemberFuncCall()) {
        Type receiverType = expr.getReceiver()->getType();
        if (receiverType.isPtrType() && !receiverType.isRef()) {
            error(expr.getReceiver()->getSrcLoc(), "cannot call member function through pointer '",
                  receiverType, "', pointer may be null");
        }
    }
    if (decl.isFuncDecl()) {
        validateArgs(expr.args, decl.getFuncDecl().params,
                     "'" + expr.getFuncName().str() + "'", expr.srcLoc);
        return decl.getFuncDecl().getFuncType()->returnType;
    } else if (decl.isGenericFuncDecl()) {
        setCurrentGenericArgs(decl.getGenericFuncDecl(), expr);
        validateArgs(expr.args, decl.getGenericFuncDecl().func->params,
                     "'" + expr.getFuncName().str() + "'", expr.srcLoc);
        currentGenericArgs.clear();
        return decl.getGenericFuncDecl().func->getFuncType()->returnType;
    } else {
        error(expr.func->getSrcLoc(), "'", expr.getFuncName(), "' is not a function");
    }
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

Type typecheck(CastExpr& expr) {
    Type sourceType = typecheck(*expr.expr);
    Type targetType = expr.type;

    switch (sourceType.getKind()) {
        case TypeKind::BasicType:
            if (sourceType.isBool() && targetType.isInt()) {
                return targetType; // bool -> int
            }
        case TypeKind::ArrayType:
        case TypeKind::TupleType:
        case TypeKind::FuncType:
            break;
        case TypeKind::PtrType:
            Type sourcePointee = sourceType.getPointee();
            if (targetType.isPtrType()) {
                Type targetPointee = targetType.getPointee();
                if (sourcePointee.isVoid()
                && (!targetPointee.isMutable() || sourcePointee.isMutable())) {
                    return targetType; // (mutable) void* -> T* / mutable void* -> mutable T*
                }
                if (targetPointee.isVoid()
                && (!targetPointee.isMutable() || sourcePointee.isMutable())) {
                    return targetType; // (mutable) T* -> void* / mutable T* -> mutable void*
                }
            }
            break;
    }
    error(expr.srcLoc, "illegal cast from '", sourceType, "' to '", targetType, "'");
}

Type typecheck(MemberExpr& expr) {
    Type baseType = typecheck(*expr.base);

    if (baseType.isPtrType()) {
        if (!baseType.isRef()) {
            error(expr.base->srcLoc, "cannot access member through pointer '", baseType,
                  "', pointer may be null");
        }
        baseType = baseType.getPointee();
    }

    if (baseType.isArrayType()) {
        if (expr.member != "count")
            error(expr.srcLoc, "no member named '", expr.member, "' in '", baseType, "'");
        return Type::getInt();
    }

    Decl& typeDecl = findInSymbolTable(baseType.getName(), SrcLoc::invalid());

    for (const FieldDecl& field : typeDecl.getTypeDecl().fields) {
        if (field.name == expr.member) {
            return field.type;
        }
    }
    error(expr.srcLoc, "no member named '", expr.member, "' in '", baseType, "'");
}

Type typecheck(SubscriptExpr& expr) {
    Type lhsType = typecheck(*expr.array);
    const ArrayType* arrayType;

    if (lhsType.isArrayType()) {
        arrayType = &llvm::cast<ArrayType>(*lhsType);
    } else if (lhsType.isRef() && lhsType.getReferee().isArrayType()) {
        arrayType = &llvm::cast<ArrayType>(*lhsType.getReferee());
    } else {
        error(expr.array->getSrcLoc(), "cannot subscript '", lhsType, "', expected array or reference-to-array");
    }

    Type indexType = typecheck(*expr.index);
    if (!isValidConversion(*expr.index, indexType, Type::getInt())) {
        error(expr.index->getSrcLoc(), "illegal subscript index type '", indexType, "', expected 'int'");
    }

    if (!arrayType->isUnsized() && expr.index->isIntLiteralExpr()
    && expr.index->getIntLiteralExpr().value >= arrayType->size) {
        error(expr.index->getSrcLoc(), "accessing array out-of-bounds with index ",
              expr.index->getIntLiteralExpr().value, ", array size is ", arrayType->size);
    }

    return arrayType->elementType;
}

Type typecheck(UnwrapExpr& expr) {
    Type type = typecheck(*expr.operand);
    if (!type.isNullablePointer())
        error(expr.srcLoc, "cannot unwrap non-pointer type '", type, "'");
    return PtrType::get(type.getPointee(), true);
}

Type typecheck(Expr& expr) {
    llvm::Optional<Type> type;
    switch (expr.getKind()) {
        case ExprKind::VariableExpr:    type = typecheck(expr.getVariableExpr()); break;
        case ExprKind::StrLiteralExpr:  type = typecheck(expr.getStrLiteralExpr()); break;
        case ExprKind::IntLiteralExpr:  type = typecheck(expr.getIntLiteralExpr()); break;
        case ExprKind::FloatLiteralExpr:type = typecheck(expr.getFloatLiteralExpr()); break;
        case ExprKind::BoolLiteralExpr: type = typecheck(expr.getBoolLiteralExpr()); break;
        case ExprKind::NullLiteralExpr: type = typecheck(expr.getNullLiteralExpr()); break;
        case ExprKind::ArrayLiteralExpr:type = typecheck(expr.getArrayLiteralExpr()); break;
        case ExprKind::PrefixExpr:      type = typecheck(expr.getPrefixExpr()); break;
        case ExprKind::BinaryExpr:      type = typecheck(expr.getBinaryExpr()); break;
        case ExprKind::CallExpr:        type = typecheck(expr.getCallExpr()); break;
        case ExprKind::CastExpr:        type = typecheck(expr.getCastExpr()); break;
        case ExprKind::MemberExpr:      type = typecheck(expr.getMemberExpr()); break;
        case ExprKind::SubscriptExpr:   type = typecheck(expr.getSubscriptExpr()); break;
        case ExprKind::UnwrapExpr:      type = typecheck(expr.getUnwrapExpr()); break;
    }
    assert(*type);
    expr.setType(*type);
    return expr.getType();
}

bool isValidConversion(std::vector<std::unique_ptr<Expr>>& exprs, Type source, Type target) {
    if (!source.isTupleType()) {
        assert(!target.isTupleType());
        assert(exprs.size() == 1);
        return isValidConversion(*exprs[0], source, target);
    }
    assert(target.isTupleType());

    for (int i = 0; i < exprs.size(); ++i) {
        if (!isValidConversion(*exprs[i], source.getSubtypes()[i], target.getSubtypes()[i])) {
            return false;
        }
    }
    return true;
}

void typecheck(ReturnStmt& stmt) {
    if (stmt.values.empty()) {
        if (!funcReturnType.isVoid()) {
            error(stmt.srcLoc, "expected return statement to return a value of type '", funcReturnType, "'");
        }
        return;
    }
    std::vector<Type> returnValueTypes;
    for (auto& expr : stmt.values) {
        returnValueTypes.push_back(typecheck(*expr));
    }
    Type returnType = returnValueTypes.size() > 1
        ? TupleType::get(std::move(returnValueTypes)) : returnValueTypes[0];
    if (!isValidConversion(stmt.values, returnType, funcReturnType)) {
        error(stmt.srcLoc, "mismatching return type '", returnType, "', expected '", funcReturnType, "'");
    }
}

void typecheck(VarDecl& decl, bool isGlobal);

void typecheck(VariableStmt& stmt) {
    typecheck(*stmt.decl, false);
}

void typecheck(IncrementStmt& stmt) {
    auto type = typecheck(*stmt.operand);
    if (!type.isMutable()) {
        error(stmt.srcLoc, "cannot increment immutable value");
    }
    // TODO: check that operand supports increment operation.
}

void typecheck(DecrementStmt& stmt) {
    auto type = typecheck(*stmt.operand);
    if (!type.isMutable()) {
        error(stmt.srcLoc, "cannot decrement immutable value");
    }
    // TODO: check that operand supports decrement operation.
}

void typecheck(IfStmt& ifStmt) {
    Type conditionType = typecheck(*ifStmt.condition);
    if (!conditionType.isBool()) {
        error(ifStmt.condition->getSrcLoc(), "'if' condition must have type 'bool'");
    }
    for (auto& stmt : ifStmt.thenBody) typecheck(*stmt);
    for (auto& stmt : ifStmt.elseBody) typecheck(*stmt);
}

void typecheck(SwitchStmt& stmt) {
    Type conditionType = typecheck(*stmt.condition);
    breakableBlocks++;
    for (SwitchCase& switchCase : stmt.cases) {
        Type caseType = typecheck(*switchCase.value);
        if (caseType != conditionType) {
            error(switchCase.value->getSrcLoc(), "case value type '", caseType,
                  "' doesn't match switch condition type '", conditionType, "'");
        }
        for (auto& caseStmt : switchCase.stmts) typecheck(*caseStmt);
    }
    for (auto& defaultStmt : stmt.defaultStmts) typecheck(*defaultStmt);
    breakableBlocks--;
}

void typecheck(WhileStmt& whileStmt) {
    Type conditionType = typecheck(*whileStmt.condition);
    if (!conditionType.isBool()) {
        error(whileStmt.condition->getSrcLoc(), "'while' condition must have type 'bool'");
    }
    breakableBlocks++;
    for (auto& stmt : whileStmt.body) typecheck(*stmt);
    breakableBlocks--;
}

void typecheck(BreakStmt& breakStmt) {
    if (breakableBlocks == 0) {
        error(breakStmt.srcLoc, "'break' is only allowed inside 'while' and 'switch' statements");
    }
}

void typecheckAssignment(Expr& lhs, Expr& rhs, SrcLoc srcLoc) {
    Type lhsType = typecheck(lhs);
    if (lhsType.isFuncType()) error(srcLoc, "cannot assign to function");
    Type rhsType = typecheck(rhs);
    if (!isValidConversion(rhs, rhsType, lhsType)) {
        error(rhs.getSrcLoc(), "cannot assign '", rhsType, "' to variable of type '", lhsType, "'");
    }
    if (!lhsType.isMutable() && !inInitializer) {
        if (lhs.isVariableExpr()) {
            error(srcLoc, "cannot assign to immutable variable '",
                  lhs.getVariableExpr().identifier, "'");
        } else {
            error(srcLoc, "cannot assign to immutable expression");
        }
    }
}

void typecheck(AssignStmt& stmt) {
    typecheckAssignment(*stmt.lhs, *stmt.rhs, stmt.srcLoc);
}

void typecheck(AugAssignStmt& stmt) {
    BinaryExpr expr(stmt.op, std::unique_ptr<Expr>(stmt.lhs.get()),
                    std::unique_ptr<Expr>(stmt.rhs.get()), stmt.srcLoc);
    typecheckAssignment(*stmt.lhs, expr, stmt.srcLoc);
    expr.getBinaryExpr().left.release();
    expr.getBinaryExpr().right.release();
}

void typecheck(Stmt& stmt) {
    switch (stmt.getKind()) {
        case StmtKind::ReturnStmt:    typecheck(stmt.getReturnStmt()); break;
        case StmtKind::VariableStmt:  typecheck(stmt.getVariableStmt()); break;
        case StmtKind::IncrementStmt: typecheck(stmt.getIncrementStmt()); break;
        case StmtKind::DecrementStmt: typecheck(stmt.getDecrementStmt()); break;
        case StmtKind::ExprStmt:      typecheck(*stmt.getExprStmt().expr); break;
        case StmtKind::DeferStmt:     typecheck(*stmt.getDeferStmt().expr); break;
        case StmtKind::IfStmt:        typecheck(stmt.getIfStmt()); break;
        case StmtKind::SwitchStmt:    typecheck(stmt.getSwitchStmt()); break;
        case StmtKind::WhileStmt:     typecheck(stmt.getWhileStmt()); break;
        case StmtKind::BreakStmt:     typecheck(stmt.getBreakStmt()); break;
        case StmtKind::AssignStmt:    typecheck(stmt.getAssignStmt()); break;
        case StmtKind::AugAssignStmt: typecheck(stmt.getAugAssignStmt()); break;
    }
}

void typecheck(ParamDecl& decl) {
    if (symbolTable.count(decl.name) > 0) {
        error(decl.srcLoc, "redefinition of '", decl.name, "'");
    }
    symbolTable.insert({ decl.name, &decl });
}

} // anonymous namespace

void delta::addToSymbolTable(FuncDecl& decl) {
    if (!importingC && symbolTable.count(mangle(decl)) > 0) {
        error(decl.srcLoc, "redefinition of '", decl.name, "'");
    }
    symbolTable.insert({ mangle(decl), &decl });
}

void delta::addToSymbolTable(GenericFuncDecl& decl) {
    if (symbolTable.count(decl.func->name) > 0) {
        error(decl.func->srcLoc, "redefinition of '", decl.func->name, "'");
    }
    symbolTable.insert({ decl.func->name, &decl });
}

void delta::addToSymbolTable(InitDecl& decl) {
    if (symbolTable.count(mangle(decl)) > 0) {
        error(decl.srcLoc, "redefinition of '", decl.getTypeName(), "' initializer");
    }
    Decl& typeDecl = findInSymbolTable(decl.getTypeName(), decl.srcLoc);
    if (!typeDecl.isTypeDecl()) error(decl.srcLoc, "'", decl.getTypeName(), "' is not a class or struct");
    decl.type = &typeDecl.getTypeDecl();

    symbolTable.insert({ mangle(decl), &decl });
}

void delta::addToSymbolTable(DeinitDecl& decl) {
    if (symbolTable.count(mangle(decl)) > 0) {
        error(decl.srcLoc, "redefinition of '", decl.getTypeName(), "' deinitializer");
    }
    Decl& typeDecl = findInSymbolTable(decl.getTypeName(), decl.srcLoc);
    if (!typeDecl.isTypeDecl()) error(decl.srcLoc, "'", decl.getTypeName(), "' is not a class or struct");
    decl.type = &typeDecl.getTypeDecl();

    symbolTable.insert({ mangle(decl), &decl });
}

void delta::addToSymbolTable(TypeDecl& decl) {
    if (!importingC && symbolTable.count(decl.name) > 0) {
        error(decl.srcLoc, "redefinition of '", decl.name, "'");
    }
    symbolTable.insert({ decl.name, &decl });
}

void delta::addToSymbolTable(VarDecl& decl) {
    if (!importingC && symbolTable.count(decl.name) > 0) {
        error(decl.srcLoc, "redefinition of '", decl.name, "'");
    }
    symbolTable.insert({ decl.name, &decl });
    globalVariables.push_back(&decl);
}

void delta::addToSymbolTable(FuncDecl&& decl) {
    std::string name = decl.name;
    nonASTDecls.push_back(llvm::make_unique<FuncDecl>(std::move(decl)));
    symbolTable.insert({ std::move(name), nonASTDecls.back().get() });
}

void delta::addToSymbolTable(TypeDecl&& decl) {
    std::string name = decl.name;
    nonASTDecls.push_back(llvm::make_unique<TypeDecl>(std::move(decl)));
    symbolTable.insert({ std::move(name), nonASTDecls.back().get() });
}

void delta::addToSymbolTable(VarDecl&& decl) {
    std::string name = decl.name;
    nonASTDecls.push_back(llvm::make_unique<VarDecl>(std::move(decl)));
    symbolTable.insert({ std::move(name), nonASTDecls.back().get() });
}

Decl& delta::findInSymbolTable(llvm::StringRef name, SrcLoc srcLoc) {
    auto it = findInSymbolTable(name);
    if (!it) error(srcLoc, "unknown identifier '", name, "'");
    return *it;
}

Decl* delta::findInSymbolTable(llvm::StringRef name) {
    auto it = symbolTable.find(name);
    return it != symbolTable.end() ? &*it->second : nullptr;
}

namespace {

void typecheckMemberFunc(FuncDecl& decl);

void typecheck(FuncDecl& decl) {
    if (!decl.receiverType.empty()) return typecheckMemberFunc(decl);
    if (decl.isExtern()) return;
    auto symbolTableBackup = symbolTable;
    for (ParamDecl& param : decl.params) {
        if (param.type.isMutable()) error(param.srcLoc, "parameter types cannot be 'mutable'");
        typecheck(param);
    }
    if (decl.returnType.isMutable()) error(decl.srcLoc, "return types cannot be 'mutable'");
    funcReturnType = decl.returnType;
    for (auto& stmt : *decl.body) typecheck(*stmt);
    funcReturnType = nullptr;
    symbolTable = std::move(symbolTableBackup);
}

void typecheckMemberFunc(FuncDecl& decl) {
    auto symbolTableBackup = symbolTable;
    Decl& receiverType = findInSymbolTable(decl.receiverType, decl.srcLoc);
    if (!receiverType.isTypeDecl()) error(decl.srcLoc, "'", decl.receiverType, "' is not a class or struct");
    symbolTable.emplace("this", new VarDecl(receiverType.getTypeDecl().getTypeForPassing(),
                                            "this", nullptr, SrcLoc::invalid()));
    for (ParamDecl& param : decl.params) typecheck(param);
    funcReturnType = decl.returnType;
    for (auto& stmt : *decl.body) typecheck(*stmt);
    funcReturnType = nullptr;
    symbolTable = std::move(symbolTableBackup);
}

void typecheck(GenericParamDecl& decl) {
    if (symbolTable.count(decl.name) > 0) {
        error(decl.srcLoc, "redefinition of '", decl.name, "'");
    }
}

void typecheck(GenericFuncDecl& decl) {
    for (auto& genericParam : decl.genericParams) typecheck(genericParam);
    typecheck(*decl.func);
}

void typecheck(InitDecl& decl) {
    auto symbolTableBackup = symbolTable;
    Decl& typeDecl = findInSymbolTable(decl.getTypeName(), decl.srcLoc);
    if (!typeDecl.isTypeDecl()) error(decl.srcLoc, "'", decl.getTypeName(), "' is not a class or struct");
    decl.type = &typeDecl.getTypeDecl();
    symbolTable.insert({ "this", new VarDecl(typeDecl.getTypeDecl().getType(),
                                             "this", nullptr, SrcLoc::invalid()) });
    for (ParamDecl& param : decl.params) typecheck(param);
    inInitializer = true;
    for (auto& stmt : *decl.body) typecheck(*stmt);
    inInitializer = false;
    symbolTable = std::move(symbolTableBackup);
}

void typecheck(DeinitDecl& decl) {
    Decl& typeDecl = findInSymbolTable(decl.getTypeName(), decl.srcLoc);
    FuncDecl funcDecl(mangle(decl), {}, typeDecl.getTypeDecl().getType(),
                      std::string(decl.getTypeName()), SrcLoc::invalid());
    funcDecl.body = decl.body;
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

void typecheck(VarDecl& decl, bool isGlobal) {
    if (!isGlobal && symbolTable.count(decl.name) > 0) {
        error(decl.srcLoc, "redefinition of '", decl.name, "'");
    }
    Type initType = nullptr;
    if (decl.initializer) {
        initType = typecheck(*decl.initializer);
        if (initType.isFuncType()) {
            error(decl.initializer->getSrcLoc(), "function pointers not implemented yet");
        }
    } else if (isGlobal) {
        error(decl.srcLoc, "global variables cannot be uninitialized");
    }

    if (auto declaredType = decl.getType()) {
        if (initType && !isValidConversion(*decl.initializer, initType, declaredType)) {
            error(decl.initializer->getSrcLoc(), "cannot initialize variable of type '", declaredType,
                "' with '", initType, "'");
        }
        symbolTable.insert({ decl.name, new VarDecl(decl) });
    } else {
        if (initType.isNull()) {
            error(decl.srcLoc, "couldn't infer type of '", decl.name, "', add a type annotation");
        }

        initType.setMutable(decl.getType().isMutable());
        decl.type = initType;
        symbolTable.insert({ decl.name, new VarDecl(decl) });
    }
}

void typecheck(FieldDecl& decl) {
}

void typecheck(ImportDecl& decl) {
    importingC = true;
    importCHeader(decl.target, includePaths);
    importingC = false;
}

void typecheck(Decl& decl) {
    switch (decl.getKind()) {
        case DeclKind::ParamDecl: typecheck(decl.getParamDecl()); break;
        case DeclKind::FuncDecl:  typecheck(decl.getFuncDecl()); break;
        case DeclKind::GenericParamDecl: typecheck(decl.getGenericParamDecl()); break;
        case DeclKind::GenericFuncDecl: typecheck(decl.getGenericFuncDecl()); break;
        case DeclKind::InitDecl:  typecheck(decl.getInitDecl()); break;
        case DeclKind::DeinitDecl:typecheck(decl.getDeinitDecl()); break;
        case DeclKind::TypeDecl:  typecheck(decl.getTypeDecl()); break;
        case DeclKind::VarDecl:   typecheck(decl.getVarDecl(), true); break;
        case DeclKind::FieldDecl: typecheck(decl.getFieldDecl()); break;
        case DeclKind::ImportDecl:typecheck(decl.getImportDecl()); break;
    }
}

} // anonymous namespace

void delta::typecheck(Module& module, const std::vector<llvm::StringRef>& includePaths) {
    ::includePaths = includePaths;

    // Infer the types of global variables for use before their declaration.
    for (VarDecl* var : globalVariables) {
        ::typecheck(*var, true);
    }

    for (auto& fileUnit : module.getFileUnits()) {
        for (auto& decl : fileUnit.getTopLevelDecls()) {
            if (!decl->isVarDecl()) {
                ::typecheck(*decl);
            }
        }
    }
}
