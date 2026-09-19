#include "typecheck.h"
#include <algorithm>
#include <limits>
#include <string>
#include <tuple>
#include <vector>
#pragma warning(push, 0)
#include <llvm/ADT/APSInt.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/ADT/StringExtras.h>
#include <llvm/Support/ErrorHandling.h>
#pragma warning(pop)
#include "../ast/arena.h"
#include "../ast/decl.h"
#include "../ast/expr.h"
#include "../ast/module.h"
#include "../ast/type.h"

using namespace cx;

void Typechecker::checkHasAccess(const Decl& decl, Location location, AccessLevel userAccessLevel) {
    // FIXME: Compare SourceFile objects instead of file path strings.
    if (decl.accessLevel == AccessLevel::Private && strcmp(decl.getLocation().file, location.file) != 0) {
        WARN(location, "'" << decl.getName() << "' is private");
    } else if (userAccessLevel != AccessLevel::None && decl.accessLevel < userAccessLevel) {
        WARN(location, "using " << decl.accessLevel << " type '" << decl.getName() << "' in " << userAccessLevel << " declaration");
    }
}

void Typechecker::maybeCaptureVariable(VariableDecl& variableDecl) {
    if (!currentFunction || !currentFunction->isLambda()) return;
    if (variableDecl.kind != DeclKind::VarDecl && variableDecl.kind != DeclKind::ParamDecl) return;
    auto* parent = variableDecl.parent;
    if (!parent || !parent->isFunctionDecl() || parent == currentFunction) return;
    if (!llvm::is_contained(currentFunction->captures, &variableDecl)) {
        currentFunction->captures.push_back(&variableDecl);
    }
}

static VariableDecl* getNarrowableDecl(const VarExpr& varExpr) {
    auto* decl = varExpr.decl;
    if (!decl || (decl->kind != DeclKind::VarDecl && decl->kind != DeclKind::ParamDecl)) return nullptr;
    auto* varDecl = llvm::cast<VariableDecl>(decl);
    // Globals can be reassigned by any call, so narrowing them without a runtime check is unsound.
    if (varDecl->isGlobal()) return nullptr;
    if (!varDecl->type || !varDecl->type.isOptionalType()) return nullptr;
    return varDecl;
}

void Typechecker::applyNarrowings(const Expr& condition, bool polarity) {
    switch (condition.kind) {
    case ExprKind::VarExpr: {
        auto* varDecl = getNarrowableDecl(llvm::cast<VarExpr>(condition));
        if (!varDecl) return;
        if (polarity) {
            narrowedTypes[varDecl] = varDecl->type.getWrappedType();
        } else {
            narrowedTypes.erase(varDecl);
        }
        return;
    }
    case ExprKind::BinaryExpr: {
        auto& binary = llvm::cast<BinaryExpr>(condition);
        if (binary.op == Token::AndAnd && polarity) {
            applyNarrowings(binary.getLHS(), true);
            applyNarrowings(binary.getRHS(), true);
            return;
        }
        if (binary.op == Token::OrOr && !polarity) {
            applyNarrowings(binary.getLHS(), false);
            applyNarrowings(binary.getRHS(), false);
            return;
        }
        if (binary.op != Token::Equal && binary.op != Token::NotEqual) return;
        // Overload resolution may have wrapped the operands in implicit casts (e.g. auto-reference).
        auto withoutCasts = [](const Expr& expr) {
            auto* current = &expr;
            while (auto* cast = llvm::dyn_cast<ImplicitCastExpr>(current))
                current = cast->operand;
            return current;
        };
        const Expr* lhs = withoutCasts(binary.getLHS());
        const Expr* rhs = withoutCasts(binary.getRHS());
        const Expr* operand = nullptr;
        if (lhs->isNullLiteralExpr() && !rhs->isNullLiteralExpr()) {
            operand = rhs;
        } else if (rhs->isNullLiteralExpr() && !lhs->isNullLiteralExpr()) {
            operand = lhs;
        } else {
            return;
        }
        auto* varExpr = llvm::dyn_cast<VarExpr>(operand);
        if (!varExpr) return;
        auto* varDecl = getNarrowableDecl(*varExpr);
        if (!varDecl) return;
        if ((binary.op == Token::NotEqual) == polarity) {
            narrowedTypes[varDecl] = varDecl->type.getWrappedType();
        } else {
            narrowedTypes.erase(varDecl);
        }
        return;
    }
    case ExprKind::UnaryExpr: {
        auto& unary = llvm::cast<UnaryExpr>(condition);
        if (unary.op == Token::Not) {
            applyNarrowings(unary.getOperand(), !polarity);
        }
        return;
    }
    default:
        return;
    }
}

void Typechecker::intersectNarrowings(const NarrowMap& other) {
    for (auto it = narrowedTypes.begin(); it != narrowedTypes.end();) {
        auto current = it++;
        if (!other.contains(current->first)) {
            narrowedTypes.erase(current);
        }
    }
}

void Typechecker::dropNarrowingsForNames(const llvm::StringSet<>& names) {
    if (names.empty() || narrowedTypes.empty()) return;
    for (auto it = narrowedTypes.begin(); it != narrowedTypes.end();) {
        auto current = it++;
        if (names.contains(current->first->getName())) {
            narrowedTypes.erase(current);
        }
    }
}

// Restores the optional type of a narrowed expression. Only used where the address
// (not the value) is consumed: `&x` denotes the whole optional.
static void unnarrow(Expr& expr) {
    if (expr.hasAssignableType() && expr.assignableType.isOptionalType() && expr.type == expr.assignableType.getWrappedType()) {
        expr.type = expr.assignableType;
    }
}

Type Typechecker::typecheckVarExpr(VarExpr& expr, bool useIsWriteOnly) {
    auto* decl = findDecl(expr.identifier, expr.location);
    checkHasAccess(*decl, expr.location, AccessLevel::None);
    decl->referenced = true;
    expr.decl = decl;

    if (auto variableDecl = llvm::dyn_cast<VariableDecl>(decl)) {
        maybeCaptureVariable(*variableDecl);
    }

    switch (decl->kind) {
    case DeclKind::VarDecl:
        if (!useIsWriteOnly) checkNotMoved(*decl, expr);
        if (!useIsWriteOnly) {
            if (auto narrowed = narrowedTypes.find(decl); narrowed != narrowedTypes.end()) return narrowed->second;
        }
        return llvm::cast<VarDecl>(decl)->type;
    case DeclKind::ParamDecl:
        if (!useIsWriteOnly) checkNotMoved(*decl, expr);
        if (!useIsWriteOnly) {
            if (auto narrowed = narrowedTypes.find(decl); narrowed != narrowedTypes.end()) return narrowed->second;
        }
        return llvm::cast<ParamDecl>(decl)->type;
    case DeclKind::FunctionDecl:
    case DeclKind::MethodDecl:
        return Type(llvm::cast<FunctionDecl>(decl)->getFunctionType(), Mutability::Mutable, Location());
    case DeclKind::GenericParamDecl:
        llvm_unreachable("cannot refer to generic parameters yet");
    case DeclKind::ConstructorDecl:
        llvm_unreachable("cannot refer to constructors yet");
    case DeclKind::DestructorDecl:
        llvm_unreachable("cannot refer to destructors yet");
    case DeclKind::FunctionTemplate:
        llvm_unreachable("cannot refer to generic functions yet");
    case DeclKind::TypeDecl:
        return llvm::cast<TypeDecl>(decl)->getType();
    case DeclKind::TypeTemplate:
        llvm_unreachable("cannot refer to generic types yet");
    case DeclKind::EnumDecl:
        ERROR(expr.location, "'" << expr.identifier << "' is not a variable");
    case DeclKind::EnumCase:
        return llvm::cast<EnumCase>(decl)->type;
    case DeclKind::FieldDecl: {
        if (currentFunction && currentFunction->isLambda()) {
            maybeCaptureVariable(*llvm::cast<VariableDecl>(findDecl("this", expr.location)));
        }
        return llvm::cast<FieldDecl>(decl)->type;
    }
    case DeclKind::ImportDecl:
        llvm_unreachable("import statement validation not implemented yet");
    }
    llvm_unreachable("all cases handled");
}

static Type typecheckStringLiteralExpr(StringLiteralExpr&) {
    return BasicType::get("string", {});
}

static Type typecheckCharacterLiteralExpr(CharacterLiteralExpr&) {
    return Type::getChar();
}

static Type typecheckIntLiteralExpr(IntLiteralExpr& expr) {
    if (expr.value.isSignedIntN(32)) {
        return Type::getInt();
    } else if (expr.value.isSignedIntN(64)) {
        return Type::getInt64();
    } else if (expr.value.isIntN(64)) {
        return Type::getUInt64();
    }
    ERROR(expr.location, "integer literal is too large");
}

static Type typecheckFloatLiteralExpr(FloatLiteralExpr&) {
    return Type::getFloat();
}

static Type typecheckBoolLiteralExpr(BoolLiteralExpr&) {
    return Type::getBool();
}

Type Typechecker::typecheckNullLiteralExpr(NullLiteralExpr&, Type expectedType) {
    if (expectedType && expectedType.isOptionalType() && !expectedType.containsUnresolvedPlaceholder()) {
        return expectedType;
    } else {
        return Type::getNull();
    }
}

static Type typecheckUndefinedLiteralExpr(UndefinedLiteralExpr&, Type expectedType) {
    ASSERT(expectedType && !expectedType.containsUnresolvedPlaceholder());
    return expectedType;
}

Type Typechecker::typecheckArrayLiteralExpr(ArrayLiteralExpr& array, Type expectedType) {
    if (array.elements.empty()) {
        if (expectedType && !expectedType.containsUnresolvedPlaceholder()) {
            return expectedType;
        } else {
            ERROR(array.location, "couldn't infer type of empty array literal");
        }
    }

    Type firstType = typecheckExpr(*array.elements[0]);

    for (auto& element : llvm::ArrayRef<Expr*>(array.elements).drop_front()) {
        Type type = typecheckExpr(*element);
        if (type != firstType) {
            ERROR(element->location, "mixed element types in array literal (expected '" << firstType << "', found '" << type << "')");
        }
    }

    return ArrayType::get(firstType, int64_t(array.elements.size()));
}

Type Typechecker::typecheckTupleExpr(TupleExpr& expr) {
    auto elements = map(expr.elements, [&](const NamedValue& namedValue) { return TupleElement{namedValue.name, typecheckExpr(*namedValue.value)}; });
    return TupleType::get(std::move(elements));
}

void Typechecker::typecheckImplicitlyBoolConvertibleExpr(Type type, Location location, bool positive) {
    if (!type.removePointer().isBool() && !type.removePointer().isOptionalType()) {
        if (type.isImplementedAsPointer()) {
            WARN(location, "type '" << type << "' " << (positive ? "is always non-null" : "cannot be null") << "; to declare it nullable, use '"
                                    << OptionalType::get(type) << "'");
        } else {
            ERROR(location, "type '" << type << "' is not convertible to boolean");
        }
    }
}

