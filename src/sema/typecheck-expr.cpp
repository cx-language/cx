#include "typecheck.h"
#include <algorithm>
#include <limits>
#include <string>
#include <vector>
#pragma warning(push, 0)
#include <llvm/ADT/APSInt.h>
#include <llvm/ADT/Optional.h>
#include <llvm/ADT/StringExtras.h>
#include <llvm/ADT/iterator_range.h>
#include <llvm/Support/ErrorHandling.h>
#pragma warning(pop)
#include "../ast/decl.h"
#include "../ast/expr.h"
#include "../ast/mangle.h"
#include "../ast/module.h"
#include "../ast/type.h"
#include "../support/utility.h"

using namespace delta;

static void checkNotMoved(const Decl& decl, const VarExpr& expr) {
    const Movable* movable;

    switch (decl.getKind()) {
        case DeclKind::ParamDecl:
            movable = &llvm::cast<ParamDecl>(decl);
            break;
        case DeclKind::VarDecl:
            movable = &llvm::cast<VarDecl>(decl);
            break;
        default:
            return;
    }

    if (movable->isMoved()) {
        std::string typeInfo;

        if (expr.hasType()) {
            typeInfo = " of type '" + expr.getType().toString(true) + "'";
        }

        ERROR(expr.getLocation(), "use of moved value '" << expr.getIdentifier() << "'" << typeInfo);
    }
}

void Typechecker::checkHasAccess(const Decl& decl, SourceLocation location, AccessLevel userAccessLevel) {
    // FIXME: Compare SourceFile objects instead of file path strings.
    if (decl.getAccessLevel() == AccessLevel::Private && strcmp(decl.getLocation().file, location.file) != 0) {
        WARN(location, "'" << decl.getName() << "' is private");
    } else if (userAccessLevel != AccessLevel::None && decl.getAccessLevel() < userAccessLevel) {
        WARN(location, "using " << decl.getAccessLevel() << " type '" << decl.getName() << "' in " << userAccessLevel << " declaration");
    }
}

void Typechecker::checkLambdaCapture(const VariableDecl& variableDecl, const VarExpr& varExpr) const {
    auto* parent = getCurrentModule()->getSymbolTable().getCurrentScope().parent;
    if (parent && parent->isLambda() && variableDecl.getParent() != parent) {
        ERROR(varExpr.getLocation(), "lambda capturing not implemented yet");
    }
}

Type Typechecker::typecheckVarExpr(VarExpr& expr, bool useIsWriteOnly) {
    auto& decl = findDecl(expr.getIdentifier(), expr.getLocation());
    checkHasAccess(decl, expr.getLocation(), AccessLevel::None);
    decl.setReferenced(true);
    expr.setDecl(&decl);

    if (auto variableDecl = llvm::dyn_cast<VariableDecl>(&decl)) {
        checkLambdaCapture(*variableDecl, expr);
    }

    switch (decl.getKind()) {
        case DeclKind::VarDecl:
            if (!useIsWriteOnly) checkNotMoved(decl, expr);
            return llvm::cast<VarDecl>(decl).getType();
        case DeclKind::ParamDecl:
            if (!useIsWriteOnly) checkNotMoved(decl, expr);
            return llvm::cast<ParamDecl>(decl).getType();
        case DeclKind::FunctionDecl:
        case DeclKind::MethodDecl:
            return Type(llvm::cast<FunctionDecl>(decl).getFunctionType(), Mutability::Mutable, SourceLocation());
        case DeclKind::GenericParamDecl:
            llvm_unreachable("cannot refer to generic parameters yet");
        case DeclKind::InitDecl:
            llvm_unreachable("cannot refer to initializers yet");
        case DeclKind::DeinitDecl:
            llvm_unreachable("cannot refer to deinitializers yet");
        case DeclKind::FunctionTemplate:
            llvm_unreachable("cannot refer to generic functions yet");
        case DeclKind::TypeDecl:
            ERROR(expr.getLocation(), "'" << expr.getIdentifier() << "' is not a variable");
        case DeclKind::TypeTemplate:
            llvm_unreachable("cannot refer to generic types yet");
        case DeclKind::EnumDecl:
            ERROR(expr.getLocation(), "'" << expr.getIdentifier() << "' is not a variable");
        case DeclKind::EnumCase:
            return llvm::cast<EnumCase>(decl).getType();
        case DeclKind::FieldDecl:
            return llvm::cast<FieldDecl>(decl).getType();
        case DeclKind::ImportDecl:
            llvm_unreachable("import statement validation not implemented yet");
    }
    llvm_unreachable("all cases handled");
}

Type typecheckStringLiteralExpr(StringLiteralExpr&) {
    return BasicType::get("StringRef", {});
}

Type typecheckCharacterLiteralExpr(CharacterLiteralExpr&) {
    return Type::getChar();
}

Type typecheckIntLiteralExpr(IntLiteralExpr& expr) {
    if (expr.getValue().isSignedIntN(32)) {
        return Type::getInt();
    }
    if (expr.getValue().isSignedIntN(64)) {
        return Type::getInt64();
    }
    ERROR(expr.getLocation(), "integer literal is too large");
}

Type typecheckFloatLiteralExpr(FloatLiteralExpr&) {
    return Type::getFloat64();
}

Type typecheckBoolLiteralExpr(BoolLiteralExpr&) {
    return Type::getBool();
}

Type typecheckNullLiteralExpr(NullLiteralExpr&) {
    return Type::getNull();
}

Type typecheckUndefinedLiteralExpr(UndefinedLiteralExpr&) {
    return Type::getUndefined();
}

Type Typechecker::typecheckArrayLiteralExpr(ArrayLiteralExpr& array) {
    Type firstType = typecheckExpr(*array.getElements()[0]);
    for (auto& element : llvm::drop_begin(array.getElements(), 1)) {
        Type type = typecheckExpr(*element);
        if (type != firstType) {
            ERROR(element->getLocation(), "mixed element types in array literal (expected '" << firstType << "', found '" << type << "')");
        }
    }
    return ArrayType::get(firstType, int64_t(array.getElements().size()));
}

Type Typechecker::typecheckTupleExpr(TupleExpr& expr) {
    auto elements = map(expr.getElements(), [&](NamedValue& namedValue) {
        return TupleElement{ namedValue.getName(), typecheckExpr(*namedValue.getValue()) };
    });
    return TupleType::get(std::move(elements));
}

static bool isPointerOp(const UnaryExpr& expr) {
    if (expr.getOperator() == Token::Star || expr.getOperator() == Token::And) return true;
    auto operandType = expr.getOperand().getType();
    if (!operandType.isPointerType()) return false;
    if (!operandType.getPointee().isArrayWithUnknownSize()) return false;
    return expr.getOperator() == Token::Increment || expr.getOperator() == Token::Decrement;
}

Type Typechecker::typecheckUnaryExpr(UnaryExpr& expr) {
    Type operandType = typecheckExpr(expr.getOperand());

    if (!isPointerOp(expr)) {
        operandType = operandType.removePointer();
    }

    if (expr.getOperator() == Token::Not) {
        if (!operandType.isBool() && !operandType.isOptionalType()) {
            ERROR(expr.getOperand().getLocation(), "invalid operand type '" << operandType << "' to logical not");
        }
        return Type::getBool();
    }

    if (expr.getOperator() == Token::Star) { // Dereference operation
        if (operandType.isOptionalType() && operandType.getWrappedType().isPointerType()) {
            WARN(expr.getLocation(), "dereferencing value of optional type '"
                                         << operandType << "' which may be null; "
                                         << "unwrap the value with a postfix '!' to silence this warning");
            operandType = operandType.getWrappedType();
        } else if (!operandType.isPointerType()) {
            ERROR(expr.getLocation(), "cannot dereference non-pointer type '" << operandType << "'");
        }
        return operandType.getPointee().removeArrayWithUnknownSize();
    }

    if (expr.getOperator() == Token::And) { // Address-of operation
        return PointerType::get(operandType);
    }

    if (expr.getOperator() == Token::Increment) {
        if (!operandType.isMutable()) {
            ERROR(expr.getLocation(), "cannot increment immutable value of type '" << operandType << "'");
        }
        if (operandType.isPointerType() && !operandType.getPointee().isArrayWithUnknownSize()) {
            auto expectedType = PointerType::get(
                ArrayType::get(operandType.getPointee(), ArrayType::unknownSize, operandType.getPointee().getMutability()));
            ERROR(expr.getLocation(), "cannot increment '" << operandType << "', should be '" << expectedType << "'");
        }
        if (!operandType.isIncrementable()) {
            ERROR(expr.getLocation(), "cannot increment '" << operandType << "'");
        }
        return Type::getVoid();
    }

    if (expr.getOperator() == Token::Decrement) {
        if (!operandType.isMutable()) {
            ERROR(expr.getLocation(), "cannot decrement immutable value of type '" << operandType << "'");
        }
        if (operandType.isPointerType() && !operandType.getPointee().isArrayWithUnknownSize()) {
            auto expectedType = PointerType::get(
                ArrayType::get(operandType.getPointee(), ArrayType::unknownSize, operandType.getPointee().getMutability()));
            ERROR(expr.getLocation(), "cannot decrement '" << operandType << "', should be '" << expectedType << "'");
        }
        if (!operandType.isDecrementable()) {
            ERROR(expr.getLocation(), "cannot decrement '" << operandType << "'");
        }
        return Type::getVoid();
    }

    return operandType;
}

static void invalidOperandsToBinaryExpr(const BinaryExpr& expr, Token::Kind op) {
    std::string hint;

    if ((expr.getRHS().isNullLiteralExpr() || expr.getLHS().isNullLiteralExpr()) && (op == Token::Equal || op == Token::NotEqual)) {
        hint += " (non-optional type '";
        if (expr.getRHS().isNullLiteralExpr()) {
            hint += expr.getLHS().getType().toString(true);
        } else {
            hint += expr.getRHS().getType().toString(true);
        }
        hint += "' cannot be null)";
    } else {
        hint = "";
    }

    ERROR(expr.getLocation(), "invalid operands '" << expr.getLHS().getType() << "' and '" << expr.getRHS().getType() << "' to '"
                                                   << toString(op) << "'" << hint);
}

static bool allowAssignmentOfUndefined(const Expr& lhs, const FunctionDecl* currentFunction) {
    if (auto* initDecl = llvm::dyn_cast<InitDecl>(currentFunction)) {
        switch (lhs.getKind()) {
            case ExprKind::VarExpr: {
                auto* fieldDecl = llvm::dyn_cast<FieldDecl>(llvm::cast<VarExpr>(lhs).getDecl());
                return fieldDecl && fieldDecl->getParent() == initDecl->getTypeDecl();
            }
            case ExprKind::MemberExpr: {
                auto* varExpr = llvm::dyn_cast<VarExpr>(llvm::cast<MemberExpr>(lhs).getBaseExpr());
                return varExpr && varExpr->getIdentifier() == "this";
            }
            default:
                return false;
        }
    }
    return false;
}

Type Typechecker::typecheckBinaryExpr(BinaryExpr& expr) {
    auto op = expr.getOperator();

    if (op == Token::Assignment) {
        typecheckAssignment(expr.getLHS(), expr.getRHS(), expr.getLocation());
        return Type::getVoid();
    }

    if (isCompoundAssignmentOperator(op)) {
        auto rhs = new BinaryExpr(withoutCompoundEqSuffix(op), &expr.getLHS(), &expr.getRHS(), expr.getLocation());
        expr = BinaryExpr(Token::Assignment, &expr.getLHS(), rhs, expr.getLocation());
        return typecheckBinaryExpr(expr);
    }

    Type leftType = typecheckExpr(expr.getLHS());
    Type rightType = typecheckExpr(expr.getRHS());

    if (!isBuiltinOp(op, leftType, rightType)) {
        return typecheckCallExpr(expr);
    }

    if (op == Token::AndAnd || op == Token::OrOr) {
        if (leftType.isBool() && rightType.isBool()) {
            return Type::getBool();
        }
        invalidOperandsToBinaryExpr(expr, op);
    }

    if (op == Token::PointerEqual || op == Token::PointerNotEqual) {
        if (!leftType.removeOptional().isPointerType() || !rightType.removeOptional().isPointerType()) {
            ERROR(expr.getLocation(), "both operands to pointer comparison operator must have pointer type");
        }

        auto leftPointeeType = leftType.removeOptional().removePointer().removeArrayWithUnknownSize();
        auto rightPointeeType = rightType.removeOptional().removePointer().removeArrayWithUnknownSize();

        if (!leftPointeeType.equalsIgnoreTopLevelMutable(rightPointeeType)) {
            WARN(expr.getLocation(), "pointers to different types are not allowed to be equal (got '" << leftType << "' and '" << rightType << "')");
        }
    }

    if (isBitwiseOperator(op) && (leftType.isFloatingPoint() || rightType.isFloatingPoint())) {
        invalidOperandsToBinaryExpr(expr, op);
    }

    bool converted = convert(&expr.getRHS(), leftType, true) || convert(&expr.getLHS(), rightType, true);

    if (!converted && (!leftType.removeOptional().isPointerType() || !rightType.removeOptional().isPointerType())) {
        invalidOperandsToBinaryExpr(expr, op);
    }

    return isComparisonOperator(op) ? Type::getBool() : expr.getLHS().getType();
}

void Typechecker::typecheckAssignment(Expr& lhs, Expr& rhs, SourceLocation location) {
    if (!lhs.isLvalue()) {
        ERROR(lhs.getLocation(), "expression is not assignable");
    }

    typecheckExpr(lhs, true);
    Type lhsType = lhs.getAssignableType();
    Type rhsType = typecheckExpr(rhs);

    if (rhs.isUndefinedLiteralExpr() && !allowAssignmentOfUndefined(lhs, currentFunction)) {
        ERROR(rhs.getLocation(), "'undefined' is only allowed as an initial value");
    }

    if (!convert(&rhs, lhsType)) {
        ERROR(location, "cannot assign '" << rhsType << "' to variable of type '" << lhsType << "'");
    }

    if (!lhsType.isMutable()) {
        switch (lhs.getKind()) {
            case ExprKind::VarExpr: {
                auto identifier = llvm::cast<VarExpr>(lhs).getIdentifier();
                ERROR(location, "cannot assign to immutable variable '" << identifier << "' of type '" << lhsType << "'");
            }
            case ExprKind::MemberExpr: {
                auto memberName = llvm::cast<MemberExpr>(lhs).getMemberName();
                ERROR(location, "cannot assign to immutable variable '" << memberName << "' of type '" << lhsType << "'");
            }
            default:
                ERROR(location, "cannot assign to immutable expression of type '" << lhsType << "'");
        }
    }

    if (!rhsType.isImplicitlyCopyable() && !lhsType.removeOptional().isPointerType()) {
        rhs.setMoved(true);
        lhs.setMoved(false);
    }

    if (onAssign) onAssign(lhs);
}

static bool checkRange(const Expr& expr, const llvm::APSInt& value, Type type, Type* convertedType) {
    if (llvm::APSInt::compareValues(value, llvm::APSInt::getMinValue(type.getIntegerBitWidth(), type.isUnsigned())) < 0 ||
        llvm::APSInt::compareValues(value, llvm::APSInt::getMaxValue(type.getIntegerBitWidth(), type.isUnsigned())) > 0) {
        ERROR(expr.getLocation(), value << " is out of range for type '" << type << "'");
    }

    if (convertedType) *convertedType = type;
    return true;
}