Type Typechecker::typecheckUnaryExpr(UnaryExpr& expr) {
    Type operandType = typecheckExpr(expr.getOperand());

    switch (expr.op) {
    case Token::Not:
        typecheckImplicitlyBoolConvertibleExpr(operandType, expr.getOperand().location, false);
        return Type::getBool();

    case Token::Star: // Dereference operation
        if (operandType.removeOptional().isPointerType()) {
            return operandType.removeOptional().getPointee();
        } else if (operandType.removeOptional().isUnsizedArrayPointer()) {
            return operandType.removeOptional().getElementType();
        }

        ERROR(expr.location, "cannot dereference non-pointer type '" << operandType << "'");

    case Token::And: // Address-of operation
        unnarrow(expr.getOperand());
        operandType = expr.getOperand().type;
        // Allow forming mutable pointers to constants. This is safe because constants will be inlined at the usage site.
        if (expr.isConstant()) {
            operandType = operandType.withMutability(Mutability::Mutable);
        }
        return PointerType::get(operandType);

    case Token::Increment:
        operandType = operandType.removePointer();

        if (!operandType.isMutable()) {
            ERROR(expr.location, "cannot increment immutable value of type '" << operandType << "'");
        } else if (!operandType.isIncrementable()) {
            ERROR(expr.location, "cannot increment '" << operandType << "'");
        }

        return Type::getVoid();

    case Token::Decrement:
        operandType = operandType.removePointer();

        if (!operandType.isMutable()) {
            ERROR(expr.location, "cannot decrement immutable value of type '" << operandType << "'");
        } else if (!operandType.isDecrementable()) {
            ERROR(expr.location, "cannot decrement '" << operandType << "'");
        }

        return Type::getVoid();

    default:
        return operandType;
    }
}

static void throwInvalidOperandsToBinaryExpr(const BinaryExpr& expr, Token::Kind op) {
    std::string hint;

    if ((expr.getRHS().isNullLiteralExpr() || expr.getLHS().isNullLiteralExpr()) && (op == Token::Equal || op == Token::NotEqual)) {
        hint += " (non-optional type '";
        if (expr.getRHS().isNullLiteralExpr()) {
            hint += expr.getLHS().type.toString();
        } else {
            hint += expr.getRHS().type.toString();
        }
        hint += "' cannot be null)";
    } else {
        hint = "";
    }

    ERROR(expr.location, "invalid operands '" << expr.getLHS().type << "' and '" << expr.getRHS().type << "' to '" << toString(op) << "'" << hint);
}

static bool allowAssignmentOfUndefined(const Expr& lhs, const FunctionDecl* currentFunction) {
    if (auto* constructorDecl = llvm::dyn_cast<ConstructorDecl>(currentFunction)) {
        switch (lhs.kind) {
        case ExprKind::VarExpr: {
            auto* fieldDecl = llvm::dyn_cast<FieldDecl>(llvm::cast<VarExpr>(lhs).decl);
            return fieldDecl && fieldDecl->getParentDecl() == constructorDecl->getTypeDecl();
        }
        case ExprKind::MemberExpr: {
            auto* varExpr = llvm::dyn_cast<VarExpr>(llvm::cast<MemberExpr>(lhs).base);
            return varExpr && varExpr->identifier == "this";
        }
        default:
            return false;
        }
    }
    return false;
}

Type Typechecker::typecheckBinaryExpr(BinaryExpr& expr) {
    auto op = expr.op;

    if (op == Token::Assignment) {
        typecheckAssignment(expr, expr.location);
        return Type::getVoid();
    }

    if (isCompoundAssignmentOperator(op)) {
        auto rhs = makeAST<BinaryExpr>(withoutCompoundEqSuffix(op), &expr.getLHS(), &expr.getRHS(), expr.location);
        expr = BinaryExpr(Token::Assignment, &expr.getLHS(), rhs, expr.location);
        return typecheckBinaryExpr(expr);
    }

    if (op == Token::AndAnd || op == Token::OrOr) {
        Type leftType = typecheckExpr(expr.getLHS());
        auto outerNarrowings = narrowedTypes;
        applyNarrowings(expr.getLHS(), op == Token::AndAnd);
        Type rightType = typecheckExpr(expr.getRHS(), false, leftType);
        // The right side may not execute (short-circuit), so only narrowings valid on both paths survive.
        intersectNarrowings(outerNarrowings);
        if (!isBuiltinOp(op, leftType, rightType)) {
            return typecheckCallExpr(expr);
        }
        if (leftType.isBool() && rightType.isBool()) {
            return Type::getBool();
        }
        throwInvalidOperandsToBinaryExpr(expr, op);
    }

    Type leftType = typecheckExpr(expr.getLHS());
    Type rightType = typecheckExpr(expr.getRHS(), false, leftType);

    if (op == Token::Equal || op == Token::NotEqual) {
        // Null checks are builtin for all optionals so they don't depend on stdlib comparison operators.
        if (expr.getLHS().isNullLiteralExpr() && rightType.isOptionalType()) {
            expr.setLHS(NOTNULL(convert(&expr.getLHS(), rightType)));
            return Type::getBool();
        }
        if (expr.getRHS().isNullLiteralExpr() && leftType.isOptionalType()) {
            expr.setRHS(NOTNULL(convert(&expr.getRHS(), leftType)));
            return Type::getBool();
        }
    }

    if ((op == Token::Equal || op == Token::NotEqual) && leftType.isTupleType() && rightType.isTupleType()) {
        auto leftElements = leftType.getTupleElements();
        auto rightElements = rightType.getTupleElements();
        auto hasDistinctNames = [](llvm::ArrayRef<TupleElement> elements) {
            for (size_t i = 0; i < elements.size(); ++i) {
                for (size_t j = i + 1; j < elements.size(); ++j) {
                    if (elements[i].name == elements[j].name) return false;
                }
            }
            return true;
        };
        // Only comparable nominally: same arity, same distinct non-empty element names. The lowering below accesses elements by name.
        // FIXME: operands with side effects are evaluated once per element; bind them to temporaries.
        auto namesMatch = leftElements.size() == rightElements.size() && !leftElements.empty() && hasDistinctNames(leftElements)
                       && hasDistinctNames(rightElements) && llvm::all_of(llvm::zip_first(leftElements, rightElements), [](auto&& pair) {
                              auto&& [left, right] = pair;
                              return !left.name.empty() && left.name == right.name;
                          });

        if (namesMatch) {
            // Lower tuple comparison to elementwise comparison (e.g. `(a == b) && (c == d)`).
            auto combiner = op == Token::Equal ? Token::AndAnd : Token::OrOr;
            Expr* result = nullptr;
            for (size_t i = 0; i < leftElements.size(); ++i) {
                auto* comparison = makeAST<BinaryExpr>(op, makeAST<MemberExpr>(&expr.getLHS(), std::string(leftElements[i].name), expr.location),
                                                       makeAST<MemberExpr>(&expr.getRHS(), std::string(rightElements[i].name), expr.location), expr.location);
                result = result ? makeAST<BinaryExpr>(combiner, result, comparison, expr.location) : comparison;
            }
            ASSERT(result);
            expr = llvm::cast<BinaryExpr>(*result);
            return typecheckBinaryExpr(expr);
        }
    }

    if (!isBuiltinOp(op, leftType, rightType)) {
        return typecheckCallExpr(expr);
    }

    if (leftType.removeOptional().isPointerType() && rightType.removeOptional().isPointerType()) {
        if (!isComparisonOperator(op)) {
            throwInvalidOperandsToBinaryExpr(expr, op);
        }

        auto leftPointeeType = leftType.removeOptional().removePointer();
        auto rightPointeeType = rightType.removeOptional().removePointer();
        if (leftPointeeType.isUnsizedArrayPointer()) leftPointeeType = leftPointeeType.getElementType();
        if (rightPointeeType.isUnsizedArrayPointer()) rightPointeeType = rightPointeeType.getElementType();

        if (!leftPointeeType.equalsIgnoreTopLevelMutable(rightPointeeType)) {
            ERROR(expr.location, "comparison of distinct pointer types ('" << leftType << "' and '" << rightType << "')");
        }
    } else if (isBitwiseOperator(op) && (leftType.isFloatingPoint() || rightType.isFloatingPoint())) {
        throwInvalidOperandsToBinaryExpr(expr, op);
    } else if (leftType.isVoid() || rightType.isVoid()) {
        throwInvalidOperandsToBinaryExpr(expr, op);
    } else if (auto convertedRHS = convert(&expr.getRHS(), leftType, true)) {
        expr.setRHS(convertedRHS);
    } else if (auto convertedLHS = convert(&expr.getLHS(), rightType, true)) {
        expr.setLHS(convertedLHS);
    } else if (!leftType.removeOptional().isPointerType() || !rightType.removeOptional().isPointerType()) {
        throwInvalidOperandsToBinaryExpr(expr, op);
    }

    return isComparisonOperator(op) ? Type::getBool() : expr.getLHS().type.removeOptional().removePointer();
}

void Typechecker::typecheckAssignment(BinaryExpr& expr, Location location) {
    auto* lhs = &expr.getLHS();
    auto* rhs = &expr.getRHS();

    typecheckExpr(*lhs, true);
    Type lhsType = lhs->assignableType;
    Type rhsType = typecheckExpr(*rhs, false, lhsType);

    if (rhs->isUndefinedLiteralExpr() && !allowAssignmentOfUndefined(*lhs, currentFunction)) {
        ERROR(rhs->location, "'undefined' is only allowed as an initial value");
    }

    if (auto converted = convert(rhs, lhsType)) {
        expr.setRHS(converted);
        rhs = converted;
    } else {
        diagnoseClosureConversion(rhsType, lhsType, location);
        ERROR(location, "cannot assign '" << rhsType << "' to '" << lhsType << "'");
    }

    // Assigning a possibly-null value invalidates narrowing; assigning a non-null value preserves it.
    if (auto* varExpr = llvm::dyn_cast<VarExpr>(lhs)) {
        auto narrowed = narrowedTypes.find(varExpr->decl);
        if (narrowed != narrowedTypes.end() && (rhsType.isOptionalType() || rhsType.isNull())) {
            narrowedTypes.erase(narrowed);
        }
    }

    if (!lhsType.isMutable()) {
        switch (lhs->kind) {
        case ExprKind::VarExpr: {
            auto identifier = llvm::cast<VarExpr>(lhs)->identifier;
            ERROR(location, "cannot assign to immutable variable '" << identifier << "' of type '" << lhsType << "'");
        }
        case ExprKind::MemberExpr: {
            auto memberName = llvm::cast<MemberExpr>(lhs)->member;
            ERROR(location, "cannot assign to immutable variable '" << memberName << "' of type '" << lhsType << "'");
        }
        default:
            ERROR(location, "cannot assign to immutable expression of type '" << lhsType << "'");
        }
    }

    if (!rhsType.isImplicitlyCopyable() && !lhsType.removeOptional().isPointerType()) {
        setMoved(rhs, true);
        setMoved(lhs, false);
    }

    if (currentInitializedFields) {
        if (auto fieldDecl = lhs->getFieldDecl()) {
            currentInitializedFields->insert(fieldDecl);
        }
    }
}