static bool hasField(TypeDecl& type, const FieldDecl& field) {
    return llvm::any_of(type.getFields(), [&](const FieldDecl& ownField) {
        return ownField.getName() == field.getName() && ownField.getType() == field.getType();
    });
}

bool Typechecker::hasMethod(TypeDecl& type, FunctionDecl& functionDecl) const {
    auto decls = findDecls(getQualifiedFunctionName(type.getType(), functionDecl.getName(), {}));

    for (Decl* decl : decls) {
        if (!decl->isFunctionDecl()) continue;
        if (!llvm::cast<FunctionDecl>(decl)->getTypeDecl()) continue;
        if (llvm::cast<FunctionDecl>(decl)->getTypeDecl()->getName() != type.getName()) continue;
        if (!llvm::cast<FunctionDecl>(decl)->signatureMatches(functionDecl, /* matchReceiver: */ false)) continue;
        return true;
    }

    return false;
}

bool Typechecker::providesInterfaceRequirements(TypeDecl& type, TypeDecl& interface, std::string* errorReason) const {
    auto thisTypeResolvedInterface = llvm::cast<TypeDecl>(interface.instantiate({ { "This", type.getType() } }, {}));

    for (auto& fieldRequirement : thisTypeResolvedInterface->getFields()) {
        if (!hasField(type, fieldRequirement)) {
            if (errorReason) {
                *errorReason = ("doesn't have field '" + fieldRequirement.getName() + "'").str();
            }
            return false;
        }
    }

    for (auto& requiredMethod : thisTypeResolvedInterface->getMethods()) {
        if (auto* functionDecl = llvm::dyn_cast<FunctionDecl>(requiredMethod)) {
            if (functionDecl->hasBody()) continue;

            if (!hasMethod(type, *functionDecl)) {
                if (errorReason) {
                    *errorReason = ("doesn't have member function '" + functionDecl->getName() + "'").str();
                }
                return false;
            }
        } else {
            ERROR(requiredMethod->getLocation(), "non-function interface member requirements are not supported yet");
        }
    }

    return true;
}

bool Typechecker::convert(Expr* expr, Type type, bool allowPointerToTemporary) const {
    Type convertedType;
    bool converted = isImplicitlyConvertible(expr, expr->getType(), type, &convertedType, allowPointerToTemporary);
    if (convertedType) expr->setType(convertedType);
    return converted;
}

bool Typechecker::isImplicitlyConvertible(const Expr* expr, Type source, Type target, Type* convertedType, bool allowPointerToTemporary) const {
    switch (source.getKind()) {
        case TypeKind::BasicType:
            if (target.isBasicType() && source.getName() == target.getName() && source.getGenericArgs() == target.getGenericArgs()) {
                return true;
            }
            break;
        case TypeKind::ArrayType:
            if (target.isArrayType() && (source.getArraySize() == target.getArraySize() || target.isArrayWithRuntimeSize()) &&
                isImplicitlyConvertible(nullptr, source.getElementType(), target.getElementType(), nullptr)) {
                return true;
            }
            break;
        case TypeKind::TupleType:
            if (target.isTupleType() && source.getTupleElements() == target.getTupleElements()) {
                return true;
            }
            break;
        case TypeKind::FunctionType:
            if (target.isFunctionType() && source.getReturnType() == target.getReturnType() && source.getParamTypes() == target.getParamTypes()) {
                return true;
            }
            break;
        case TypeKind::PointerType:
            if (target.isPointerType() && (source.getPointee().isMutable() || !target.getPointee().isMutable()) &&
                (isImplicitlyConvertible(nullptr, source.getPointee(), target.getPointee(), nullptr) || target.getPointee().isVoid())) {
                return true;
            }
            break;
        case TypeKind::OptionalType:
            if (target.isOptionalType() && (source.getWrappedType().isMutable() || !target.getWrappedType().isMutable()) &&
                isImplicitlyConvertible(nullptr, source.getWrappedType(), target.getWrappedType(), nullptr)) {
                return true;
            }
            break;
    }

    if (expr) {
        if (expr->getType().isEnumType() && llvm::cast<EnumDecl>(expr->getType().getDecl())->getTagType() == target) {
            return true;
        }

        // Auto-cast integer constants to parameter type if within range, error out if not within range.
        if ((expr->getType().isInteger() || expr->getType().isChar() || expr->getType().isEnumType()) && expr->isConstant()) {
            const auto& value = expr->getConstantIntegerValue();

            if (target.isInteger()) {
                return checkRange(*expr, value, target, convertedType);
            }

            if (target.isFloatingPoint()) {
                // TODO: Check that the integer value is losslessly convertible to the target type?
                if (convertedType) *convertedType = target;
                return true;
            }
        } else if (expr->getType().isFloatingPoint() && expr->isConstant() && target.isFloatingPoint()) {
            // TODO: Check that the floating-point value is losslessly convertible to the target type?
            if (convertedType) *convertedType = target;
            return true;
        } else if (expr->isNullLiteralExpr() && target.isOptionalType()) {
            if (convertedType) *convertedType = target;
            return true;
        } else if (expr->isUndefinedLiteralExpr()) {
            if (convertedType) *convertedType = target;
            return true;
        } else if (expr->isStringLiteralExpr() && target.removeOptional().isPointerType() &&
                   target.removeOptional().getPointee().isChar() && !target.removeOptional().getPointee().isMutable()) {
            // Special case: allow passing string literals as C-strings (const char*).
            if (convertedType) *convertedType = target;
            return true;
        } else if (expr->isArrayLiteralExpr() && target.isArrayType()) {
            bool isConvertible = llvm::all_of(llvm::cast<ArrayLiteralExpr>(expr)->getElements(), [&](auto& element) {
                return isImplicitlyConvertible(element, source.getElementType(), target.getElementType(), nullptr);
            });

            if (isConvertible) {
                for (auto& element : llvm::cast<ArrayLiteralExpr>(expr)->getElements()) {
                    element->setType(target.getElementType());
                }
                if (convertedType) *convertedType = target;
                return true;
            }
        }
    }

    if ((allowPointerToTemporary || (expr && expr->isLvalue())) && target.removeOptional().isPointerType() &&
        (source.isMutable() || !target.removeOptional().getPointee().isMutable()) &&
        isImplicitlyConvertible(expr, source, target.removeOptional().getPointee(), nullptr)) {
        if (convertedType) *convertedType = source;
        return true;
    } else if (source.isPointerType() && source.getPointee() == target) {
        // Auto-dereference.
        if (convertedType) *convertedType = target;
        return true;
    } else if (target.isOptionalType() && (!expr || !expr->isNullLiteralExpr()) &&
               isImplicitlyConvertible(expr, source, target.getWrappedType(), nullptr)) {
        return true;
    } else if (source.isArrayType() && target.removeOptional().isPointerType() && target.removeOptional().getPointee().isArrayType() &&
               isImplicitlyConvertible(nullptr, source.getElementType(), target.removeOptional().getPointee().getElementType(), nullptr)) {
        if (convertedType) *convertedType = source;
        return true;
    } else if (source.isArrayType() && target.removeOptional().isPointerType() &&
               isImplicitlyConvertible(nullptr, source.getElementType(), target.removeOptional().getPointee(), nullptr)) {
        if (convertedType) *convertedType = source;
        return true;
    } else if (source.isPointerType() && source.getPointee().isArrayType() && target.removeOptional().isPointerType() &&
               isImplicitlyConvertible(nullptr, source.getPointee().getElementType(), target.removeOptional().getPointee(), nullptr)) {
        if (convertedType) *convertedType = source;
        return true;
    } else if (source.isPointerType() && target.removeOptional().isPointerType() && target.removeOptional().getPointee().isArrayWithUnknownSize()) {
        return true;
    } else if (source.isTupleType() && target.isTupleType()) {
        auto* tupleExpr = llvm::dyn_cast_or_null<TupleExpr>(expr);
        auto sourceElements = source.getTupleElements();
        auto targetElements = target.getTupleElements();

        for (size_t i = 0; i < sourceElements.size(); ++i) {
            if (sourceElements[i].name != targetElements[i].name) return false;
            auto* elementValue = tupleExpr ? tupleExpr->getElements()[i].getValue() : nullptr;

            if (!isImplicitlyConvertible(elementValue, sourceElements[i].type, targetElements[i].type, nullptr)) {
                return false;
            }
        }

        if (convertedType) *convertedType = target;
        return true;
    }

    return false;
}