static void checkRange(const Expr& expr, const llvm::APSInt& value, Type type) {
    if (llvm::APSInt::compareValues(value, llvm::APSInt::getMinValue(type.getIntegerBitWidth(), type.isUnsigned())) < 0
        || llvm::APSInt::compareValues(value, llvm::APSInt::getMaxValue(type.getIntegerBitWidth(), type.isUnsigned())) > 0) {
        ERROR(expr.location, value << " is out of range for type '" << type << "'");
    }
}

static bool hasField(TypeDecl& type, const FieldDecl& field) {
    return llvm::any_of(type.fields, [&](const FieldDecl& f) { return f.getName() == field.getName() && f.type == field.type; });
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
    auto thisTypeResolvedInterface = llvm::cast<TypeDecl>(interface.instantiate({{"This", type.getType()}}, {}));

    for (auto& fieldRequirement : thisTypeResolvedInterface->fields) {
        if (!hasField(type, fieldRequirement)) {
            if (errorReason) {
                *errorReason = ("doesn't have field '" + fieldRequirement.getName() + "'").str();
            }
            return false;
        }
    }

    for (auto& requiredMethod : thisTypeResolvedInterface->methods) {
        if (auto* functionDecl = llvm::dyn_cast<FunctionDecl>(requiredMethod)) {
            if (functionDecl->body) continue;

            if (!hasMethod(type, *functionDecl)) {
                if (errorReason) {
                    auto params = map(functionDecl->getParams(), [](const ParamDecl& param) {
                        return ((param.isPublic ? "public " : "") + param.type.toString() + " " + param.getName()).str();
                    });
                    *errorReason = ("doesn't have member function '" + functionDecl->getName() + "' (expected prototype '"
                                    + functionDecl->getReturnType().toString() + " " + functionDecl->getName() + "(" + llvm::join(params, ", ") + ")')")
                                       .str();
                }
                return false;
            }
        } else {
            ERROR(requiredMethod->getLocation(), "non-function interface member requirements are not supported yet");
        }
    }

    return true;
}

Expr* Typechecker::convert(Expr* expr, Type type, bool allowPointerToTemporary) const {
    std::optional<ImplicitCastExpr::Kind> implicitCastKind;
    if (Type convertedType = isImplicitlyConvertible(expr, expr->type, type, allowPointerToTemporary, &implicitCastKind)) {
        if (implicitCastKind) {
            auto* cast = makeAST<ImplicitCastExpr>(expr, convertedType, *implicitCastKind);
            if (*implicitCastKind == ImplicitCastExpr::AutoReference && expr->hasAssignableType() && expr->assignableType.isOptionalType()
                && !expr->assignableType.getWrappedType().isImplementedAsPointer() && expr->type == expr->assignableType.getWrappedType()) {
                // Preserve narrowing through the reference: the backend derives the payload address from the divergence.
                cast->assignableType = expr->assignableType;
            }
            return cast;
        } else if (convertedType != expr->type) {
            expr->type = convertedType;

            if (auto* ifExpr = llvm::dyn_cast<IfExpr>(expr)) {
                ifExpr->thenExpr->type = convertedType;
                ifExpr->elseExpr->type = convertedType;
            }
        }
        return expr;
    }
    return nullptr;
}

Type Typechecker::isImplicitlyConvertible(const Expr* expr, Type source, Type target, bool allowPointerToTemporary,
                                          std::optional<ImplicitCastExpr::Kind>* implicitCastKind) const {
    if (source.isBasicType() && target.isBasicType() && source.getName() == target.getName() && source.getGenericArgs() == target.getGenericArgs()) {
        return source;
    }

    if (source.isArrayType() && (target.isArrayType() || target.isArrayRef()) && source.getElementType() == target.getElementType()) {
        if (target.isArrayType() && source.getArraySize() == target.getArraySize()) return source;
        if (source.isConstantArray() && (target.isUnsizedArrayPointer() || target.isArrayRef())) return source;
    }

    if (source.isTupleType() && target.isTupleType() && source.getTupleElements() == target.getTupleElements()) {
        return source;
    }

    if (source.isFunctionType() && target.isFunctionType() && source.getReturnType() == target.getReturnType()
        && source.getParamTypes() == target.getParamTypes()) {
        return source;
    }

    if (source.isPointerType() && target.isPointerType() && (source.getPointee().isMutable() || !target.getPointee().isMutable())
        && (isImplicitlyConvertible(nullptr, source.getPointee(), target.getPointee()) || target.getPointee().isVoid())) {
        return source;
    }

    if (source.isOptionalType() && target.isOptionalType() && (source.getWrappedType().isMutable() || !target.getWrappedType().isMutable())
        && isImplicitlyConvertible(nullptr, source.getWrappedType(), target.getWrappedType())) {
        return source;
    }

    if (expr) {
        if (expr->type.isEnumType() && llvm::cast<EnumDecl>(expr->type.getDecl())->getTagType() == target) {
            return source;
        }

        if (auto* ifExpr = llvm::dyn_cast<IfExpr>(expr)) {
            if (isImplicitlyConvertible(ifExpr->thenExpr, ifExpr->thenExpr->type, target)
                && isImplicitlyConvertible(ifExpr->elseExpr, ifExpr->elseExpr->type, target)) {
                return target;
            }
        }

        // Auto-cast integer constants to target type if within range, error out if not within range.
        if ((expr->type.isInteger() || expr->type.isChar() || expr->type.isEnumType()) && expr->isConstant()) {
            auto value = expr->getConstantIntegerValue();
            auto adjustedTarget = allowPointerToTemporary ? target.removePointer() : target; // Convert e.g. int literal to uint when comparing to uint*.

            if (adjustedTarget.isInteger()) {
                checkRange(*expr, value, adjustedTarget);
                return adjustedTarget;
            }

            if (adjustedTarget.isFloatingPoint()) {
                // TODO: Check that the integer value is losslessly convertible to the target type?
                return adjustedTarget;
            }
        }

        if (expr->type.isFloatingPoint() && expr->isConstant() && target.isFloatingPoint()) {
            // TODO: Check that the floating-point value is losslessly convertible to the target type?
            return target;
        }

        if (expr->isNullLiteralExpr() && target.isOptionalType()) {
            return target;
        }

        // Special case: allow passing string literals as C-strings (const char* or const char[*]).
        if (expr->isStringLiteralExpr()
            && ((target.removeOptional().isPointerType() && target.removeOptional().getPointee().isChar() && !target.removeOptional().getPointee().isMutable())
                || (target.removeOptional().isUnsizedArrayPointer() && target.removeOptional().getElementType().isChar()
                    && !target.removeOptional().getElementType().isMutable()))) {
            return target;
        }

        if (expr->isArrayLiteralExpr() && target.isConstantArray()) {
            auto arrayLiteralExpr = llvm::cast<ArrayLiteralExpr>(expr);
            bool isConvertible = llvm::all_of(
                arrayLiteralExpr->elements, [&](Expr* element) { return isImplicitlyConvertible(element, source.getElementType(), target.getElementType()); });

            if (isConvertible) {
                for (auto& element : arrayLiteralExpr->elements) {
                    // FIXME: Don't set type here.
                    element->type = target.getElementType();
                }
                return target;
            }
        }
    }

    if ((allowPointerToTemporary || (expr && expr->isLvalue())) && target.removeOptional().isPointerType() &&
        // Allow forming mutable pointers to constants. This is safe because constants will be inlined at the usage site.
        (source.isMutable() || (expr && expr->isConstant()) || !target.removeOptional().getPointee().isMutable())
        && isImplicitlyConvertible(expr, source, target.removeOptional().getPointee())) {
        if (implicitCastKind) *implicitCastKind = ImplicitCastExpr::AutoReference;
        return source;
    }

    if (source.isPointerType() && source.getPointee() == target && expr && !expr->isReferenceExpr()) {
        if (implicitCastKind) *implicitCastKind = ImplicitCastExpr::AutoDereference;
        return target;
    }

    if (target.isOptionalType() && (!expr || !expr->isNullLiteralExpr()) && isImplicitlyConvertible(expr, source, target.getWrappedType())) {
        if (implicitCastKind) *implicitCastKind = ImplicitCastExpr::OptionalWrap;
        return target;
    }

    // Calls returning optionals implicitly unwrap like any other expression; the null
    // analyzer warns unless the unwrapped value is proven non-null at the use site.
    if (source.isOptionalType() && source.getWrappedType() == target && expr) {
        if (implicitCastKind) *implicitCastKind = ImplicitCastExpr::OptionalUnwrap;
        return target;
    }

    if (source.isArrayType() && target.removeOptional().isPointerType()
        && isImplicitlyConvertible(nullptr, source.getElementType(), target.removeOptional().getPointee())) {
        return source;
    }

    if (source.isPointerType() && source.getPointee().isConstantArray() && (target.isArrayRef() || target.isUnsizedArrayPointer())
        && source.getPointee().getElementType() == target.getElementType()) {
        return source;
    }

    if (source.isPointerType() && source.getPointee().isArrayType() && target.removeOptional().isPointerType()
        && isImplicitlyConvertible(nullptr, source.getPointee().getElementType(), target.removeOptional().getPointee())) {
        return source;
    }

    // Allow conversion from T[*]? to T* and void*
    if (source.removeOptional().isUnsizedArrayPointer() && target.isPointerType()
        && (source.removeOptional().getElementType() == target.getPointee() || target.getPointee().isVoid())) {
        return source;
    }

    if (target.isUnsizedArrayPointer() && source.isPointerType() && target.getElementType() == source.getPointee()) {
        return source;
    }

    if (source.isTupleType() && target.isTupleType()) {
        auto* tupleExpr = llvm::dyn_cast_or_null<TupleExpr>(expr);
        auto sourceElements = source.getTupleElements();
        auto targetElements = target.getTupleElements();

        for (size_t i = 0; i < sourceElements.size(); ++i) {
            if (sourceElements[i].name != targetElements[i].name) {
                return Type();
            }

            auto* elementValue = tupleExpr ? tupleExpr->elements[i].value : nullptr;

            if (!isImplicitlyConvertible(elementValue, sourceElements[i].type, targetElements[i].type)) {
                return Type();
            }
        }

        return target;
    }

    return Type();
}

bool cx::containsGenericParam(Type type, llvm::StringRef genericParam) {
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

    case TypeKind::UnresolvedType:
        llvm_unreachable("invalid unresolved type");
    }

    llvm_unreachable("all cases handled");
}

Type Typechecker::findGenericArg(Type argType, Type paramType, llvm::StringRef genericParam) {
    if (paramType.isBasicType() && paramType.getName() == genericParam) {
        return argType;
    }

    if (argType.isClosureType()) {
        // Infer from the user-visible signature; argument conversion still rejects
        // capturing lambdas where plain function pointers are expected.
        auto closureParams = argType.getClosureParamTypes();
        std::vector<Type> paramTypes(closureParams.begin(), closureParams.end());
        return findGenericArg(FunctionType::get(argType.getClosureReturnType(), std::move(paramTypes), false), paramType, genericParam);
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
        if (paramType.isTupleType()) {
            for (auto&& [argTypeElement, paramTypeElement] : llvm::zip_first(argType.getTupleElements(), paramType.getTupleElements())) {
                if (Type type = findGenericArg(argTypeElement.type, paramTypeElement.type, genericParam)) {
                    return type;
                }
            }
        }
        break;

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

    case TypeKind::UnresolvedType:
        llvm_unreachable("invalid unresolved type");
    }

    // TODO: Should probably try matching generic arg also with implicitly-converted values, instead duplicating the special cases here. This is bug prone.

    if (paramType.removeOptional().isPointerType()) {
        return findGenericArg(argType, paramType.removeOptional().getPointee(), genericParam);
    }

    if (paramType.isArrayRef() && argType.removeOptional().removePointer().isArrayType()) {
        return findGenericArg(argType.removeOptional().removePointer().getElementType(), paramType.getElementType(), genericParam);
    }

    return Type();
}