static bool containsGenericParam(Type type, llvm::StringRef genericParam) {
    switch (type.getKind()) {
        case TypeKind::BasicType:
            for (Type genericArg : type.getGenericArgs()) {
                if (containsGenericParam(genericArg, genericParam)) {
                    return true;
                }
            }
            return type.getName() == genericParam;

        case TypeKind::ArrayType:
            return containsGenericParam(type.getElementType(), genericParam);

        case TypeKind::TupleType:
            llvm_unreachable("unimplemented");

        case TypeKind::FunctionType:
            for (Type paramType : type.getParamTypes()) {
                if (containsGenericParam(paramType, genericParam)) {
                    return true;
                }
            }
            return containsGenericParam(type.getReturnType(), genericParam);

        case TypeKind::PointerType:
            return containsGenericParam(type.getPointee(), genericParam);

        case TypeKind::OptionalType:
            return containsGenericParam(type.getWrappedType(), genericParam);
    }

    llvm_unreachable("all cases handled");
}

static Type findGenericArg(Type argType, Type paramType, llvm::StringRef genericParam) {
    if (paramType.isBasicType() && paramType.getName() == genericParam) {
        return argType;
    }

    switch (argType.getKind()) {
        case TypeKind::BasicType:
            if (!argType.getGenericArgs().empty() && paramType.isBasicType() && paramType.getName() == argType.getName()) {
                ASSERT(argType.getGenericArgs().size() == paramType.getGenericArgs().size());
                for (auto&& [argTypeGenericArg, paramTypeGenericArg] : llvm::zip_first(argType.getGenericArgs(), paramType.getGenericArgs())) {
                    if (Type type = findGenericArg(argTypeGenericArg, paramTypeGenericArg, genericParam)) {
                        return type;
                    }
                }
            }
            break;

        case TypeKind::ArrayType:
            if (paramType.isArrayType()) {
                return findGenericArg(argType.getElementType(), paramType.getElementType(), genericParam);
            }
            break;

        case TypeKind::TupleType:
            llvm_unreachable("unimplemented");

        case TypeKind::FunctionType:
            if (paramType.isFunctionType()) {
                for (auto&& [argTypeParamType, paramTypeParamTypes] : llvm::zip_first(argType.getParamTypes(), paramType.getParamTypes())) {
                    if (Type type = findGenericArg(argTypeParamType, paramTypeParamTypes, genericParam)) {
                        return type;
                    }
                }
                return findGenericArg(argType.getReturnType(), paramType.getReturnType(), genericParam);
            }
            break;

        case TypeKind::PointerType:
            if (paramType.isPointerType()) {
                return findGenericArg(argType.getPointee(), paramType.getPointee(), genericParam);
            }
            break;

        case TypeKind::OptionalType:
            if (paramType.isOptionalType()) {
                return findGenericArg(argType.getWrappedType(), paramType.getWrappedType(), genericParam);
            }
            break;
    }

    if (paramType.removeOptional().isPointerType()) {
        return findGenericArg(argType, paramType.removeOptional().getPointee(), genericParam);
    }

    return Type();
}

std::vector<Type> Typechecker::inferGenericArgs(llvm::ArrayRef<GenericParamDecl> genericParams, CallExpr& call,
                                                llvm::ArrayRef<ParamDecl> params, bool returnOnError) {
    if (call.getArgs().size() != params.size()) return {};

    std::vector<Type> inferredGenericArgs;

    for (auto& genericParam : genericParams) {
        Type genericArg;
        Expr* genericArgValue;

        for (auto&& [param, arg] : llvm::zip_first(params, call.getArgs())) {
            Type paramType = param.getType();

            if (containsGenericParam(paramType, genericParam.getName())) {
                // FIXME: The args will also be typechecked by validateArgs() after this function. Get rid of this duplicated typechecking.
                auto* argValue = arg.getValue();
                Type argType = typecheckExpr(*argValue);
                Type maybeGenericArg = findGenericArg(argType, paramType, genericParam.getName());
                if (!maybeGenericArg) continue;

                if (!genericArg) {
                    genericArg = maybeGenericArg;
                    genericArgValue = argValue;
                } else {
                    Type paramTypeWithGenericArg = paramType.resolve({ { genericParam.getName(), genericArg } });
                    Type paramTypeWithMaybeGenericArg = paramType.resolve({ { genericParam.getName(), maybeGenericArg } });

                    if (convert(argValue, paramTypeWithGenericArg, true)) {
                        continue;
                    } else if (convert(genericArgValue, paramTypeWithMaybeGenericArg, true)) {
                        genericArg = maybeGenericArg;
                        genericArgValue = argValue;
                    } else {
                        ERROR(call.getLocation(), "couldn't infer generic parameter '" << genericParam.getName() << "' of '"
                                                                                       << call.getFunctionName() << "' because of conflicting argument types '"
                                                                                       << genericArg << "' and '" << maybeGenericArg << "'");
                    }
                }
            }
        }

        if (genericArg) {
            inferredGenericArgs.push_back(genericArg);
        } else {
            return {};
        }
    }

    ASSERT(genericParams.size() == inferredGenericArgs.size());

    for (auto&& [genericParam, genericArg] : llvm::zip(genericParams, inferredGenericArgs)) {
        if (!genericParam.getConstraints().empty()) {
            ASSERT(genericParam.getConstraints().size() == 1, "cannot have multiple generic constraints yet");

            auto* interface = getTypeDecl(*llvm::cast<BasicType>(genericParam.getConstraints()[0].getBase()));
            std::string errorReason;

            if (genericArg.isBasicType()) {
                auto* typeDecl = getTypeDecl(*llvm::cast<BasicType>(genericArg.getBase()));

                if (!typeDecl || !typeDecl->hasInterface(*interface)) {
                    if (returnOnError) {
                        return {};
                    } else {
                        ERROR(call.getLocation(), "type '" << genericArg << "' doesn't implement interface '" << interface->getName() << "'");
                    }
                }
            }
        }
    }

    return inferredGenericArgs;
}

void delta::validateGenericArgCount(size_t genericParamCount, llvm::ArrayRef<Type> genericArgs, llvm::StringRef name, SourceLocation location) {
    if (genericArgs.size() < genericParamCount) {
        ERROR(location, "too few generic arguments to '" << name << "', expected " << genericParamCount);
    } else if (genericArgs.size() > genericParamCount) {
        ERROR(location, "too many generic arguments to '" << name << "', expected " << genericParamCount);
    }
}

static void validateArgCount(size_t paramCount, size_t argCount, bool isVariadic, llvm::StringRef name, SourceLocation location) {
    if (argCount < paramCount) {
        ERROR(location, "too few arguments to '" << name << "', expected " << (isVariadic ? "at least " : "") << paramCount);
    } else if (!isVariadic && argCount > paramCount) {
        ERROR(location, "too many arguments to '" << name << "', expected " << paramCount);
    }
}

llvm::StringMap<Type> Typechecker::getGenericArgsForCall(llvm::ArrayRef<GenericParamDecl> genericParams, CallExpr& call,
                                                         llvm::ArrayRef<ParamDecl> params, bool returnOnError) {
    ASSERT(!genericParams.empty());
    std::vector<Type> inferredGenericArgs;
    llvm::ArrayRef<Type> genericArgTypes;

    if (call.getGenericArgs().empty()) {
        if (call.getArgs().empty()) {
            ERROR(call.getLocation(), "can't infer generic parameters without function arguments");
        }

        inferredGenericArgs = inferGenericArgs(genericParams, call, params, returnOnError);
        if (inferredGenericArgs.empty()) return {};
        ASSERT(inferredGenericArgs.size() == genericParams.size());
        genericArgTypes = inferredGenericArgs;
    } else {
        genericArgTypes = call.getGenericArgs();
    }

    llvm::StringMap<Type> genericArgs;
    auto genericArg = genericArgTypes.begin();

    for (const GenericParamDecl& genericParam : genericParams) {
        genericArgs.try_emplace(genericParam.getName(), *genericArg++);
    }

    return genericArgs;
}

Type Typechecker::typecheckBuiltinConversion(CallExpr& expr) {
    if (expr.getArgs().size() != 1) {
        ERROR(expr.getLocation(), "expected single argument to converting initializer");
    }
    if (!expr.getGenericArgs().empty()) {
        ERROR(expr.getLocation(), "expected no generic arguments to converting initializer");
    }
    if (!expr.getArgs().front().getName().empty()) {
        ERROR(expr.getLocation(), "expected unnamed argument to converting initializer");
    }

    auto sourceType = typecheckExpr(*expr.getArgs().front().getValue());
    auto targetType = BasicType::get(expr.getFunctionName(), {});

    if (sourceType == targetType) {
        WARN(expr.getCallee().getLocation(), "unnecessary conversion to same type");
    }

    expr.setType(targetType);
    return expr.getType();
}

static std::vector<Note> getCandidateNotes(llvm::ArrayRef<Decl*> candidates) {
    return map(candidates, [](Decl* candidate) { return Note{ candidate->getLocation(), "candidate function:" }; });
}

static std::vector<Type> getGenericArgTypes(const llvm::StringMap<Type>& genericArgs) {
    std::vector<Type> types;
    types.reserve(genericArgs.size());

    for (auto& genericArg : genericArgs) {
        types.emplace_back(genericArg.getValue());
    }

    return types;
}

Decl* Typechecker::resolveOverload(llvm::ArrayRef<Decl*> decls, CallExpr& expr, llvm::StringRef callee, bool returnNullOnError) {
    llvm::SmallVector<Decl*, 1> matches;
    std::vector<Decl*> initDecls;
    bool isInitCall = false;

    for (Decl* decl : decls) {
        switch (decl->getKind()) {
            case DeclKind::FunctionTemplate: {
                auto* functionTemplate = llvm::cast<FunctionTemplate>(decl);
                auto genericParams = functionTemplate->getGenericParams();

                if (!expr.getGenericArgs().empty() && expr.getGenericArgs().size() != genericParams.size()) {
                    if (decls.size() == 1) {
                        validateGenericArgCount(genericParams.size(), expr.getGenericArgs(), expr.getFunctionName(), expr.getLocation());
                    }
                    continue;
                }

                auto params = functionTemplate->getFunctionDecl()->getParams();
                auto genericArgs = getGenericArgsForCall(genericParams, expr, params, decls.size() != 1);
                if (genericArgs.empty()) continue; // Couldn't infer generic arguments.

                auto* functionDecl = functionTemplate->instantiate(genericArgs);

                if (decls.size() == 1 && !returnNullOnError) {
                    validateArgs(expr, *functionDecl, callee, expr.getCallee().getLocation());
                    declsToTypecheck.emplace_back(functionDecl);
                    return functionDecl;
                }
                if (argumentsMatch(expr, functionDecl)) {
                    declsToTypecheck.emplace_back(functionDecl); // TODO: Do this only after the final match has been selected.
                    matches.push_back(functionDecl);
                }
                break;
            }
            case DeclKind::FunctionDecl:
            case DeclKind::MethodDecl:
            case DeclKind::InitDecl: {
                auto& functionDecl = llvm::cast<FunctionDecl>(*decl);

                if (decls.size() == 1 && !returnNullOnError) {
                    validateGenericArgCount(0, expr.getGenericArgs(), expr.getFunctionName(), expr.getLocation());
                    validateArgs(expr, functionDecl, callee, expr.getCallee().getLocation());
                    return &functionDecl;
                }
                if (argumentsMatch(expr, &functionDecl)) {
                    matches.push_back(&functionDecl);
                }
                break;
            }
            case DeclKind::TypeDecl: {
                auto* typeDecl = llvm::cast<TypeDecl>(decl);
                isInitCall = true;
                validateGenericArgCount(0, expr.getGenericArgs(), expr.getFunctionName(), expr.getLocation());
                auto qualifiedName = getQualifiedFunctionName(typeDecl->getType(), "init", {});
                initDecls = findDecls(qualifiedName);
                ASSERT(!initDecls.empty());
                ASSERT(decls.size() == 1);
                decls = initDecls;

                for (Decl* decl : initDecls) {
                    auto& initDecl = llvm::cast<InitDecl>(*decl);

                    if (initDecls.size() == 1 && !returnNullOnError) {
                        validateArgs(expr, initDecl, callee, expr.getCallee().getLocation());
                        return &initDecl;
                    }
                    if (argumentsMatch(expr, &initDecl)) {
                        matches.push_back(&initDecl);
                    }
                }
                break;
            }
            case DeclKind::TypeTemplate: {
                auto* typeTemplate = llvm::cast<TypeTemplate>(decl);
                isInitCall = true;

                std::vector<InitDecl*> instantiatedInitDecls;

                for (auto& method : typeTemplate->getTypeDecl()->getMethods()) {
                    auto* initDecl = llvm::dyn_cast<InitDecl>(method);
                    if (!initDecl) continue;
                    initDecls.push_back(initDecl);
                }

                ASSERT(decls.size() == 1);
                decls = initDecls;

                for (auto* decl : initDecls) {
                    auto* initDecl = llvm::cast<InitDecl>(decl);
                    auto genericArgs = getGenericArgsForCall(typeTemplate->getGenericParams(), expr, initDecl->getParams(), initDecls.size() != 1);
                    if (genericArgs.empty()) continue; // Couldn't infer generic arguments.

                    TypeDecl* typeDecl = nullptr;

                    auto decls = findDecls(getQualifiedTypeName(typeTemplate->getTypeDecl()->getName(), getGenericArgTypes(genericArgs)));
                    if (decls.empty()) {
                        typeDecl = typeTemplate->instantiate(genericArgs);
                        getCurrentModule()->addToSymbolTable(*typeDecl);
                        declsToTypecheck.push_back(typeDecl); // TODO: Can these be typechecked right away?
                    } else {
                        typeDecl = llvm::cast<TypeDecl>(decls[0]);
                    }

                    for (auto& method : typeDecl->getMethods()) {
                        auto* initDecl = llvm::dyn_cast<InitDecl>(method);
                        if (!initDecl) continue;
                        instantiatedInitDecls.push_back(initDecl);
                    }
                }

                for (auto* instantiatedInitDecl : instantiatedInitDecls) {
                    if (initDecls.size() == 1 && !returnNullOnError) {
                        validateArgs(expr, *instantiatedInitDecl, callee, expr.getCallee().getLocation());
                        return instantiatedInitDecl;
                    }
                    if (argumentsMatch(expr, instantiatedInitDecl)) {
                        matches.push_back(instantiatedInitDecl);
                    }
                }
                break;
            }
            case DeclKind::VarDecl:
            case DeclKind::ParamDecl:
            case DeclKind::FieldDecl: {
                auto* variableDecl = llvm::cast<VariableDecl>(decl);

                if (auto* functionType = llvm::dyn_cast<FunctionType>(variableDecl->getType().getBase())) {
                    auto paramDecls = functionType->getParamDecls(variableDecl->getLocation());

                    if (decls.size() == 1 && !returnNullOnError) {
                        validateArgs(expr, paramDecls, false, callee, expr.getCallee().getLocation());
                        return variableDecl;
                    }
                    if (argumentsMatch(expr, nullptr, paramDecls)) {
                        matches.push_back(variableDecl);
                    }
                }
                break;
            }
            case DeclKind::DeinitDecl:
                matches.push_back(decl);
                break;

            default:
                continue;
        }
    }

    switch (matches.size()) {
        case 1:
            validateArgs(expr, *matches.front());
            return matches.front();

        case 0: {
            if (returnNullOnError) {
                return nullptr;
            }

            if (decls.size() == 0) {
                ERROR(expr.getCallee().getLocation(), "unknown identifier '" << callee << "'");
            }

            bool atLeastOneFunction = llvm::any_of(decls, [](Decl* decl) {
                return decl->isFunctionDecl() || decl->isFunctionTemplate() || decl->isTypeDecl() || decl->isTypeTemplate();
            });

            if (atLeastOneFunction) {
                auto argTypeStrings = map(expr.getArgs(), [](const NamedValue& arg) {
                    auto type = arg.getValue()->getType();
                    return type ? type.toString(true) : "???";
                });

                ERROR_WITH_NOTES(expr.getCallee().getLocation(), getCandidateNotes(decls),
                                 "no matching " << (isInitCall ? "initializer for '" : "function for call to '") << callee
                                                << "' with argument list of type '(" << llvm::join(argTypeStrings, ", ") << ")'");
            } else {
                ERROR(expr.getCallee().getLocation(), "'" << callee << "' is not a function");
            }
        }
        default:
            if (returnNullOnError) {
                return nullptr;
            }

            bool allMatchesAreFromC = llvm::all_of(matches, [](Decl* match) {
                return match->getModule() && match->getModule()->getName().endswith_lower(".h");
            });

            if (allMatchesAreFromC) {
                validateArgs(expr, *matches.front());
                return matches.front();
            }

            for (auto* match : matches) {
                if (match->getModule() && match->getModule()->getName() == "std") {
                    validateArgs(expr, *match);
                    return match;
                }
            }

            ERROR_WITH_NOTES(expr.getCallee().getLocation(), getCandidateNotes(decls),
                             "ambiguous reference to '" << callee << (isInitCall ? ".init'" : "'"));
    }
}