static Type replaceUnresolvedGenericParamsWithPlaceholders(Type type, llvm::ArrayRef<GenericParamDecl> genericParams) {
    llvm::StringMap<Type> placeholders;

    for (auto& genericParam : genericParams) {
        placeholders.try_emplace(genericParam.getName(), UnresolvedType::get());
    }

    return type.resolve(placeholders);
}

std::vector<Type> Typechecker::inferGenericArgsFromCallArgs(llvm::ArrayRef<GenericParamDecl> genericParams, CallExpr& call, llvm::ArrayRef<ParamDecl> params,
                                                            bool returnOnError) {
    if (call.args.size() > params.size()) return {};

    for (size_t i = call.args.size(); i < params.size(); ++i) {
        if (!params[i].defaultValue) return {};
    }

    std::vector<Type> inferredGenericArgs;

    for (auto& genericParam : genericParams) {
        Type genericArg;
        Expr* genericArgValue = nullptr;

        for (auto&& [arg, param] : llvm::zip_first(call.args, params)) {
            Type paramType = param.type;

            if (containsGenericParam(paramType, genericParam.getName())) {
                // FIXME: The args will also be typechecked by validateAndConvertArguments() after this function. Get rid of this duplicated typechecking.
                auto* argValue = arg.value;
                auto expectedType = replaceUnresolvedGenericParamsWithPlaceholders(paramType, genericParams);
                // TODO: Should probably not typecheck here because it might change the expression's type?
                Type argType = argValue->hasType() ? argValue->type : typecheckExpr(*argValue, false, expectedType);
                Type maybeGenericArg = findGenericArg(argType, paramType, genericParam.getName());
                if (!maybeGenericArg) continue;

                if (!genericArg) {
                    genericArg = maybeGenericArg;
                    genericArgValue = argValue;
                } else {
                    Type paramTypeWithGenericArg = paramType.resolve({{genericParam.getName(), genericArg}});
                    Type paramTypeWithMaybeGenericArg = paramType.resolve({{genericParam.getName(), maybeGenericArg}});

                    if (isImplicitlyConvertible(argValue, argValue->type, paramTypeWithGenericArg, true)) {
                        continue;
                    } else if (isImplicitlyConvertible(genericArgValue, genericArgValue->type, paramTypeWithMaybeGenericArg, true)) {
                        genericArg = maybeGenericArg;
                        genericArgValue = argValue;
                    } else {
                        return {}; // TODO: Return "conflict argument types" as reason for inference failure.
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
        if (!genericParam.constraints.empty()) {
            ASSERT(genericParam.constraints.size() == 1, "cannot have multiple generic constraints yet");
            auto* interface = getTypeDecl(*llvm::cast<BasicType>(genericParam.constraints[0].typeBase));

            if (auto basicType = llvm::dyn_cast<BasicType>(genericArg.typeBase)) {
                auto* typeDecl = getTypeDecl(*basicType);
                if (typeDecl && typeDecl->hasInterface(*interface)) {
                    continue;
                }
            }

            if (returnOnError) {
                return {};
            } else {
                ERROR(call.location, "type '" << genericArg << "' doesn't implement interface '" << interface->getName() << "'");
            }
        }
    }

    return inferredGenericArgs;
}

std::optional<VariadicGenericArgs> Typechecker::inferVariadicGenericArgs(llvm::ArrayRef<GenericParamDecl> genericParams, CallExpr& call,
                                                                         llvm::ArrayRef<ParamDecl> params, bool returnOnError) {
    ASSERT(!params.empty() && params.back().isPack);
    auto fixedParams = params.drop_back();
    Type packType = params.back().type;

    if (call.args.size() < fixedParams.size()) return std::nullopt;
    size_t packCount = call.args.size() - fixedParams.size();

    for (size_t i = fixedParams.size(); i < call.args.size(); ++i) {
        if (!call.args[i].name.empty()) {
            if (returnOnError) return std::nullopt;
            ERROR(call.args[i].location, "variadic arguments cannot have labels");
        }
    }

    std::vector<const GenericParamDecl*> fixedGenerics, packGenerics;
    for (auto& genericParam : genericParams) {
        bool inPack = packType && containsGenericParam(packType, genericParam.getName());
        bool inFixed = false;
        for (auto& param : fixedParams) {
            if (param.type && containsGenericParam(param.type, genericParam.getName())) {
                inFixed = true;
                break;
            }
        }
        if (inPack && inFixed) return std::nullopt;
        if (inPack) {
            packGenerics.push_back(&genericParam);
        } else if (inFixed) {
            fixedGenerics.push_back(&genericParam);
        } else {
            return std::nullopt;
        }
    }

    VariadicGenericArgs result;
    result.packArgs.resize(packCount);

    for (auto* genericParam : fixedGenerics) {
        Type genericArg;
        Expr* genericArgValue = nullptr;

        for (size_t i = 0; i < fixedParams.size(); ++i) {
            Type paramType = fixedParams[i].type;
            if (!paramType || !containsGenericParam(paramType, genericParam->getName())) continue;

            auto* argValue = call.args[i].value;
            auto expectedType = replaceUnresolvedGenericParamsWithPlaceholders(paramType, genericParams);
            Type argType = argValue->hasType() ? argValue->type : typecheckExpr(*argValue, false, expectedType);
            Type maybeGenericArg = findGenericArg(argType, paramType, genericParam->getName());
            if (!maybeGenericArg) continue;

            if (!genericArg) {
                genericArg = maybeGenericArg;
                genericArgValue = argValue;
            } else {
                Type paramTypeWithGenericArg = paramType.resolve({{genericParam->getName(), genericArg}});
                Type paramTypeWithMaybeGenericArg = paramType.resolve({{genericParam->getName(), maybeGenericArg}});

                if (isImplicitlyConvertible(argValue, argValue->type, paramTypeWithGenericArg, true)) {
                    continue;
                } else if (isImplicitlyConvertible(genericArgValue, genericArgValue->type, paramTypeWithMaybeGenericArg, true)) {
                    genericArg = maybeGenericArg;
                    genericArgValue = argValue;
                } else {
                    return std::nullopt;
                }
            }
        }

        if (!genericArg) return std::nullopt;
        result.fixedArgs[genericParam->getName()] = genericArg;
    }

    for (size_t j = 0; j < packCount; ++j) {
        auto* argValue = call.args[fixedParams.size() + j].value;
        auto expectedType = replaceUnresolvedGenericParamsWithPlaceholders(packType, genericParams);
        Type argType = argValue->hasType() ? argValue->type : typecheckExpr(*argValue, false, expectedType);

        for (auto* genericParam : packGenerics) {
            Type genericArg = findGenericArg(argType, packType, genericParam->getName());
            if (!genericArg) return std::nullopt;
            result.packArgs[j][genericParam->getName()] = genericArg;
        }
    }

    auto checkConstraint = [&](const GenericParamDecl& genericParam, Type genericArg) -> bool {
        if (genericParam.constraints.empty()) return true;
        ASSERT(genericParam.constraints.size() == 1, "cannot have multiple generic constraints yet");
        auto* interface = getTypeDecl(*llvm::cast<BasicType>(genericParam.constraints[0].typeBase));

        if (auto basicType = llvm::dyn_cast<BasicType>(genericArg.typeBase)) {
            auto* typeDecl = getTypeDecl(*basicType);
            if (typeDecl && typeDecl->hasInterface(*interface)) return true;
        }

        if (returnOnError) return false;
        ERROR(call.location, "type '" << genericArg << "' doesn't implement interface '" << interface->getName() << "'");
    };

    for (auto* genericParam : fixedGenerics) {
        if (!checkConstraint(*genericParam, result.fixedArgs[genericParam->getName()])) return std::nullopt;
    }
    for (size_t j = 0; j < packCount; ++j) {
        for (auto* genericParam : packGenerics) {
            if (!checkConstraint(*genericParam, result.packArgs[j][genericParam->getName()])) return std::nullopt;
        }
    }

    for (auto& genericParam : genericParams) {
        auto it = result.fixedArgs.find(genericParam.getName());
        if (it != result.fixedArgs.end()) result.cacheKey.push_back(it->second);
    }
    for (size_t j = 0; j < packCount; ++j) {
        for (auto& genericParam : genericParams) {
            auto it = result.packArgs[j].find(genericParam.getName());
            if (it != result.packArgs[j].end()) result.cacheKey.push_back(it->second);
        }
    }

    return result;
}

void cx::diagnoseClosureConversion(Type source, Type target, Location location) {
    if (source.isClosureType() && target.isFunctionType()) {
        ERROR(location, "cannot convert capturing lambda '" << source << "' to function type '" << target << "'");
    }
}

void cx::validateGenericArgCount(size_t genericParamCount, llvm::ArrayRef<Type> genericArgs, llvm::StringRef name, Location location) {
    if (genericArgs.size() < genericParamCount) {
        REPORT_ERROR(location, "too few generic arguments to '" << name << "', expected " << genericParamCount);
    } else if (genericArgs.size() > genericParamCount) {
        REPORT_ERROR(location, "too many generic arguments to '" << name << "', expected " << genericParamCount);
    }
}

llvm::StringMap<Type> Typechecker::getGenericArgsForCall(llvm::ArrayRef<GenericParamDecl> genericParams, CallExpr& call, FunctionDecl* decl, bool returnOnError,
                                                         Type expectedType) {
    ASSERT(!genericParams.empty());
    std::vector<Type> inferredGenericArgs;
    llvm::ArrayRef<Type> genericArgTypes;

    if (call.genericArgs.empty()) {
        if (expectedType && expectedType.isBasicType() && !expectedType.getGenericArgs().empty()
            && llvm::none_of(expectedType.getGenericArgs(), [](Type t) { return t.isUnresolvedType(); })
            && BasicType::get(expectedType.getName(), {}).getDecl() == (decl->isConstructorDecl() ? decl->getTypeDecl() : decl->getReturnType().getDecl())) {
            genericArgTypes = expectedType.getGenericArgs();
        } else if (call.args.empty()) {
            if (returnOnError) return {};
            ERROR(call.location, "can't infer generic parameters, please specify them explicitly");
        } else {
            inferredGenericArgs = inferGenericArgsFromCallArgs(genericParams, call, decl->getParams(), returnOnError);
            if (inferredGenericArgs.empty()) return {};
            ASSERT(inferredGenericArgs.size() == genericParams.size());
            genericArgTypes = inferredGenericArgs;
        }
    } else {
        genericArgTypes = call.genericArgs;
    }

    llvm::StringMap<Type> genericArgs;
    auto genericArg = genericArgTypes.begin();

    for (const GenericParamDecl& genericParam : genericParams) {
        genericArgs.try_emplace(genericParam.getName(), *genericArg++);
    }

    return genericArgs;
}

Type Typechecker::typecheckBuiltinConversion(CallExpr& expr) {
    if (expr.args.size() != 1) {
        ERROR(expr.location, "expected single argument to converting constructor");
    }
    if (!expr.genericArgs.empty()) {
        ERROR(expr.location, "expected no generic arguments to converting constructor");
    }
    if (!expr.args.front().name.empty()) {
        ERROR(expr.location, "expected unnamed argument to converting constructor");
    }

    auto sourceType = typecheckExpr(*expr.args.front().value);
    auto targetType = BasicType::get(expr.getFunctionName(), {});

    if (sourceType == targetType) {
        WARN(expr.callee->location, "unnecessary conversion to same type");
    }

    expr.type = targetType;
    return expr.type;
}

static std::vector<Note> getCandidateNotes(llvm::ArrayRef<Decl*> unfilteredCandidates, const CallExpr& expr) {
    std::vector<Decl*> candidates;
    for (Decl* candidate : unfilteredCandidates) {
        FunctionDecl* functionDecl = llvm::dyn_cast<FunctionDecl>(candidate);
        if (!functionDecl) {
            auto functionTemplate = llvm::dyn_cast<FunctionTemplate>(candidate);
            functionDecl = functionTemplate ? functionTemplate->functionDecl : nullptr;
        }

        if (!functionDecl || functionDecl->getParams().size() == expr.args.size()) {
            candidates.push_back(candidate);
        }
    }
    if (candidates.empty()) candidates = unfilteredCandidates;

    bool multipleModules = candidates.size() > 1 && llvm::any_of(candidates, [&](Decl* c) { return c->getModule() != candidates[0]->getModule(); });

    return map(candidates, [&](Decl* c) {
        auto message = "candidate function" + (multipleModules && c->getModule() ? " in module '" + c->getModule()->name + "'" : "") + ":";
        return Note{c->getLocation(), std::move(message)};
    });
}

static const Match* findMatchByPredicate(llvm::ArrayRef<Match> matches, const CallExpr& call, llvm::function_ref<bool(Type param, Type arg)> predicate) {
    const Match* result = nullptr;

    for (auto& match : matches) {
        std::vector<ParamDecl> params;
        if (auto functionDecl = llvm::dyn_cast<FunctionDecl>(match.decl)) {
            params = functionDecl->getParams();
        } else if (auto variableDecl = llvm::dyn_cast<VariableDecl>(match.decl)) {
            params = llvm::cast<FunctionType>(variableDecl->type.typeBase)->getParamDecls();
        } else {
            llvm_unreachable("unhandled callee decl");
        }

        if (params.size() == call.args.size()) {
            if (llvm::all_of(llvm::zip_first(params, call.args), [&](auto&& pair) {
                    auto&& [param, arg] = pair;
                    return predicate(param.type, arg.value->type);
                })) {
                if (result) return nullptr;
                result = &match;
            }
        }
    }

    return result;
}

static bool isStdlibDecl(const Match& match) {
    return match.decl->getModule() && match.decl->getModule()->name == "std";
}

static bool isCHeaderDecl(const Match& match) {
    return match.decl->getModule() && match.decl->getModule()->name.ends_with("_h");
}

static const Match* resolveAmbiguousOverload(llvm::ArrayRef<Match> matches, const CallExpr& call) {
    if (llvm::count_if(matches, isStdlibDecl) == 1 && llvm::all_of(matches, [](auto& match) { return isStdlibDecl(match) || isCHeaderDecl(match); })) {
        return llvm::find_if(matches, isStdlibDecl);
    } else if (llvm::all_of(matches, isCHeaderDecl)) {
        // Redeclarations in multiple C headers are considered the same declaration, so just return one of them.
        return &matches[0];
    } else if (llvm::count_if(matches, [](auto& match) { return match.didConvertArguments == false; }) == 1) {
        return llvm::find_if(matches, [](auto& match) { return match.didConvertArguments == false; });
    } else if (auto match = findMatchByPredicate(matches, call, [](Type param, Type arg) { return param == arg; })) {
        return match;
    } else if (auto match = findMatchByPredicate(matches, call, [](Type param, Type arg) { return param == arg.getPointerTo(); })) {
        return match;
    } else {
        return nullptr;
    }
}

static bool equals(const llvm::StringMap<Type>& a, const llvm::StringMap<Type>& b) {
    if (a.size() != b.size()) return false;

    for (auto& aEntry : a) {
        auto bEntry = b.find(aEntry.getKey());
        if (bEntry == b.end() || aEntry.getValue() != bEntry->getValue()) return false;
    }

    return true;
}

static std::vector<ParamDecl> getVariableCalleeParams(const VariableDecl& calleeDecl) {
    if (calleeDecl.type.isClosureType()) {
        return map(calleeDecl.type.getClosureParamTypes(), [&](Type paramType) { return ParamDecl(paramType, "", false, calleeDecl.getLocation()); });
    }
    return llvm::cast<FunctionType>(calleeDecl.type.typeBase)->getParamDecls(calleeDecl.getLocation());
}

Decl* Typechecker::resolveOverload(llvm::ArrayRef<Decl*> decls, CallExpr& expr, llvm::StringRef callee, Type expectedType) {
    std::vector<Match> matches;
    std::vector<Match> templateMatches;
    llvm::ArrayRef<Decl*> candidates = decls;
    std::vector<ConstructorDecl*> constructorDecls;
    bool isConstructorCall = false;

    for (Decl* decl : decls) {
        switch (decl->kind) {
        case DeclKind::FunctionTemplate: {
            auto* functionTemplate = llvm::cast<FunctionTemplate>(decl);
            auto genericParams = functionTemplate->genericParams;

            if (functionTemplate->functionDecl->hasPack()) {
                if (!expr.genericArgs.empty()) {
                    if (decls.size() == 1) {
                        ERROR(expr.location, "cannot specify generic arguments explicitly for variadic function '" << expr.getFunctionName() << "'");
                    }
                    continue;
                }

                auto variadicArgs = inferVariadicGenericArgs(genericParams, expr, functionTemplate->functionDecl->getParams(), decls.size() != 1);
                if (!variadicArgs) continue;

                auto* functionDecl = functionTemplate->instantiateVariadic(variadicArgs->fixedArgs, variadicArgs->packArgs, std::move(variadicArgs->cacheKey));

                if (decls.size() == 1) {
                    validateAndConvertArguments(expr, *functionDecl, callee, expr.callee->location);
                    deferTypechecking(functionDecl);
                    return functionDecl;
                }
                if (auto match = matchArguments(expr, functionDecl)) {
                    templateMatches.push_back(*match);
                }
                break;
            }

            if (!expr.genericArgs.empty() && expr.genericArgs.size() != genericParams.size()) {
                if (decls.size() == 1) {
                    validateGenericArgCount(genericParams.size(), expr.genericArgs, expr.getFunctionName(), expr.location);
                }
                continue;
            }

            auto genericArgs = getGenericArgsForCall(genericParams, expr, functionTemplate->functionDecl, decls.size() != 1, expectedType);
            if (genericArgs.empty()) continue; // Couldn't infer generic arguments.

            auto* functionDecl = functionTemplate->instantiate(genericArgs);

            if (decls.size() == 1) {
                validateAndConvertArguments(expr, *functionDecl, callee, expr.callee->location);
                deferTypechecking(functionDecl);
                return functionDecl;
            }
            if (auto match = matchArguments(expr, functionDecl)) {
                templateMatches.push_back(*match);
            }
            break;
        }
        case DeclKind::FunctionDecl:
        case DeclKind::MethodDecl:
        case DeclKind::ConstructorDecl: {
            auto functionDecl = llvm::cast<FunctionDecl>(decl);

            // TODO: Figure out where to perform this.
            if (functionDecl && functionDecl->getTypeDecl() && functionDecl->getTypeDecl()->isInterface()) {
                functionDecl = functionDecl->instantiate({{"This", functionDecl->getTypeDecl()->getType()}}, {});
            }

            if (decls.size() == 1) {
                validateGenericArgCount(0, expr.genericArgs, expr.getFunctionName(), expr.location);
                validateAndConvertArguments(expr, *functionDecl, callee, expr.callee->location);
                return functionDecl;
            }
            if (auto match = matchArguments(expr, functionDecl)) {
                matches.push_back(*match);
            }
            break;
        }
        case DeclKind::TypeDecl: {
            auto* typeDecl = llvm::cast<TypeDecl>(decl);
            isConstructorCall = true;
            constructorDecls = typeDecl->getConstructors();
            if (constructorDecls.empty()) {
                // Interfaces and C-imported unions have no constructors, so calling one is always an error.
                if (typeDecl->isInterface()) {
                    ERROR(expr.callee->location, "cannot construct interface '" << typeDecl->getName() << "'");
                }
                ERROR(expr.callee->location, "type '" << typeDecl->getName() << "' has no constructors");
            }
            if (decls.size() == 1) {
                candidates = llvm::ArrayRef(reinterpret_cast<Decl**>(constructorDecls.data()), constructorDecls.size());
            }

            // With explicit generic arguments and competing same-named declarations, let the other declarations
            // handle the call (a non-generic struct can't match explicitly-generic arguments anyway).
            if (!expr.genericArgs.empty() && decls.size() != 1) {
                continue;
            }
            validateGenericArgCount(0, expr.genericArgs, expr.getFunctionName(), expr.location);

            for (auto* constructorDecl : constructorDecls) {
                if (decls.size() == 1 && constructorDecls.size() == 1) {
                    validateAndConvertArguments(expr, *constructorDecl, callee, expr.callee->location);
                    return constructorDecl;
                }
                if (auto match = matchArguments(expr, constructorDecl)) {
                    matches.push_back(*match);
                }
            }
            break;
        }
        case DeclKind::TypeTemplate: {
            auto* typeTemplate = llvm::cast<TypeTemplate>(decl);
            isConstructorCall = true;
            constructorDecls = typeTemplate->typeDecl->getConstructors();
            if (decls.size() == 1) {
                candidates = llvm::ArrayRef(reinterpret_cast<Decl**>(constructorDecls.data()), constructorDecls.size());
            }

            std::vector<llvm::StringMap<Type>> genericArgSets;

            for (auto* constructorDecl : constructorDecls) {
                auto genericArgs =
                    getGenericArgsForCall(typeTemplate->genericParams, expr, constructorDecl, decls.size() != 1 || constructorDecls.size() != 1, expectedType);
                if (genericArgs.empty()) continue; // Couldn't infer generic arguments.
                if (llvm::find_if(genericArgSets, [&](auto& set) { return equals(set, genericArgs); }) == genericArgSets.end()) {
                    genericArgSets.push_back(genericArgs);
                }
            }

            for (auto& genericArgs : genericArgSets) {
                TypeDecl* typeDecl = nullptr;

                auto genericArgTypes = map(genericArgs, [](auto& entry) { return entry.getValue(); });
                auto typeDecls = findDecls(getQualifiedTypeName(typeTemplate->typeDecl->getName(), genericArgTypes));

                if (typeDecls.empty()) {
                    typeDecl = typeTemplate->instantiate(genericArgs);
                    currentModule->addToSymbolTable(*typeDecl);
                    deferTypechecking(typeDecl);
                } else {
                    typeDecl = llvm::cast<TypeDecl>(typeDecls[0]);
                }

                for (auto* constructorDecl : typeDecl->getConstructors()) {
                    if (decls.size() == 1 && constructorDecls.size() == 1) {
                        validateAndConvertArguments(expr, *constructorDecl, callee, expr.callee->location);
                        return constructorDecl;
                    }
                    if (auto match = matchArguments(expr, constructorDecl)) {
                        templateMatches.push_back(*match);
                    }
                }
            }
            break;
        }
        case DeclKind::VarDecl:
        case DeclKind::ParamDecl:
        case DeclKind::FieldDecl: {
            auto* variableDecl = llvm::cast<VariableDecl>(decl);

            // The initializer errored; the real error was already reported.
            if (!variableDecl->type) {
                if (decls.size() == 1) throw CompileError::dependentError();
                break;
            }

            if (variableDecl->type.isFunctionType() || variableDecl->type.isClosureType()) {
                auto paramDecls = getVariableCalleeParams(*variableDecl);

                if (decls.size() == 1) {
                    validateAndConvertArguments(expr, paramDecls, false, callee, expr.callee->location);
                    return variableDecl;
                }
                if (auto match = matchArguments(expr, variableDecl, paramDecls)) {
                    matches.push_back(*match);
                }
            }
            break;
        }
        case DeclKind::DestructorDecl:
            matches.push_back({decl, false});
            break;

        default:
            continue;
        }
    }

    if (matches.empty()) {
        matches = std::move(templateMatches);
    }

    // Distinct same-named templates (e.g. a local type shadowing a stdlib type) resolve to the same
    // instantiation, matching the same declaration twice. A duplicate match is still one candidate.
    std::vector<Match> uniqueMatches;
    for (auto& match : matches) {
        if (!llvm::any_of(uniqueMatches, [&](auto& unique) { return unique.decl == match.decl; })) {
            uniqueMatches.push_back(match);
        }
    }
    matches = std::move(uniqueMatches);

    if (matches.size() > 1) {
        bool hasNonPack = llvm::any_of(matches, [](const Match& match) {
            auto* functionDecl = llvm::dyn_cast<FunctionDecl>(match.decl);
            return !functionDecl || !functionDecl->isPackInstantiation;
        });
        if (hasNonPack) {
            std::vector<Match> nonPackMatches;
            for (auto& match : matches) {
                auto* functionDecl = llvm::dyn_cast<FunctionDecl>(match.decl);
                if (!functionDecl || !functionDecl->isPackInstantiation) nonPackMatches.push_back(match);
            }
            matches = std::move(nonPackMatches);
        }
    }

    auto calleeWithGenericArgs = getQualifiedTypeName(callee, expr.genericArgs);

    if (matches.size() > 1) {
        for (auto& arg : expr.args) {
            if (!arg.value->hasType()) {
                typecheckExpr(*arg.value);
            }
        }

        if (auto match = resolveAmbiguousOverload(matches, expr)) {
            matches = {*match};
        } else {
            ERROR_WITH_NOTES(expr.callee->location, getCandidateNotes(map(matches, [](auto& match) { return match.decl; }), expr),
                             "ambiguous reference to '" << calleeWithGenericArgs << "'" << (isConstructorCall ? " constructor" : ""));
        }
    }

    if (matches.size() == 1) {
        validateAndConvertArguments(expr, *matches.front().decl);
        deferTypechecking(matches.front().decl);
        return matches.front().decl;
    }

    if (decls.empty()) {
        ERROR(expr.callee->location, "unknown identifier '" << callee << "'");
    }

    bool atLeastOneFunction =
        llvm::any_of(decls, [](Decl* decl) { return decl->isFunctionDecl() || decl->isFunctionTemplate() || decl->isTypeDecl() || decl->isTypeTemplate(); });

    if (atLeastOneFunction) {
        if (auto binaryExpr = llvm::dyn_cast<BinaryExpr>(&expr)) {
            // Don't list candidate functions for operators; they're usually irrelevant stdlib overloads that drown out the actual error.
            ERROR(expr.callee->location, "no matching operator '" << binaryExpr->op << "' with arguments '" << binaryExpr->getLHS().type << "' and '"
                                                                  << binaryExpr->getRHS().type << "'");
        } else {
            auto argTypes = map(expr.args, [&](const NamedValue& arg) { return typecheckExpr(*arg.value).toString(); });
            ERROR_WITH_NOTES(expr.callee->location, getCandidateNotes(candidates, expr),
                             (isConstructorCall ? "no matching constructor '" : "no matching function '")
                                 << calleeWithGenericArgs << "(" << llvm::join(argTypes, ", ") << ")'");
        }
    } else {
        ERROR(expr.callee->location, "'" << callee << "' is not a function");
    }
}

std::vector<Decl*> Typechecker::findCalleeCandidates(const CallExpr& expr, llvm::StringRef callee) {
    TypeDecl* receiverTypeDecl;

    if (expr.receiverType && expr.receiverType.removePointer().isBasicType()) {
        receiverTypeDecl = getTypeDecl(*llvm::cast<BasicType>(expr.receiverType.removePointer().typeBase));
    } else {
        receiverTypeDecl = nullptr;
    }

    return findDecls(callee, receiverTypeDecl, isPostProcessing);
}

Type Typechecker::typecheckCallExpr(CallExpr& expr, Type expectedType) {
    if (!expr.callsNamedFunction()) {
        ERROR(expr.location, "anonymous function calls not implemented yet");
    }

    if (Type::isBuiltinScalar(expr.getFunctionName())) {
        return typecheckBuiltinConversion(expr);
    }

    if (expr.isBuiltinCast()) {
        return typecheckBuiltinCast(expr);
    }

    if (expr.getFunctionName() == "assert") {
        ParamDecl assertParam(Type::getBool(), "", false, Location());
        validateAndConvertArguments(expr, assertParam, false, expr.getFunctionName(), expr.location);
        validateGenericArgCount(0, expr.genericArgs, expr.getFunctionName(), expr.location);
        return Type::getVoid();
    }

    Decl* decl;

    if (auto* enumCase = getEnumCase(*expr.callee)) {
        decl = enumCase;
        llvm::cast<MemberExpr>(*expr.callee).decl = decl;
    } else if (expr.callee->isMemberExpr()) {
        Type receiverType = typecheckExpr(*expr.getReceiver());
        expr.receiverType = receiverType;

        if (receiverType.removeOptional().removePointer().isArrayType()) {
            // TODO: Move these member functions to a 'struct Array' declaration in stdlib.
            if (expr.getFunctionName() == "data") {
                validateAndConvertArguments(expr, {}, false, expr.getFunctionName(), expr.location);
                validateGenericArgCount(0, expr.genericArgs, expr.getFunctionName(), expr.location);
                return ArrayType::get(receiverType.removePointer().getElementType(), ArrayType::UnknownSize);
            }
            if (expr.getFunctionName() == "size") {
                validateAndConvertArguments(expr, {}, false, expr.getFunctionName(), expr.location);
                validateGenericArgCount(0, expr.genericArgs, expr.getFunctionName(), expr.location);
                return ArrayType::getIndexType();
            }
            if (expr.getFunctionName() == "iterator") {
                validateAndConvertArguments(expr, {}, false, expr.getFunctionName(), expr.location);
                validateGenericArgCount(0, expr.genericArgs, expr.getFunctionName(), expr.location);
                return BasicType::get("ArrayIterator", receiverType.removePointer().getElementType());
            }

            ERROR(expr.getReceiver()->location, "type '" << receiverType.removePointer() << "' has no member function '" << expr.getFunctionName() << "'");
        } else if (receiverType.removeOptional().removePointer().isBuiltinType() && expr.getFunctionName() == "deinit") {
            return Type::getVoid();
        }

        if (expr.args.size() == 1 && expr.getFunctionName() == "init") {
            typecheckExpr(*expr.args[0].value);

            if (expr.isMoveInit()) {
                if (!expr.args[0].value->type.isImplicitlyCopyable()) {
                    setMoved(expr.args[0].value, true);
                }
                return Type::getVoid();
            }
        }

        auto callee = expr.getQualifiedFunctionName();
        auto decls = findCalleeCandidates(expr, callee);

        if (decls.empty() && expr.getFunctionName() == "deinit") {
            return Type::getVoid();
        }

        decl = resolveOverload(decls, expr, callee, expectedType);
    } else {
        auto callee = expr.getFunctionName();
        auto decls = findCalleeCandidates(expr, callee);
        decl = resolveOverload(decls, expr, callee, expectedType);

        if (auto* constructorDecl = llvm::dyn_cast<ConstructorDecl>(decl)) {
            expr.receiverType = constructorDecl->getTypeDecl()->getType();
        } else if (decl->isMethodDecl()) {
            auto* varDecl = llvm::cast<VarDecl>(findDecl("this", expr.callee->location));
            maybeCaptureVariable(*varDecl);
            expr.receiverType = varDecl->type;
        }
    }

    checkHasAccess(*decl, expr.callee->location, AccessLevel::None);
    if (auto* constructorDecl = llvm::dyn_cast<ConstructorDecl>(decl)) {
        checkHasAccess(*constructorDecl->getTypeDecl(), expr.callee->location, AccessLevel::None);
    }

    std::vector<ParamDecl> params;

    if (auto functionDecl = llvm::dyn_cast<FunctionDecl>(decl)) {
        params = functionDecl->getParams();
    } else if (auto variableDecl = llvm::dyn_cast<VariableDecl>(decl)) {
        params = getVariableCalleeParams(*variableDecl);
    } else {
        auto type = llvm::cast<EnumCase>(decl)->associatedType;
        params = map(type.getTupleElements(), [&](auto& e) { return ParamDecl(e.type, std::string(e.name), false, decl->getLocation()); });
        validateAndConvertArguments(expr, params, false, decl->getName(), expr.location);
    }

    for (auto&& [param, arg] : llvm::zip_longest(params, expr.args)) {
        if (arg && !arg->value->type.isImplicitlyCopyable() && (!param || !param->type.isImplicitlyCopyable())) {
            setMoved(arg->value, true);
        }
    }

    expr.calleeDecl = decl;
    decl->referenced = true;

    if (auto* variableDecl = llvm::dyn_cast<VariableDecl>(decl)) {
        maybeCaptureVariable(*variableDecl);
        // Closure calls emit the callee to load the closure value, so it must be resolved like other lvalues.
        if (variableDecl->type.isClosureType()) {
            if (auto* varExpr = llvm::dyn_cast<VarExpr>(expr.callee)) {
                checkNotMoved(*variableDecl, *varExpr);
                varExpr->decl = variableDecl;
                varExpr->type = variableDecl->type;
                varExpr->assignableType = variableDecl->type;
            } else if (llvm::isa<MemberExpr>(expr.callee)) {
                typecheckExpr(*expr.callee);
            }
        }
    }

    if (auto constructorDecl = llvm::dyn_cast<ConstructorDecl>(decl)) {
        if (constructorDecl->getTypeDecl()->isInterface()) {
            typecheckFunctionDecl(*constructorDecl);
        }
        return llvm::cast<ConstructorDecl>(decl)->getTypeDecl()->getType();
    } else if (auto functionDecl = llvm::dyn_cast<FunctionDecl>(decl)) {
        return functionDecl->getFunctionType()->returnType;
    } else if (auto variableDecl = llvm::dyn_cast<VariableDecl>(decl)) {
        if (variableDecl->type.isClosureType()) {
            return variableDecl->type.getClosureReturnType();
        }
        return llvm::cast<FunctionType>(variableDecl->type.typeBase)->returnType;
    } else {
        return llvm::cast<EnumCase>(decl)->type;
    }
}

ArgumentValidation Typechecker::getArgumentValidationResult(CallExpr& expr, llvm::ArrayRef<ParamDecl> params, bool isVariadic) {
    if (expr.args.size() < params.size()) {
        for (size_t i = expr.args.size(); i < params.size(); ++i) {
            if (!params[i].defaultValue) return ArgumentValidation::tooFew();
        }
    } else if (!isVariadic && expr.args.size() > params.size()) {
        return ArgumentValidation::tooMany();
    }

    bool didConvertArguments = false;

    for (size_t i = 0; i < expr.args.size(); ++i) {
        auto& arg = expr.args[i];
        auto* param = i < params.size() ? &params[i] : nullptr;

        if (!arg.name.empty() && (!param || arg.name != param->getName())) {
            return ArgumentValidation::invalidName(i);
        }

        if (param) {
            bool hadType = arg.value->hasType();

            if (!arg.value->hasType()) {
                typecheckExpr(*arg.value, false, param ? param->type : Type());
            }

            bool invalidType = false;
            std::optional<ImplicitCastExpr::Kind> implicitCastKind;
            if (Type convertedType = isImplicitlyConvertible(arg.value, arg.value->type, param->type, true, &implicitCastKind)) {
                didConvertArguments = didConvertArguments || convertedType != arg.value->type || implicitCastKind.has_value();
            } else {
                invalidType = true;
            }

            if (!hadType) arg.value->removeTypes();
            if (invalidType) return ArgumentValidation::invalidType(i);
        }
    }

    return ArgumentValidation::success(didConvertArguments);
}

std::optional<Match> Typechecker::matchArguments(CallExpr& expr, Decl* calleeDecl, llvm::ArrayRef<ParamDecl> params) {
    bool isVariadic = false;
    if (auto functionDecl = llvm::dyn_cast<FunctionDecl>(calleeDecl)) {
        params = functionDecl->getParams();
        isVariadic = functionDecl->isVariadic();
    }
    auto result = getArgumentValidationResult(expr, params, isVariadic);
    if (result.error) return std::nullopt;
    return Match{calleeDecl, result.didConvertArguments};
}

void Typechecker::validateAndConvertArguments(CallExpr& expr, const Decl& calleeDecl, llvm::StringRef functionName, Location location) {
    if (auto functionDecl = llvm::dyn_cast<FunctionDecl>(&calleeDecl)) {
        validateAndConvertArguments(expr, functionDecl->getParams(), functionDecl->isVariadic(), functionName, location);
    } else {
        auto paramDecls = getVariableCalleeParams(llvm::cast<VariableDecl>(calleeDecl));
        validateAndConvertArguments(expr, paramDecls, false, functionName, location);
    }
}

void Typechecker::validateAndConvertArguments(CallExpr& expr, llvm::ArrayRef<ParamDecl> params, bool isVariadic, llvm::StringRef callee, Location location) {
    auto result = getArgumentValidationResult(expr, params, isVariadic);

    // Arguments are type-checked here for error messages, but type-converted only in the success case below
    // (they might not convert properly in the case of error).
    for (int i = 0; i < std::max(params.size(), expr.args.size()); i++) {
        auto* arg = i < expr.args.size() ? &expr.args[i] : nullptr;
        auto* param = i < params.size() ? &params[i] : nullptr;
        if (!arg) continue;
        if (!arg->value->hasType()) typecheckExpr(*arg->value, false, param ? param->type : Type());
    }

    switch (result.error) {
    case ArgumentValidation::None:
        for (int i = 0; i < std::max(params.size(), expr.args.size()); i++) {
            auto* arg = i < expr.args.size() ? &expr.args[i] : nullptr;
            auto* param = i < params.size() ? &params[i] : nullptr;
            if (!arg) continue;
            if (param) arg->value = convert(arg->value, param->type, true);
        }
        for (size_t i = expr.args.size(); i < params.size(); ++i) {
            const ParamDecl& param = params[i];
            ASSERT(param.defaultValue);
            Expr* defaultArg = param.defaultValue->instantiate({});
            if (!defaultArg->hasType()) typecheckExpr(*defaultArg, false, param.type);
            if (Expr* converted = convert(defaultArg, param.type, true)) {
                defaultArg = converted;
            } else {
                ERROR(expr.location, "cannot assign '" << defaultArg->type << "' to '" << param.type << "'");
            }
            expr.args.emplace_back(std::string(param.getName()), defaultArg, expr.location);
        }
        break;
    case ArgumentValidation::TooFew: {
        size_t requiredParamCount = 0;
        for (auto& param : params) {
            if (!param.defaultValue) ++requiredParamCount;
        }
        bool hasOptionalParams = requiredParamCount != params.size();
        REPORT_ERROR(location, "too few arguments to '" << callee << "', expected " << ((isVariadic || hasOptionalParams) ? "at least " : "")
                                                        << (hasOptionalParams ? requiredParamCount : params.size()));
        break;
    }
    case ArgumentValidation::TooMany:
        REPORT_ERROR(location, "too many arguments to '" << callee << "', expected " << params.size());
        break;
    case ArgumentValidation::InvalidName: {
        auto& arg = expr.args[result.index];
        auto* param = &params[result.index];
        ERROR(arg.location, "invalid argument name '" << arg.name << "' for parameter '" << param->getName() << "'");
        break;
    }
    case ArgumentValidation::InvalidType: {
        auto& arg = expr.args[result.index];
        auto* param = &params[result.index];
        diagnoseClosureConversion(arg.value->type, param->type, arg.location);
        ERROR(arg.location,
              "invalid argument #" << (result.index + 1) << " type '" << arg.value->type << "' to '" << callee << "', expected '" << param->type << "'");
        break;
    }
    }
}

static bool isValidCast(Type sourceType, Type targetType) {
    switch (sourceType.getKind()) {
    case TypeKind::BasicType:
        if (sourceType.isOptionalType()) {
            Type sourceWrappedType = sourceType.getWrappedType();

            if (sourceWrappedType.isPointerType() && targetType.isOptionalType()) {
                Type targetWrappedType = targetType.getWrappedType();

                if (targetWrappedType.isPointerType() && isValidCast(sourceWrappedType, targetWrappedType)) {
                    return true;
                }
            }
        }

        if (sourceType.isInteger() && targetType.removeOptional().isPointerType()) {
            return true;
        }

        return false;

    case TypeKind::TupleType:
    case TypeKind::FunctionType:
        return false;

    case TypeKind::PointerType: {
        Type sourcePointee = sourceType.getPointee();

        if (targetType.isPointerType()) {
            Type targetPointee = targetType.getPointee();

            if (sourcePointee.isVoid() && (!targetPointee.isMutable() || sourcePointee.isMutable())) {
                return true;
            } else if (targetPointee.isVoid() && (!targetPointee.isMutable() || sourcePointee.isMutable())) {
                return true;
            } else if (targetPointee.isConstantArray() && sourcePointee == targetPointee.getElementType()) {
                return true;
            }
        } else if (targetType.isUnsizedArrayPointer()) {
            if (sourcePointee.isVoid() && (!targetType.getElementType().isMutable() || sourcePointee.isMutable())) {
                return true;
            } else if (sourcePointee == targetType.getElementType()) {
                return true;
            }
        }

        return false;
    }
    case TypeKind::ArrayType: {
        if (targetType.isPointerType()) {
            Type targetPointee = targetType.getPointee();

            if (targetPointee.isVoid() && (!targetPointee.isMutable() || sourceType.getElementType().isMutable())) {
                return true;
            }
        }

        return false;
    }
    case TypeKind::UnresolvedType:
        llvm_unreachable("invalid unresolved type");
    }

    llvm_unreachable("all cases handled");
}

Type Typechecker::typecheckBuiltinCast(CallExpr& expr) {
    Type sourceType = typecheckExpr(*expr.args.front().value);
    Type targetType = expr.genericArgs.front();
    ParamDecl param(sourceType, "", false, expr.location);

    validateGenericArgCount(1, expr.genericArgs, expr.getFunctionName(), expr.location);
    validateAndConvertArguments(expr, param, false, expr.getFunctionName(), expr.location);

    if (!isValidCast(sourceType, targetType) && !isValidCast(sourceType.removeOptional(), targetType)) {
        ERROR(expr.callee->location, "illegal cast from '" << sourceType << "' to '" << targetType << "'");
    }

    return targetType;
}

Type Typechecker::typecheckSizeofExpr(SizeofExpr& expr) {
    typecheckType(expr.operandType, AccessLevel::None);
    return Type::getUInt64();
}

Type Typechecker::typecheckMemberExpr(MemberExpr& expr) {
    if (auto* enumCase = getEnumCase(expr)) {
        checkHasAccess(*enumCase->getEnumDecl(), expr.base->location, AccessLevel::None);
        expr.decl = enumCase;
        return enumCase->type;
    }

    Type baseType = typecheckExpr(*expr.base);
    if (!expr.base->isThis()) baseType = baseType.removeOptional();
    baseType = baseType.removePointer();

    if (baseType.isArrayType()) {
        if (llvm::is_contained({"count", "length", "size"}, expr.member)) {
            ERROR(expr.location, "use the '.size()' member function to get the number of elements in an array");
        }
    } else if (baseType.isTupleType()) {
        for (auto& element : baseType.getTupleElements()) {
            if (element.name == expr.member) {
                return element.type;
            }
        }
    } else {
        for (auto& field : baseType.getDecl()->fields) {
            if (field.getName() == expr.member) {
                checkHasAccess(field, expr.location, AccessLevel::None);
                expr.decl = &field;
                return field.type.withMutability(baseType.mutability);
            }
        }
    }

    ERROR(expr.location, "no member named '" << expr.member << "' in '" << baseType << "'");
}

Type Typechecker::typecheckIndexExpr(IndexExpr& expr) {
    Type lhsType = typecheckExpr(*expr.getBase());
    Type arrayType;

    if (lhsType.removeOptional().isArrayType()) {
        arrayType = lhsType.removeOptional();
    } else if (lhsType.isPointerType() && lhsType.getPointee().isArrayType()) {
        arrayType = lhsType.getPointee();
    } else if (lhsType.removeOptional().removePointer().isBuiltinType()) {
        ERROR(expr.location, "'" << lhsType << "' doesn't provide an index operator");
    } else {
        return typecheckCallExpr(expr).removePointer();
    }

    Expr* indexExpr = expr.getIndex();
    Type indexType = typecheckExpr(*indexExpr);

    if (auto converted = convert(indexExpr, ArrayType::getIndexType())) {
        expr.setIndex(converted);
        indexExpr = converted;
    } else {
        ERROR(indexExpr->location, "illegal index type '" << indexType << "', expected '" << ArrayType::getIndexType() << "'");
    }

    if (arrayType.isConstantArray()) {
        if (indexExpr->isConstant()) {
            auto index = indexExpr->getConstantIntegerValue();

            if (index < 0 || index >= arrayType.getArraySize()) {
                WARN(indexExpr->location, "accessing array out-of-bounds with index " << index << ", array size is " << arrayType.getArraySize());
            }
        }
    }

    return arrayType.getElementType();
}

Type Typechecker::typecheckIndexAssignmentExpr(IndexAssignmentExpr& expr) {
    auto elementType = typecheckIndexExpr(expr);

    if (!expr.getBase()->type.removeOptional().removePointer().isArrayType()) {
        return typecheckCallExpr(expr);
    }

    typecheckExpr(*expr.getValue());

    if (auto converted = convert(expr.getValue(), elementType)) {
        expr.setValue(converted);
    } else {
        ERROR(expr.getValue()->location, "cannot assign '" << expr.getValue()->type << "' to '" << elementType << "'");
    }

    return Type::getVoid();
}

Type Typechecker::typecheckUnwrapExpr(UnwrapExpr& expr) {
    Type type = typecheckExpr(*expr.operand);
    if (!type.isOptionalType()) {
        WARN(expr.location, "unwrapping non-optional type '" << type << "' has no effect");
        return type;
    }
    return type.getWrappedType();
}

// A capturing lambda evaluates to a closure: an anonymous struct holding the function
// plus the captured values. The function takes the captures as hidden leading parameters,
// so its stored type includes them ahead of the user parameters.
static Type createClosureType(FunctionDecl& lambdaDecl, Location location) {
    static uint64_t nameCounter = 0;
    std::string name = "__closure" + std::to_string(nameCounter++);
    Module& module = *lambdaDecl.getModule();

    std::vector<Type> fnParamTypes;
    for (auto* captured : lambdaDecl.captures) {
        fnParamTypes.push_back(captured->type);
    }
    for (auto& param : lambdaDecl.getParams()) {
        fnParamTypes.push_back(param.type);
    }
    Type fnType = FunctionType::get(lambdaDecl.getReturnType(), std::move(fnParamTypes), false);

    std::vector<Type> interfaces;
    if (llvm::all_of(lambdaDecl.captures, [](auto* captured) { return captured->type.isImplicitlyCopyable(); })) {
        interfaces.push_back(BasicType::get("Copyable", {}));
    }

    // Default access: closures are anonymous, so they can't leak through API surfaces the way named private types can.
    auto* closureDecl =
        makeAST<TypeDecl>(TypeTag::Struct, std::move(name), std::vector<Type>(), std::move(interfaces), AccessLevel::Default, module, nullptr, location);
    closureDecl->addField(FieldDecl(fnType, "__fn", nullptr, *closureDecl, AccessLevel::Private, location));
    for (auto* captured : lambdaDecl.captures) {
        closureDecl->addField(FieldDecl(captured->type, ("__capture_" + captured->getName()).str(), nullptr, *closureDecl, AccessLevel::Private, location));
    }

    Type closureType = BasicType::get(closureDecl->getName(), {});
    llvm::cast<BasicType>(closureType.typeBase)->decl = closureDecl;
    return closureType.withLocation(location);
}

Type Typechecker::typecheckLambdaExpr(LambdaExpr& expr, Type expectedType) {
    for (size_t i = 0, e = expr.functionDecl->getParams().size(); i < e; ++i) {
        auto& param = expr.functionDecl->getParams()[i];
        if (!param.type) {
            Type inferredType;
            if (expectedType && expectedType.isFunctionType() && i < expectedType.getParamTypes().size()) {
                inferredType = expectedType.getParamTypes()[i];
            }
            if (!inferredType) {
                ERROR(param.getLocation(), "couldn't infer type for parameter '" << param.getName() << "'");
            }
            param.type = NOTNULL(inferredType);
        }
    }

    expr.functionDecl->parentFunction = currentFunction;
    typecheckFunctionDecl(*expr.functionDecl);

    if (expr.functionDecl->captures.empty()) {
        return Type(expr.functionDecl->getFunctionType(), Mutability::Mutable, expr.location);
    }

    for (auto* captured : expr.functionDecl->captures) {
        VarExpr use(captured->getName().str(), expr.location);
        checkNotMoved(*captured, use);
        if (!captured->type.isImplicitlyCopyable()) {
            movedDecls.insert(captured);
        }
    }

    return createClosureType(*expr.functionDecl, expr.location);
}

Type Typechecker::typecheckIfExpr(IfExpr& expr) {
    auto conditionType = typecheckExpr(*expr.condition);
    typecheckImplicitlyBoolConvertibleExpr(conditionType, expr.condition->location);
    auto outerNarrowings = narrowedTypes;
    applyNarrowings(*expr.condition, true);
    auto thenType = typecheckExpr(*expr.thenExpr);
    auto thenNarrowings = narrowedTypes;
    narrowedTypes = outerNarrowings;
    applyNarrowings(*expr.condition, false);
    auto elseType = typecheckExpr(*expr.elseExpr);
    intersectNarrowings(thenNarrowings);

    if (auto convertedElse = convert(expr.elseExpr, thenType)) {
        expr.elseExpr = convertedElse;
        return thenType;
    } else if (auto convertedThen = convert(expr.thenExpr, elseType)) {
        expr.thenExpr = convertedThen;
        return elseType;
    } else {
        ERROR(expr.location, "incompatible operand types ('" << thenType << "' and '" << elseType << "')");
    }
}

Type Typechecker::typecheckExpr(Expr& expr, bool useIsWriteOnly, Type expectedType) {
    Type type;

    switch (expr.kind) {
    case ExprKind::VarExpr:
        type = typecheckVarExpr(llvm::cast<VarExpr>(expr), useIsWriteOnly);
        if (!type) throw CompileError::dependentError(); // Variable initializer had an error, don't report uses of that variable as errors.
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
        type = typecheckNullLiteralExpr(llvm::cast<NullLiteralExpr>(expr), expectedType);
        break;
    case ExprKind::UndefinedLiteralExpr:
        type = typecheckUndefinedLiteralExpr(llvm::cast<UndefinedLiteralExpr>(expr), expectedType);
        break;
    case ExprKind::ArrayLiteralExpr:
        type = typecheckArrayLiteralExpr(llvm::cast<ArrayLiteralExpr>(expr), expectedType);
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
        type = typecheckCallExpr(llvm::cast<CallExpr>(expr), expectedType);
        break;
    case ExprKind::SizeofExpr:
        type = typecheckSizeofExpr(llvm::cast<SizeofExpr>(expr));
        break;
    case ExprKind::MemberExpr:
        type = typecheckMemberExpr(llvm::cast<MemberExpr>(expr));
        break;
    case ExprKind::IndexExpr:
        type = typecheckIndexExpr(llvm::cast<IndexExpr>(expr));
        break;
    case ExprKind::IndexAssignmentExpr:
        type = typecheckIndexAssignmentExpr(llvm::cast<IndexAssignmentExpr>(expr));
        break;
    case ExprKind::UnwrapExpr:
        type = typecheckUnwrapExpr(llvm::cast<UnwrapExpr>(expr));
        break;
    case ExprKind::LambdaExpr:
        type = typecheckLambdaExpr(llvm::cast<LambdaExpr>(expr), expectedType);
        break;
    case ExprKind::IfExpr:
        type = typecheckIfExpr(llvm::cast<IfExpr>(expr));
        break;
    case ExprKind::ImplicitCastExpr:
        typecheckExpr(*llvm::cast<ImplicitCastExpr>(expr).operand, useIsWriteOnly, expectedType);
        type = expr.type;
        break;
    case ExprKind::VarDeclExpr:
        typecheckVarDecl(*llvm::cast<VarDeclExpr>(expr).varDecl);
        type = llvm::cast<VarDeclExpr>(expr).varDecl->type;
        break;
    }

    expr.type = type;
    expr.assignableType = type;

    if (expr.kind == ExprKind::VarExpr) {
        auto* decl = llvm::cast<VarExpr>(expr).decl;
        if (decl && narrowedTypes.contains(decl)) {
            expr.assignableType = llvm::cast<VariableDecl>(decl)->type;
        }
    }

    if (!type.isUndefined()) { // TODO: Don't special-case the 'undefined' type.
        // Expression types derive from already-checked declarations, so rechecking their
        // generic arguments would only duplicate warnings (e.g. inside generic method bodies).
        typecheckType(type, AccessLevel::None, /*recheckGenericArgs=*/false);
    }

    return expr.type;
}

EnumCase* Typechecker::getEnumCase(const Expr& expr) {
    if (auto* memberExpr = llvm::dyn_cast<MemberExpr>(&expr)) {
        if (auto* varExpr = llvm::dyn_cast<VarExpr>(memberExpr->base)) {
            auto decls = findDecls(varExpr->identifier);
            if (decls.size() == 1) {
                if (auto* enumDecl = llvm::dyn_cast<EnumDecl>(decls.front())) {
                    auto* enumCase = enumDecl->getCaseByName(memberExpr->member);
                    if (!enumCase) {
                        ERROR(expr.location, "enum '" << enumDecl->getName() << "' has no case named '" << memberExpr->member << "'");
                    }
                    return enumCase;
                }
            }
        }
    }

    return nullptr;
}

void Typechecker::setMoved(Expr* expr, bool isMoved) {
    if (auto* varExpr = llvm::dyn_cast<VarExpr>(expr)) {
        ASSERT(varExpr->decl);

        // Moving out of a capture would leave the closure's stored copy in a moved-from state
        // while the closure stays callable, so only copies of implicitly copyable captures are allowed out.
        if (isMoved && currentFunction && currentFunction->isLambda() && varExpr->type && !varExpr->type.isImplicitlyCopyable()) {
            if (auto* variableDecl = llvm::dyn_cast<VariableDecl>(varExpr->decl)) {
                auto* parent = variableDecl->parent;
                if ((variableDecl->kind == DeclKind::VarDecl || variableDecl->kind == DeclKind::ParamDecl) && parent && parent->isFunctionDecl()
                    && parent != currentFunction) {
                    ERROR(varExpr->location, "cannot move from captured variable '" << varExpr->identifier << "'");
                }
            }
        }

        if (isMoved) {
            movedDecls.insert(varExpr->decl);
        } else {
            movedDecls.erase(varExpr->decl);
        }
    }
}

void Typechecker::checkNotMoved(const Decl& decl, const VarExpr& expr) {
    if (movedDecls.count(&decl)) {
        ERROR(expr.location, "use of moved value '" << expr.identifier << "'");
    }
}