std::vector<Decl*> Typechecker::findCalleeCandidates(const CallExpr& expr, llvm::StringRef callee) {
    TypeDecl* receiverTypeDecl;

    if (expr.getReceiverType() && expr.getReceiverType().removePointer().isBasicType()) {
        receiverTypeDecl = getTypeDecl(*llvm::cast<BasicType>(expr.getReceiverType().removePointer().getBase()));
    } else {
        receiverTypeDecl = nullptr;
    }

    return findDecls(callee, receiverTypeDecl, isPostProcessing);
}

Type Typechecker::typecheckCallExpr(CallExpr& expr) {
    if (!expr.callsNamedFunction()) {
        ERROR(expr.getLocation(), "anonymous function calls not implemented yet");
    }

    if (Type::isBuiltinScalar(expr.getFunctionName())) {
        return typecheckBuiltinConversion(expr);
    }

    if (expr.isBuiltinCast()) {
        return typecheckBuiltinCast(expr);
    }

    for (auto& arg : expr.getArgs()) {
        typecheckExpr(*arg.getValue());
    }

    if (expr.getFunctionName() == "assert") {
        ParamDecl assertParam(Type::getBool(), "", SourceLocation());
        validateArgs(expr, assertParam, false, expr.getFunctionName(), expr.getLocation());
        validateGenericArgCount(0, expr.getGenericArgs(), expr.getFunctionName(), expr.getLocation());
        return Type::getVoid();
    }

    Decl* decl;

    if (auto* enumCase = getEnumCase(expr.getCallee())) {
        decl = enumCase;
        llvm::cast<MemberExpr>(expr.getCallee()).setDecl(*decl);
    } else if (expr.getCallee().isMemberExpr()) {
        Type receiverType = typecheckExpr(*expr.getReceiver());
        expr.setReceiverType(receiverType);

        if (receiverType.isOptionalType()) {
            WARN(expr.getReceiver()->getLocation(), "calling member function through value of optional type '"
                                                        << receiverType << "' which may be null; "
                                                        << "unwrap the value with a postfix '!' to silence this warning");
        } else if (receiverType.removePointer().isArrayType()) {
            if (expr.getFunctionName() == "size") {
                validateArgs(expr, {}, false, expr.getFunctionName(), expr.getLocation());
                validateGenericArgCount(0, expr.getGenericArgs(), expr.getFunctionName(), expr.getLocation());
                return ArrayType::getIndexType();
            }

            ERROR(expr.getReceiver()->getLocation(),
                  "type '" << receiverType.removePointer() << "' has no member function '" << expr.getFunctionName() << "'");
        } else if (receiverType.removePointer().isBuiltinType() && expr.getFunctionName() == "deinit") {
            return Type::getVoid();
        }

        if (expr.isMoveInit()) {
            if (!expr.getArgs()[0].getValue()->getType().isImplicitlyCopyable()) {
                expr.getArgs()[0].getValue()->setMoved(true);
            }
            return Type::getVoid();
        }

        auto callee = expr.getQualifiedFunctionName();
        auto decls = findCalleeCandidates(expr, callee);

        if (decls.empty() && expr.getFunctionName() == "deinit") {
            return Type::getVoid();
        }

        decl = resolveOverload(decls, expr, callee);
    } else {
        auto callee = expr.getFunctionName();
        auto decls = findCalleeCandidates(expr, callee);
        // TODO: Prevent duplicates and remove this call:
        decls.erase(std::unique(decls.begin(), decls.end()), decls.end());
        decl = resolveOverload(decls, expr, callee);

        if (auto* initDecl = llvm::dyn_cast<InitDecl>(decl)) {
            expr.setReceiverType(initDecl->getTypeDecl()->getType());
        } else if (decl->isMethodDecl()) {
            auto& varDecl = llvm::cast<VarDecl>(findDecl("this", expr.getCallee().getLocation()));
            expr.setReceiverType(varDecl.getType());
        }
    }

    checkHasAccess(*decl, expr.getCallee().getLocation(), AccessLevel::None);
    if (auto* initDecl = llvm::dyn_cast<InitDecl>(decl)) {
        checkHasAccess(*initDecl->getTypeDecl(), expr.getCallee().getLocation(), AccessLevel::None);
    }

    std::vector<ParamDecl> params;

    switch (decl->getKind()) {
        case DeclKind::FunctionDecl:
        case DeclKind::MethodDecl:
        case DeclKind::InitDecl:
        case DeclKind::DeinitDecl:
            params = llvm::cast<FunctionDecl>(decl)->getParams();
            break;

        case DeclKind::VarDecl:
        case DeclKind::ParamDecl:
        case DeclKind::FieldDecl: {
            auto type = llvm::cast<VariableDecl>(decl)->getType();
            params = llvm::cast<FunctionType>(type.getBase())->getParamDecls();
            break;
        }
        case DeclKind::EnumCase: {
            auto type = llvm::cast<EnumCase>(decl)->getAssociatedType();
            params = map(type.getTupleElements(), [&](auto& e) { return ParamDecl(e.type, std::string(e.name), decl->getLocation()); });
            validateArgs(expr, params, false, decl->getName(), expr.getLocation());
            break;
        }
        default:
            llvm_unreachable("invalid callee decl");
    }

    for (auto&& [param, arg] : llvm::zip_first(params, expr.getArgs())) {
        if (!param.getType().isImplicitlyCopyable()) {
            arg.getValue()->setMoved(true);
        }
    }

    expr.setCalleeDecl(decl);
    decl->setReferenced(true);

    switch (decl->getKind()) {
        case DeclKind::FunctionDecl:
        case DeclKind::MethodDecl:
        case DeclKind::DeinitDecl:
            return llvm::cast<FunctionDecl>(decl)->getFunctionType()->getReturnType();

        case DeclKind::InitDecl:
            return llvm::cast<InitDecl>(decl)->getTypeDecl()->getType();

        case DeclKind::VarDecl:
        case DeclKind::ParamDecl:
        case DeclKind::FieldDecl:
            return llvm::cast<FunctionType>(llvm::cast<VariableDecl>(decl)->getType().getBase())->getReturnType();

        case DeclKind::EnumCase:
            return llvm::cast<EnumCase>(decl)->getType();

        default:
            llvm_unreachable("invalid callee decl");
    }
}

bool Typechecker::argumentsMatch(const CallExpr& expr, const FunctionDecl* functionDecl, llvm::ArrayRef<ParamDecl> params) const {
    if (functionDecl) params = functionDecl->getParams();
    bool isVariadic = functionDecl && functionDecl->isVariadic();
    auto args = expr.getArgs();

    if (args.size() < params.size()) {
        return false;
    }

    if (!isVariadic && args.size() > params.size()) {
        return false;
    }

    for (size_t i = 0; i < args.size(); ++i) {
        auto& arg = args[i];
        auto* param = i < params.size() ? &params[i] : nullptr;

        if (!arg.getName().empty() && (!param || arg.getName() != param->getName())) {
            return false;
        }

        if (param && !isImplicitlyConvertible(arg.getValue(), arg.getValue()->getType(), param->getType(), nullptr, true)) {
            return false;
        }
    }

    return true;
}

void Typechecker::validateArgs(CallExpr& expr, const Decl& calleeDecl, llvm::StringRef functionName, SourceLocation location) const {
    switch (calleeDecl.getKind()) {
        case DeclKind::FunctionDecl:
        case DeclKind::MethodDecl:
        case DeclKind::InitDecl:
        case DeclKind::DeinitDecl: {
            auto& functionDecl = llvm::cast<FunctionDecl>(calleeDecl);
            validateArgs(expr, functionDecl.getParams(), functionDecl.isVariadic(), functionName, location);
            break;
        }
        case DeclKind::VarDecl:
        case DeclKind::ParamDecl:
        case DeclKind::FieldDecl: {
            auto* variableDecl = llvm::cast<VariableDecl>(&calleeDecl);
            if (auto* functionType = llvm::dyn_cast<FunctionType>(variableDecl->getType().getBase())) {
                auto paramDecls = functionType->getParamDecls(variableDecl->getLocation());
                validateArgs(expr, paramDecls, false, functionName, location);
            }
            break;
        }
        default:
            llvm_unreachable("invalid callee decl");
    }
}

void Typechecker::validateArgs(CallExpr& expr, llvm::ArrayRef<ParamDecl> params, bool isVariadic, llvm::StringRef functionName,
                               SourceLocation location) const {
    auto args = expr.getArgs();
    validateArgCount(params.size(), args.size(), isVariadic, functionName, location);

    for (size_t i = 0; i < args.size(); ++i) {
        auto& arg = args[i];
        auto* param = i < params.size() ? &params[i] : nullptr;

        if (!arg.getName().empty() && (!param || arg.getName() != param->getName())) {
            ERROR(arg.getLocation(), "invalid argument name '" << arg.getName() << "' for parameter '" << param->getName() << "'");
        }

        if (param && !convert(arg.getValue(), param->getType(), true)) {
            ERROR(arg.getLocation(), "invalid argument #" << (i + 1) << " type '" << arg.getValue()->getType() << "' to '" << functionName
                                                          << "', expected '" << param->getType() << "'");
        }
    }
}

static bool isValidCast(Type sourceType, Type targetType) {
    switch (sourceType.getKind()) {
        case TypeKind::BasicType:
        case TypeKind::ArrayType:
        case TypeKind::TupleType:
        case TypeKind::FunctionType:
            return false;

        case TypeKind::PointerType: {
            Type sourcePointee = sourceType.getPointee();

            if (targetType.isPointerType()) {
                Type targetPointee = targetType.getPointee();

                if (sourcePointee.isVoid() && (!targetPointee.isMutable() || sourcePointee.isMutable())) {
                    return true;
                }

                if (targetPointee.isVoid() && (!targetPointee.isMutable() || sourcePointee.isMutable())) {
                    return true;
                }

                if ((targetPointee.isArrayWithConstantSize() || targetPointee.isArrayWithUnknownSize()) &&
                    sourcePointee == targetPointee.getElementType()) {
                    return true;
                }
            }

            return false;
        }
        case TypeKind::OptionalType: {
            Type sourceWrappedType = sourceType.getWrappedType();

            if (sourceWrappedType.isPointerType() && targetType.isOptionalType()) {
                Type targetWrappedType = targetType.getWrappedType();

                if (targetWrappedType.isPointerType() && isValidCast(sourceWrappedType, targetWrappedType)) {
                    return true;
                }
            }

            return false;
        }
    }

    llvm_unreachable("all cases handled");
}

Type Typechecker::typecheckBuiltinCast(CallExpr& expr) {
    validateGenericArgCount(1, expr.getGenericArgs(), expr.getFunctionName(), expr.getLocation());
    validateArgCount(1, expr.getArgs().size(), false, expr.getFunctionName(), expr.getLocation());

    Type sourceType = typecheckExpr(*expr.getArgs().front().getValue());
    Type targetType = expr.getGenericArgs().front();

    if (isValidCast(sourceType, targetType)) {
        return targetType;
    }

    ERROR(expr.getCallee().getLocation(), "illegal cast from '" << sourceType << "' to '" << targetType << "'");
}

Type Typechecker::typecheckSizeofExpr(SizeofExpr&) {
    return Type::getUInt64();
}

Type Typechecker::typecheckAddressofExpr(AddressofExpr& expr) {
    if (!typecheckExpr(expr.getOperand()).removeOptional().isPointerType()) {
        ERROR(expr.getLocation(), "operand to 'addressof' must have pointer type");
    }
    return Type::getUIntPtr();
}

Type Typechecker::typecheckMemberExpr(MemberExpr& expr) {
    if (auto* enumCase = getEnumCase(expr)) {
        checkHasAccess(*enumCase->getEnumDecl(), expr.getBaseExpr()->getLocation(), AccessLevel::None);
        expr.setDecl(*enumCase);
        return enumCase->getType();
    }

    Type baseType = typecheckExpr(*expr.getBaseExpr());

    if (baseType.isOptionalType()) {
        WARN(expr.getBaseExpr()->getLocation(), "accessing member through value of optional type '"
                                                    << baseType << "' which may be null; "
                                                    << "unwrap the value with a postfix '!' to silence this warning");
        baseType = baseType.getWrappedType();
    }

    if (baseType.isPointerType()) {
        baseType = baseType.getPointee();
    }

    if (baseType.isArrayType()) {
        auto sizeSynonyms = { "count", "length", "size" };

        if (llvm::is_contained(sizeSynonyms, expr.getMemberName())) {
            ERROR(expr.getLocation(), "use the '.size()' member function to get the number of elements in an array");
        }
    }

    if (auto* typeDecl = baseType.getDecl()) {
        for (auto& field : typeDecl->getFields()) {
            if (field.getName() == expr.getMemberName()) {
                checkHasAccess(field, expr.getLocation(), AccessLevel::None);
                expr.setDecl(field);
                return field.getType().withMutability(baseType.getMutability());
            }
        }
    }

    if (baseType.isTupleType()) {
        for (auto& element : baseType.getTupleElements()) {
            if (element.name == expr.getMemberName()) {
                return element.type;
            }
        }
    }

    ERROR(expr.getLocation(), "no member named '" << expr.getMemberName() << "' in '" << baseType << "'");
}

Type Typechecker::typecheckSubscriptExpr(SubscriptExpr& expr) {
    Type lhsType = typecheckExpr(*expr.getBaseExpr());
    Type arrayType;

    if (lhsType.isArrayType()) {
        arrayType = lhsType;
    } else if (lhsType.isPointerType() && lhsType.getPointee().isArrayType()) {
        arrayType = lhsType.getPointee();
    } else if (lhsType.removePointer().isBuiltinType()) {
        ERROR(expr.getLocation(), "'" << lhsType << "' doesn't provide a subscript operator");
    } else {
        return typecheckCallExpr(expr);
    }

    Type indexType = typecheckExpr(*expr.getIndexExpr());

    if (!convert(expr.getIndexExpr(), ArrayType::getIndexType())) {
        ERROR(expr.getIndexExpr()->getLocation(), "illegal subscript index type '" << indexType << "', expected '" << ArrayType::getIndexType() << "'");
    }

    if (arrayType.isArrayWithConstantSize()) {
        if (expr.getIndexExpr()->isConstant()) {
            auto index = expr.getIndexExpr()->getConstantIntegerValue();

            if (index < 0 || index >= arrayType.getArraySize()) {
                ERROR(expr.getIndexExpr()->getLocation(),
                      "accessing array out-of-bounds with index " << index << ", array size is " << arrayType.getArraySize());
            }
        }
    }

    return arrayType.getElementType();
}

Type Typechecker::typecheckUnwrapExpr(UnwrapExpr& expr) {
    Type type = typecheckExpr(expr.getOperand());
    if (!type.isOptionalType()) {
        ERROR(expr.getLocation(), "cannot unwrap non-optional type '" << type << "'");
    }
    return type.getWrappedType();
}

Type Typechecker::typecheckLambdaExpr(LambdaExpr& expr) {
    typecheckFunctionDecl(*expr.getFunctionDecl());
    return Type(expr.getFunctionDecl()->getFunctionType(), Mutability::Mutable, expr.getLocation());
}

Type Typechecker::typecheckIfExpr(IfExpr& expr) {
    auto conditionType = typecheckExpr(*expr.getCondition());

    if (!conditionType.isBool() && !conditionType.isOptionalType()) {
        ERROR(expr.getCondition()->getLocation(), "if-expression condition must have type 'bool' or optional type");
    }

    auto thenType = typecheckExpr(*expr.getThenExpr());
    auto elseType = typecheckExpr(*expr.getElseExpr());

    if (convert(expr.getThenExpr(), elseType)) {
        return elseType;
    } else if (convert(expr.getElseExpr(), thenType)) {
        return thenType;
    } else {
        ERROR(expr.getLocation(), "incompatible operand types ('" << thenType << "' and '" << elseType << "')");
    }
}

Type Typechecker::typecheckExpr(Expr& expr, bool useIsWriteOnly) {
    llvm::Optional<Type> type;

    switch (expr.getKind()) {
        case ExprKind::VarExpr:
            type = typecheckVarExpr(llvm::cast<VarExpr>(expr), useIsWriteOnly);
            break;
        case ExprKind::StringLiteralExpr:
            type = typecheckStringLiteralExpr(llvm::cast<StringLiteralExpr>(expr));
            break;
        case ExprKind::CharacterLiteralExpr:
            type = typecheckCharacterLiteralExpr(llvm::cast<CharacterLiteralExpr>(expr));
            break;
        case ExprKind::IntLiteralExpr:
            type = typecheckIntLiteralExpr(llvm::cast<IntLiteralExpr>(expr));
            break;
        case ExprKind::FloatLiteralExpr:
            type = typecheckFloatLiteralExpr(llvm::cast<FloatLiteralExpr>(expr));
            break;
        case ExprKind::BoolLiteralExpr:
            type = typecheckBoolLiteralExpr(llvm::cast<BoolLiteralExpr>(expr));
            break;
        case ExprKind::NullLiteralExpr:
            type = typecheckNullLiteralExpr(llvm::cast<NullLiteralExpr>(expr));
            break;
        case ExprKind::UndefinedLiteralExpr:
            type = typecheckUndefinedLiteralExpr(llvm::cast<UndefinedLiteralExpr>(expr));
            break;
        case ExprKind::ArrayLiteralExpr:
            type = typecheckArrayLiteralExpr(llvm::cast<ArrayLiteralExpr>(expr));
            break;
        case ExprKind::TupleExpr:
            type = typecheckTupleExpr(llvm::cast<TupleExpr>(expr));
            break;
        case ExprKind::UnaryExpr:
            type = typecheckUnaryExpr(llvm::cast<UnaryExpr>(expr));
            break;
        case ExprKind::BinaryExpr:
            type = typecheckBinaryExpr(llvm::cast<BinaryExpr>(expr));
            break;
        case ExprKind::CallExpr:
            type = typecheckCallExpr(llvm::cast<CallExpr>(expr));
            break;
        case ExprKind::SizeofExpr:
            type = typecheckSizeofExpr(llvm::cast<SizeofExpr>(expr));
            break;
        case ExprKind::AddressofExpr:
            type = typecheckAddressofExpr(llvm::cast<AddressofExpr>(expr));
            break;
        case ExprKind::MemberExpr:
            type = typecheckMemberExpr(llvm::cast<MemberExpr>(expr));
            break;
        case ExprKind::SubscriptExpr:
            type = typecheckSubscriptExpr(llvm::cast<SubscriptExpr>(expr));
            break;
        case ExprKind::UnwrapExpr:
            type = typecheckUnwrapExpr(llvm::cast<UnwrapExpr>(expr));
            break;
        case ExprKind::LambdaExpr:
            type = typecheckLambdaExpr(llvm::cast<LambdaExpr>(expr));
            break;
        case ExprKind::IfExpr:
            type = typecheckIfExpr(llvm::cast<IfExpr>(expr));
            break;
    }

    if (type->isOptionalType() && isGuaranteedNonNull(expr)) {
        expr.setType(type->removeOptional());
    } else {
        expr.setType(*type);
    }
    expr.setAssignableType(*type);
    return expr.getType();
}

EnumCase* Typechecker::getEnumCase(const Expr& expr) {
    if (auto* memberExpr = llvm::dyn_cast<MemberExpr>(&expr)) {
        if (auto* varExpr = llvm::dyn_cast<VarExpr>(memberExpr->getBaseExpr())) {
            auto decls = findDecls(varExpr->getIdentifier());
            if (decls.size() == 1) {
                if (auto* enumDecl = llvm::dyn_cast<EnumDecl>(decls.front())) {
                    auto* enumCase = enumDecl->getCaseByName(memberExpr->getMemberName());
                    if (!enumCase) {
                        ERROR(expr.getLocation(), "enum '" << enumDecl->getName() << "' has no case named '" << memberExpr->getMemberName() << "'");
                    }
                    return enumCase;
                }
            }
        }
    }

    return nullptr;
}
