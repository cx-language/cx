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
    // Access warnings for members of generic instantiations are suppressed:
    // the use-site type expression is already checked with the use location
    // (see the generic-argument rechecking in typecheckType), so checks with
    // template-definition locations would only produce duplicate noise.
    if (suppressAccessWarnings) return;
    if (decl.accessLevel == AccessLevel::Private && !inSameModule(decl, location)) {
        WARN(location, "'" << decl.getName() << "' is private");
    } else if (userAccessLevel != AccessLevel::None && decl.accessLevel < userAccessLevel) {
        WARN(location, "using " << decl.accessLevel << " type '" << decl.getName() << "' in " << userAccessLevel << " declaration");
    }
}

bool Typechecker::inSameModule(const Decl& decl, Location location) const {
    if (Module* declModule = decl.getModule()) {
        if (Module* useModule = findModuleForFile(location.file)) return declModule == useModule;
    }
    // Fall back to comparing file paths for declarations without module information.
    const char* declFile = decl.getLocation().file;
    return declFile && location.file && strcmp(declFile, location.file) == 0;
}

Module* Typechecker::findModuleForFile(const char* file) const {
    if (!file) return nullptr;
    auto matches = [&](const Module* module) {
        for (auto& sourceFile : module->sourceFiles) {
            if (sourceFile.filePath == file) return true;
        }
        return false;
    };
    if (currentModule && matches(currentModule)) return currentModule;
    for (Module* module : Module::getAllImportedModules()) {
        if (matches(module)) return module;
    }
    return nullptr;
}

void Typechecker::maybeCaptureVariable(VariableDecl& variableDecl) {
    if (!currentFunction || !currentFunction->isLambda()) return;
    if (variableDecl.kind != DeclKind::VarDecl && variableDecl.kind != DeclKind::ParamDecl) return;
    auto* parent = variableDecl.parent;
    if (!parent || !parent->isFunctionDecl() || parent == currentFunction) return;
    // Capture transitively: every lambda between the use and the owner must capture too,
    // so codegen can resolve the decl through each intermediate scope.
    for (FunctionDecl* function = currentFunction; function && function->isLambda() && function != parent; function = function->parentFunction) {
        if (!llvm::is_contained(function->captures, &variableDecl)) {
            function->captures.push_back(&variableDecl);
        }
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

// Maps call args to params: named args by name (order-free), positional args to
// the next unassigned param in declaration order. Missing params must have defaults.
// On success fills argToParam (param index or -1 for variadic extra) and paramToArg
// (arg index or -1 for missing) and returns nullopt; otherwise returns the error.
static std::optional<ArgumentValidation> computeArgParamMapping(llvm::ArrayRef<NamedValue> args, llvm::ArrayRef<ParamDecl> params, bool isVariadic,
                                                                std::vector<int>& argToParam, std::vector<int>& paramToArg) {
    argToParam.assign(args.size(), -1);
    paramToArg.assign(params.size(), -1);

    for (size_t i = 0; i < args.size(); ++i) {
        auto& arg = args[i];
        if (arg.name.empty()) {
            auto unassigned = llvm::find(paramToArg, -1);
            if (unassigned == paramToArg.end()) {
                if (isVariadic) {
                    argToParam[i] = -1;
                } else {
                    return ArgumentValidation::tooMany();
                }
            } else {
                size_t paramIndex = size_t(unassigned - paramToArg.begin());
                argToParam[i] = int(paramIndex);
                paramToArg[paramIndex] = int(i);
            }
        } else {
            int paramIndex = -1;
            for (size_t j = 0; j < params.size(); ++j) {
                if (!params[j].getName().empty() && params[j].getName() == arg.name) {
                    paramIndex = int(j);
                    break;
                }
            }
            if (paramIndex == -1) return ArgumentValidation::invalidName(i);
            if (paramToArg[size_t(paramIndex)] != -1) return ArgumentValidation::duplicateName(i);
            argToParam[i] = paramIndex;
            paramToArg[size_t(paramIndex)] = int(i);
        }
    }

    for (size_t j = 0; j < params.size(); ++j) {
        if (paramToArg[j] == -1 && !params[j].defaultValue) return ArgumentValidation::tooFew();
    }
    return std::nullopt;
}

Type Typechecker::typecheckVarExpr(VarExpr& expr, bool useIsWriteOnly, Type expectedType) {
    if (findDecls(expr.identifier).empty()) {
        if (auto* enumCase = getExpectedEnumCase(expr.identifier, expectedType)) {
            MemberExpr qualified(makeAST<VarExpr>(enumCase->getEnumDecl()->getName(), expr.location), expr.identifier, expr.location);
            if (auto* resolvedCase = getEnumCase(qualified, expectedType)) {
                checkHasAccess(*resolvedCase->getEnumDecl(), expr.location, AccessLevel::None);
                expr.decl = resolvedCase;
                return resolvedCase->type;
            }
        }
    }

    auto* decl = findDecl(expr.identifier, expr.location, expr.endLocation);
    checkHasAccess(*decl, expr.location, AccessLevel::None);
    decl->referenced = true;
    expr.decl = decl;

    if (auto variableDecl = llvm::dyn_cast<VariableDecl>(decl)) {
        maybeCaptureVariable(*variableDecl);
    }

    switch (decl->kind) {
    case DeclKind::VarDecl: {
        auto* varDecl = llvm::cast<VarDecl>(decl);
        if (!useIsWriteOnly) checkNotMoved(*decl, expr);
        if (!useIsWriteOnly && !varDecl->isGlobal() && !varDecl->initializer && !definitelyAssignedDecls.count(decl)) {
            ERROR_RANGE(expr.location, expr.endLocation, "use of uninitialized variable '" << expr.identifier << "'");
        }
        if (!useIsWriteOnly) {
            if (auto narrowed = narrowedTypes.find(decl); narrowed != narrowedTypes.end()) return narrowed->second;
        }
        return varDecl->type;
    }
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
        ERROR_RANGE(expr.location, expr.endLocation, "cannot refer to generic parameter '" << expr.identifier << "' as a value");
    case DeclKind::ConstructorDecl:
        ERROR_RANGE(expr.location, expr.endLocation, "cannot refer to constructor '" << expr.identifier << "' as a value");
    case DeclKind::DestructorDecl:
        ERROR_RANGE(expr.location, expr.endLocation, "cannot refer to destructor '" << expr.identifier << "' as a value");
    case DeclKind::FunctionTemplate:
        ERROR_RANGE(expr.location, expr.endLocation, "cannot refer to generic function '" << expr.identifier << "' without specifying type arguments");
    case DeclKind::TypeDecl:
        return llvm::cast<TypeDecl>(decl)->getType();
    case DeclKind::TypeTemplate:
        ERROR_RANGE(expr.location, expr.endLocation, "'" << expr.identifier << "' is not a variable");
    case DeclKind::EnumDecl:
        ERROR_RANGE(expr.location, expr.endLocation, "'" << expr.identifier << "' is not a variable");
    case DeclKind::EnumCase:
        return llvm::cast<EnumCase>(decl)->type;
    case DeclKind::FieldDecl: {
        if (currentFunction && currentFunction->isLambda()) {
            maybeCaptureVariable(*llvm::cast<VariableDecl>(findDecl("this", expr.location)));
        }
        return llvm::cast<FieldDecl>(decl)->type;
    }
    case DeclKind::ImportDecl:
        ERROR_RANGE(expr.location, expr.endLocation, "cannot refer to import '" << expr.identifier << "' as a value");
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
    ERROR_RANGE(expr.location, expr.endLocation, "integer literal is too large");
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
            Type unwrapped = expectedType;
            while (unwrapped.isOptionalType()) {
                unwrapped = unwrapped.getWrappedType();
            }
            if (unwrapped.isArrayType() || unwrapped.isSlice()) {
                return expectedType;
            }
        }
        ERROR(array.location, "couldn't infer type of empty array literal");
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

Type Typechecker::typecheckAnonymousStructExpr(AnonymousStructExpr& expr) {
    auto elements = map(expr.elements, [&](const NamedValue& namedValue) {
        if (namedValue.name.empty()) {
            ERROR(namedValue.location, "unnamed anonymous struct members are not supported yet; name each field (e.g. `(x = 1, y = 2)`)");
        }
        return AnonymousStructElement{namedValue.name, typecheckExpr(*namedValue.value)};
    });
    return AnonymousStructType::get(std::move(elements));
}

void Typechecker::typecheckImplicitlyBoolConvertibleExpr(Type type, Location location, Location endLocation, bool positive) {
    if (!type.removePointer().isBool() && !type.removePointer().isOptionalType()) {
        if (type.isImplementedAsPointer()) {
            WARN_RANGE(location, endLocation,
                       "type '" << type << "' " << (positive ? "is always non-null" : "cannot be null") << "; to declare it nullable, use '"
                                << OptionalType::get(type) << "'");
        } else {
            ERROR_RANGE(location, endLocation, "type '" << type << "' is not convertible to boolean");
        }
    }
}

Type Typechecker::typecheckUnaryExpr(UnaryExpr& expr) {
    Type operandType = typecheckExpr(expr.getOperand());

    switch (expr.op) {
    case Token::Not:
        typecheckImplicitlyBoolConvertibleExpr(operandType, expr.getOperand().location, expr.getOperand().endLocation, false);
        return Type::getBool();

    case Token::Star: // Dereference operation
        if (operandType.removeOptional().isPointerType()) {
            return operandType.removeOptional().getPointee();
        } else if (operandType.removeOptional().isUnsizedArrayPointer()) {
            return operandType.removeOptional().getElementType();
        }

        ERROR_RANGE(expr.location, expr.endLocation, "cannot dereference non-pointer type '" << operandType << "'");

    case Token::And: // Address-of operation
        unnarrow(expr.getOperand());
        operandType = expr.getOperand().type;
        // Allow forming mutable pointers to constants. This is safe because constants will be inlined at the usage site.
        if (expr.isConstant()) {
            operandType = operandType.withMutability(Mutability::Mutable);
        }
        // Taking the address of a borrow exposes the borrowed address; it never nests.
        return PointerType::get(operandType.removeReference());

    case Token::Increment:
        operandType = operandType.removePointer();

        if (!operandType.isMutable()) {
            ERROR_RANGE(expr.location, expr.endLocation, "cannot increment immutable value of type '" << operandType << "'");
        } else if (!operandType.isIncrementable()) {
            ERROR_RANGE(expr.location, expr.endLocation, "cannot increment '" << operandType << "'");
        }

        return Type::getVoid();

    case Token::Decrement:
        operandType = operandType.removePointer();

        if (!operandType.isMutable()) {
            ERROR_RANGE(expr.location, expr.endLocation, "cannot decrement immutable value of type '" << operandType << "'");
        } else if (!operandType.isDecrementable()) {
            ERROR_RANGE(expr.location, expr.endLocation, "cannot decrement '" << operandType << "'");
        }

        return Type::getVoid();

    default:
        return operandType;
    }
}

static bool checkRange(const Expr& expr, const llvm::APSInt& value, Type type, bool diagnoseOutOfRange);

// Suggests '*' when a comparison mixes a raw pointer with an integer constant of its
// pointee type. Only reachable when the builtin deref above didn't apply (e.g. optional
// or void pointers) or no overload matched, where spelling the deref explicitly may help.
static std::string mixedPointerOperandHint(const BinaryExpr& expr) {
    if (!isComparisonOperator(expr.op)) return "";

    const Expr &lhs = expr.getLHS(), &rhs = expr.getRHS();
    auto isRawPointer = [](Type type) { return type.removeOptional().isPointerType() && !type.removeOptional().isReferenceType(); };

    auto check = [&](const Expr& ptrSide, const Expr& valueSide) -> std::string {
        Type pointee = ptrSide.type.removeOptional().getPointee();
        if (!valueSide.isConstant() || (!valueSide.type.isInteger() && !valueSide.type.isChar()) || !pointee.isInteger()) return "";
        if (!checkRange(valueSide, valueSide.getConstantIntegerValue(), pointee, false)) return "";
        if (auto* var = llvm::dyn_cast<VarExpr>(&ptrSide)) {
            return (" (to compare the pointed-to value, dereference '*" + var->identifier + "')").str();
        }
        return " (to compare the pointed-to value, dereference the pointer operand with '*')";
    };

    if (isRawPointer(lhs.type) && !isRawPointer(rhs.type)) return check(lhs, rhs);
    if (isRawPointer(rhs.type) && !isRawPointer(lhs.type)) return check(rhs, lhs);
    return "";
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
        hint = mixedPointerOperandHint(expr);
    }

    ERROR_RANGE(expr.location, expr.endLocation,
                "invalid operands '" << expr.getLHS().type << "' and '" << expr.getRHS().type << "' to '" << toString(op) << "'" << hint);
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

EnumCase* cx::getIsEnumCase(Expr& expr) {
    if (auto* varExpr = llvm::dyn_cast<VarExpr>(&expr)) {
        return llvm::dyn_cast<EnumCase>(varExpr->decl);
    }
    if (auto* memberExpr = llvm::dyn_cast<MemberExpr>(&expr)) {
        return llvm::dyn_cast<EnumCase>(memberExpr->decl);
    }
    return nullptr;
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

    if (op == Token::QuestionQuestion) {
        return typecheckNullCoalescingExpr(expr);
    }

    if (op == Token::Is) {
        Type leftType = typecheckExpr(expr.getLHS());
        if (!leftType.isEnumType()) {
            ERROR(expr.getLHS().location, "left side of 'is' must be an enum, got '" << leftType << "'");
        }
        typecheckExpr(expr.getRHS(), false, leftType);
        auto* enumCase = getIsEnumCase(expr.getRHS());
        if (!enumCase || enumCase->getEnumDecl() != leftType.getDecl()) {
            ERROR(expr.getRHS().location, "right side of 'is' must be a case of enum '" << leftType << "'");
        }
        return Type::getBool();
    }

    if (op == Token::AndAnd || op == Token::OrOr) {
        Type leftType = typecheckExpr(expr.getLHS());
        auto outerNarrowings = narrowedTypes;
        auto afterLHSAssignedDecls = definitelyAssignedDecls;
        applyNarrowings(expr.getLHS(), op == Token::AndAnd);
        Type rightType = typecheckExpr(expr.getRHS(), false, leftType);
        // The right side may not execute (short-circuit), so only narrowings valid on both paths survive.
        intersectNarrowings(outerNarrowings);
        definitelyAssignedDecls = afterLHSAssignedDecls;
        // Like if conditions, operands may be optionals, testing for non-null.
        if ((leftType.isBool() || leftType.isOptionalType()) && (rightType.isBool() || rightType.isOptionalType())) {
            return Type::getBool();
        }
        if (!isBuiltinOp(op, leftType, rightType)) {
            return typecheckCallExpr(expr);
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

    if ((op == Token::Equal || op == Token::NotEqual) && leftType.isAnonymousStructType() && rightType.isAnonymousStructType()) {
        auto leftElements = leftType.getAnonymousStructElements();
        auto rightElements = rightType.getAnonymousStructElements();
        auto hasDistinctNames = [](llvm::ArrayRef<AnonymousStructElement> elements) {
            for (size_t i = 0; i < elements.size(); ++i) {
                for (size_t j = i + 1; j < elements.size(); ++j) {
                    if (elements[i].name == elements[j].name) return false;
                }
            }
            return true;
        };
        // Only comparable nominally: same arity, same distinct non-empty element names. The lowering below accesses elements by name.
        auto namesMatch = leftElements.size() == rightElements.size() && !leftElements.empty() && hasDistinctNames(leftElements)
                       && hasDistinctNames(rightElements) && llvm::all_of(llvm::zip_first(leftElements, rightElements), [](auto&& pair) {
                              auto&& [left, right] = pair;
                              return !left.name.empty() && left.name == right.name;
                          });

        if (namesMatch) {
            // Lower anonymous struct comparison to elementwise comparison (e.g. `(a == b) && (c == d)`).
            auto combiner = op == Token::Equal ? Token::AndAnd : Token::OrOr;
            // Bind each side to a compiler-generated temporary so operands with
            // side effects evaluate once; the element accesses below read the
            // temporaries. (`__`-prefixed identifiers are reserved for the
            // compiler, so these can't collide with user declarations.)
            // Outside functions there is no scope for temporaries, but global
            // initializers can only be constants, so comparing the operands
            // directly is harmless there.
            VarDecl* lhsTemp = nullptr;
            VarDecl* rhsTemp = nullptr;
            Expr* lhsBase = &expr.getLHS();
            Expr* rhsBase = &expr.getRHS();
            if (currentFunction) {
                static uint64_t anonymousStructTempCounter = 0;
                lhsTemp = makeAST<VarDecl>(leftType, "__anonymous_struct_lhs_" + std::to_string(anonymousStructTempCounter++), nullptr, currentFunction,
                                           AccessLevel::None, *currentModule, expr.location);
                rhsTemp = makeAST<VarDecl>(rightType, "__anonymous_struct_rhs_" + std::to_string(anonymousStructTempCounter++), nullptr, currentFunction,
                                           AccessLevel::None, *currentModule, expr.location);
                typecheckVarDecl(*lhsTemp);
                typecheckVarDecl(*rhsTemp);
                // The temporaries have no initializer; codegen binds them to
                // the operand values before emitting the lowering.
                definitelyAssignedDecls.insert(lhsTemp);
                definitelyAssignedDecls.insert(rhsTemp);
                lhsBase = makeAST<VarExpr>(lhsTemp->getName(), expr.location);
                rhsBase = makeAST<VarExpr>(rhsTemp->getName(), expr.location);
            }
            Expr* result = nullptr;
            for (size_t i = 0; i < leftElements.size(); ++i) {
                auto* comparison = makeAST<BinaryExpr>(op, makeAST<MemberExpr>(lhsBase, leftElements[i].name, expr.location),
                                                       makeAST<MemberExpr>(rhsBase, rightElements[i].name, expr.location), expr.location);
                result = result ? makeAST<BinaryExpr>(combiner, result, comparison, expr.location) : comparison;
            }
            ASSERT(result);
            if (!currentFunction) {
                expr = llvm::cast<BinaryExpr>(*result);
                return typecheckBinaryExpr(expr);
            }
            Type loweredType = typecheckBinaryExpr(llvm::cast<BinaryExpr>(*result));
            ASSERT(loweredType.isBool());
            expr.anonymousStructTempLHS = lhsTemp;
            expr.anonymousStructTempRHS = rhsTemp;
            expr.anonymousStructComparisonLowering = result;
            return Type::getBool();
        }
    }

    if (!isBuiltinOp(op, leftType, rightType)) {
        return typecheckCallExpr(expr);
    }

    // Operators auto-deref: unless both operands are raw pointers (an address comparison),
    // deref any pointer or borrow operand so the operation applies to the pointed-to values.
    // Derefing up front lets the conversions below handle inexact matches, e.g. 'uint*' against
    // an 'int' constant. Optional and unsized-array-pointer operands are excluded: they keep
    // the conversions below (null-aware identity, pointer-to-array decay). Null literals are
    // excluded so 'p == null' still reports the pointer type.
    bool bothRawPointers = leftType.isPointerType() && rightType.isPointerType() && !leftType.isReferenceType() && !rightType.isReferenceType();
    bool eitherSpecial = leftType.isOptionalType() || rightType.isOptionalType() || leftType.isUnsizedArrayPointer() || rightType.isUnsizedArrayPointer();
    if (!bothRawPointers && !eitherSpecial) {
        if (leftType.isPointerType() && !leftType.getPointee().isVoid() && !expr.getRHS().isNullLiteralExpr()) {
            expr.setLHS(makeAST<ImplicitCastExpr>(&expr.getLHS(), leftType.getPointee(), ImplicitCastExpr::AutoDereference));
            leftType = leftType.getPointee();
        }
        if (rightType.isPointerType() && !rightType.getPointee().isVoid() && !expr.getLHS().isNullLiteralExpr()) {
            expr.setRHS(makeAST<ImplicitCastExpr>(&expr.getRHS(), rightType.getPointee(), ImplicitCastExpr::AutoDereference));
            rightType = rightType.getPointee();
        }
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
            ERROR_RANGE(expr.location, expr.endLocation, "comparison of distinct pointer types ('" << leftType << "' and '" << rightType << "')");
        }
    } else if (isBitwiseOperator(op) && (leftType.isFloatingPoint() || rightType.isFloatingPoint())) {
        throwInvalidOperandsToBinaryExpr(expr, op);
    } else if (leftType.isVoid() || rightType.isVoid()) {
        throwInvalidOperandsToBinaryExpr(expr, op);
    } else if (auto convertedRHS = convert(&expr.getRHS(), leftType, true, false)) {
        expr.setRHS(convertedRHS);
    } else if (auto convertedLHS = convert(&expr.getLHS(), rightType, true, false)) {
        expr.setLHS(convertedLHS);
    } else if (!leftType.removeOptional().isPointerType() || !rightType.removeOptional().isPointerType()) {
        throwInvalidOperandsToBinaryExpr(expr, op);
    }

    Type resultType = isComparisonOperator(op) ? Type::getBool() : expr.getLHS().type.removeOptional().removePointer();

    if ((op == Token::Plus || op == Token::Minus || op == Token::Star) && (resultType.isInteger() || resultType.isInt128() || resultType.isUInt128())
        && expr.isConstant()) {
        // Like the runtime overflow check, diagnose overflowing constant arithmetic at compile time.
        checkRange(expr, expr.getConstantIntegerValue(), resultType, /* diagnoseOutOfRange: */ true);
    }

    return resultType;
}

// Walks member and index bases to the underlying variable, if any.
static VarExpr* getAssignmentBaseVarExpr(Expr& lhs) {
    Expr* current = &lhs;
    while (true) {
        if (auto* varExpr = llvm::dyn_cast<VarExpr>(current)) return varExpr;
        if (auto* memberExpr = llvm::dyn_cast<MemberExpr>(current)) {
            current = memberExpr->base;
            continue;
        }
        if (auto* indexExpr = llvm::dyn_cast<IndexExpr>(current)) {
            current = indexExpr->getBase();
            continue;
        }
        if (auto* indexAssignExpr = llvm::dyn_cast<IndexAssignmentExpr>(current)) {
            current = indexAssignExpr->getBase();
            continue;
        }
        return nullptr;
    }
}

void Typechecker::typecheckAssignment(BinaryExpr& expr, Location location) {
    auto* lhs = &expr.getLHS();
    auto* rhs = &expr.getRHS();

    typecheckExpr(*lhs, true);
    if (!lhs->isLvalue()) {
        ERROR(lhs->location, "cannot assign to expression of type '" << lhs->type << "'");
    }
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
        ERROR(location, "cannot assign '" << rhsType << "' to '" << lhsType << "'" << narrowingHint(rhsType, lhsType));
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

    if (auto* varExpr = llvm::dyn_cast<VarExpr>(lhs)) {
        expr.lhsIsMoved = movedDecls.count(varExpr->decl);
    }
    if (auto* baseVarExpr = getAssignmentBaseVarExpr(*lhs)) {
        if (baseVarExpr->decl->isVarDecl()) {
            definitelyAssignedDecls.insert(baseVarExpr->decl);
        }
    }

    if (!rhsType.removeReference().isImplicitlyCopyable() && !lhsType.removeOptional().isPointerType()) {
        setMoved(rhs, true);
        setMoved(lhs, false);
    }

    if (currentInitializedFields) {
        if (auto fieldDecl = lhs->getFieldDecl()) {
            currentInitializedFields->insert(fieldDecl);
        }
    }
}

static bool checkRange(const Expr& expr, const llvm::APSInt& value, Type type, bool diagnoseOutOfRange) {
    int width;
    bool isUnsigned;
    if (type.isInteger()) {
        width = type.getIntegerBitWidth();
        isUnsigned = type.isUnsigned();
    } else if (type.isInt128() || type.isUInt128()) {
        width = 128;
        isUnsigned = type.isUInt128();
    } else {
        llvm_unreachable("checkRange only supports integer types");
    }
    if (llvm::APSInt::compareValues(value, llvm::APSInt::getMinValue(width, isUnsigned)) < 0
        || llvm::APSInt::compareValues(value, llvm::APSInt::getMaxValue(width, isUnsigned)) > 0) {
        if (!diagnoseOutOfRange) return false;
        ERROR_RANGE(expr.location, expr.endLocation, value << " is out of range for type '" << type << "'");
    }
    return true;
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

Expr* Typechecker::convert(Expr* expr, Type type, bool allowPointerToTemporary, bool diagnoseOutOfRange) const {
    std::optional<ImplicitCastExpr::Kind> implicitCastKind;
    if (Type convertedType = isImplicitlyConvertible(expr, expr->type, type, allowPointerToTemporary, &implicitCastKind, diagnoseOutOfRange)) {
        if (implicitCastKind) {
            if (*implicitCastKind == ImplicitCastExpr::OptionalWrap && expr->type != convertedType.getWrappedType()) {
                // One wrap node constructs a single level, so convert the operand to the wrapped
                // type first (e.g. `int` to `int?` when wrapping to `int??`). Each recursion
                // strips one optional level, so this terminates.
                expr = convert(expr, convertedType.getWrappedType(), allowPointerToTemporary, diagnoseOutOfRange);
                if (!expr) return nullptr;
            }
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
                if (Expr* convertedThen = convert(ifExpr->thenExpr, convertedType, allowPointerToTemporary)) {
                    ifExpr->thenExpr = convertedThen;
                }
                if (Expr* convertedElse = convert(ifExpr->elseExpr, convertedType, allowPointerToTemporary)) {
                    ifExpr->elseExpr = convertedElse;
                }
            }

            if (auto* arrayLiteral = llvm::dyn_cast<ArrayLiteralExpr>(expr); arrayLiteral && convertedType.isConstantArray()) {
                for (auto& element : arrayLiteral->elements) {
                    if (Expr* convertedElement = convert(element, convertedType.getElementType(), allowPointerToTemporary)) {
                        element = convertedElement;
                    }
                }
            }

            if (auto* anonymousStructExpr = llvm::dyn_cast<AnonymousStructExpr>(expr); anonymousStructExpr && convertedType.isAnonymousStructType()) {
                auto targetElements = convertedType.getAnonymousStructElements();
                for (size_t i = 0; i < anonymousStructExpr->elements.size(); ++i) {
                    if (Expr* convertedElement = convert(anonymousStructExpr->elements[i].value, targetElements[i].type, allowPointerToTemporary)) {
                        anonymousStructExpr->elements[i].value = convertedElement;
                    }
                }
            }
        }
        return expr;
    }
    return nullptr;
}

static int getFloatBitWidth(Type type) {
    if (type.isFloat64()) return 64;
    if (type.isFloat80()) return 80;
    return 32; // float and float32
}

// True when every value of the source numeric type is exactly representable in the target.
static bool isSafeNumericWidening(Type source, Type target) {
    auto width = [](Type type) { return type.isChar() ? 8 : type.getIntegerBitWidth(); };
    auto isSigned = [](Type type) { return type.isInteger() && type.isSigned(); }; // char zero-extends

    if ((source.isInteger() || source.isChar()) && target.isInteger()) {
        if (width(target) < width(source)) return false;
        if (isSigned(source) && !isSigned(target)) return false;
        if (width(target) == width(source) && isSigned(source) != isSigned(target)) return false;
        return true;
    }

    if ((source.isInteger() || source.isChar()) && target.isFloatingPoint()) {
        // Largest integer width exactly representable in the mantissa.
        int maxWidth = target.isFloat64() ? 32 : target.isFloat80() ? 64 : 16;
        return width(source) <= maxWidth;
    }

    if (source.isFloatingPoint() && target.isFloatingPoint()) {
        return getFloatBitWidth(target) >= getFloatBitWidth(source);
    }

    return false;
}

// True when viewing source bits as target needs no conversion: the types must match
// exactly, except that const may be added (nothing can be written through a const
// target, so narrowing the uses is safe). Anything else, even representation-preserving
// widening like `int*` to `int*?`, would let writes through the reinterpreted pointer
// break the source invariant, so pointers to it can't be reinterpreted.
static bool isReinterpretible(Type source, Type target) {
    return source == target || (!target.isMutable() && source.equalsIgnoreTopLevelMutable(target));
}

Type Typechecker::isImplicitlyConvertible(const Expr* expr, Type source, Type target, bool allowPointerToTemporary,
                                          std::optional<ImplicitCastExpr::Kind>* implicitCastKind, bool diagnoseOutOfRange) const {
    if (source.isBasicType() && target.isBasicType() && source.getName() == target.getName() && source.getGenericArgs() == target.getGenericArgs()) {
        return source;
    }

    if (source.isArrayType() && (target.isArrayType() || target.isSlice()) && source.getElementType() == target.getElementType()) {
        if (target.isArrayType() && source.getArraySize() == target.getArraySize()) return source;
        if (source.isConstantArray() && (target.isUnsizedArrayPointer() || target.isSlice())) return source;
    }

    if (source.isBasicType() && target.isSlice() && source.getName() == "List" && source.getGenericArgs() == target.getGenericArgs()) {
        return source;
    }

    if (source.isBasicType() && source.getName() == "StringBuffer" && target.isBasicType() && target.getName() == "string") {
        return source;
    }

    if (source.isAnonymousStructType() && target.isAnonymousStructType() && source.getAnonymousStructElements() == target.getAnonymousStructElements()) {
        return source;
    }

    if (source.isFunctionType() && target.isFunctionType() && source.getReturnType() == target.getReturnType()
        && source.getParamTypes() == target.getParamTypes()) {
        return source;
    }

    if (source.isPointerType() && target.isPointerType() && source.isReferenceType() == target.isReferenceType()
        && (source.getPointee().isMutable() || !target.getPointee().isMutable())
        && (isReinterpretible(source.getPointee(), target.getPointee()) || target.getPointee().isVoid())) {
        return source;
    }

    if (source.isOptionalType() && target.isOptionalType() && (source.getWrappedType().isMutable() || !target.getWrappedType().isMutable())) {
        // Only a no-op when the wrapped conversion needs no cast; nesting changes (e.g. `int?` to `int??`)
        // fall through to the wrap rule below.
        std::optional<ImplicitCastExpr::Kind> wrappedCastKind;
        if (isImplicitlyConvertible(nullptr, source.getWrappedType(), target.getWrappedType(), false, &wrappedCastKind, diagnoseOutOfRange)
            && !wrappedCastKind) {
            return source;
        }
    }

    if (expr) {
        // Only tag-only enums convert to their tag type; payload enums (including optionals) don't.
        if (expr->type.isEnumType() && !llvm::cast<EnumDecl>(expr->type.getDecl())->hasAssociatedValues()
            && llvm::cast<EnumDecl>(expr->type.getDecl())->getTagType() == target) {
            return source;
        }

        if (auto* ifExpr = llvm::dyn_cast<IfExpr>(expr)) {
            if (isImplicitlyConvertible(ifExpr->thenExpr, ifExpr->thenExpr->type, target, false, nullptr, diagnoseOutOfRange)
                && isImplicitlyConvertible(ifExpr->elseExpr, ifExpr->elseExpr->type, target, false, nullptr, diagnoseOutOfRange)) {
                return target;
            }
        }

        // Auto-cast integer constants to target type if within range, error out if not within range.
        if ((expr->type.isInteger() || expr->type.isChar() || (expr->type.isEnumType() && !llvm::cast<EnumDecl>(expr->type.getDecl())->hasAssociatedValues()))
            && expr->isConstant()) {
            auto value = expr->getConstantIntegerValue();
            // Convert e.g. int literal to uint when binding to uint&; raw pointer parameters require an explicit '&'.
            auto adjustedTarget = allowPointerToTemporary ? target.removeReference() : target;

            if (adjustedTarget.isInteger()) {
                if (!checkRange(*expr, value, adjustedTarget, diagnoseOutOfRange)) return Type();
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
        if (expr->isStringLiteralExpr() && !target.removeOptional().isReferenceType()
            && ((target.removeOptional().isPointerType() && target.removeOptional().getPointee().isChar() && !target.removeOptional().getPointee().isMutable())
                || (target.removeOptional().isUnsizedArrayPointer() && target.removeOptional().getElementType().isChar()
                    && !target.removeOptional().getElementType().isMutable()))) {
            return target;
        }

        if (expr->isArrayLiteralExpr() && target.isConstantArray()) {
            auto arrayLiteralExpr = llvm::cast<ArrayLiteralExpr>(expr);
            bool isConvertible = llvm::all_of(arrayLiteralExpr->elements, [&](Expr* element) {
                return isImplicitlyConvertible(element, source.getElementType(), target.getElementType(), false, nullptr, diagnoseOutOfRange);
            });

            if (isConvertible) {
                return target;
            }
        }
    }

    // Safe numeric widening applies to values; constants are folded by the rule above.
    if (isSafeNumericWidening(source, target)) {
        if (implicitCastKind) *implicitCastKind = ImplicitCastExpr::NumericWiden;
        return target;
    }

    // Bind values to borrow parameters implicitly. Borrowing never copies, moves, or stores, and requires
    // an exact type match: the backend passes the operand's address as is, so no representation change
    // (wrapping, view conversion) may happen underneath the borrow. Raw pointer parameters require '&'.
    if ((allowPointerToTemporary || (expr && expr->isLvalue())) && target.isReferenceType() &&
        // Allow forming mutable borrows of constants. This is safe because constants will be inlined at the usage site.
        (source.isMutable() || (expr && expr->isConstant()) || !target.getPointee().isMutable()) && source.equalsIgnoreTopLevelMutable(target.getPointee())) {
        if (implicitCastKind) *implicitCastKind = ImplicitCastExpr::AutoReference;
        return source;
    }

    // Reborrow a pointer as a borrow of its pointee. The address passes through unchanged;
    // this only reinterprets the static type, so borrows keep working where pointers flow.
    if (target.isReferenceType() && source.isPointerType() && !source.isReferenceType() && (source.getPointee().isMutable() || !target.getPointee().isMutable())
        && source.getPointee().equalsIgnoreTopLevelMutable(target.getPointee())) {
        if (implicitCastKind) *implicitCastKind = ImplicitCastExpr::Reborrow;
        return target;
    }

    // Reborrow an optional pointer as an optional borrow. Like Reborrow, the address passes
    // through unchanged with null staying null; this only reinterprets the static type.
    if (target.isOptionalType() && target.getWrappedType().isReferenceType() && source.isOptionalType() && source.getWrappedType().isPointerType()
        && !source.getWrappedType().isReferenceType() && (source.getWrappedType().getPointee().isMutable() || !target.getWrappedType().getPointee().isMutable())
        && source.getWrappedType().getPointee().equalsIgnoreTopLevelMutable(target.getWrappedType().getPointee())) {
        if (implicitCastKind) *implicitCastKind = ImplicitCastExpr::Reborrow;
        return target;
    }

    if (source.isPointerType() && source.getPointee() == target && expr && !expr->isReferenceExpr() && target.isImplicitlyCopyable()) {
        // Implicit dereference copies the pointee; moving out of a pointer
        // requires an explicit '*' so moves are visible at the use site.
        if (implicitCastKind) *implicitCastKind = ImplicitCastExpr::AutoDereference;
        return target;
    }

    if (target.isOptionalType() && (!expr || !expr->isNullLiteralExpr())
        && isImplicitlyConvertible(expr, source, target.getWrappedType(), allowPointerToTemporary, nullptr, diagnoseOutOfRange)) {
        if (implicitCastKind) *implicitCastKind = ImplicitCastExpr::OptionalWrap;
        return target;
    }

    // Calls returning optionals implicitly unwrap like any other expression; the null
    // analyzer warns unless the unwrapped value is proven non-null at the use site.
    if (source.isOptionalType() && source.getWrappedType() == target && expr) {
        if (implicitCastKind) *implicitCastKind = ImplicitCastExpr::OptionalUnwrap;
        return target;
    }

    if (source.isArrayType() && target.removeOptional().isPointerType() && isReinterpretible(source.getElementType(), target.removeOptional().getPointee())) {
        return source;
    }

    if (source.isPointerType() && source.getPointee().isConstantArray() && (target.isSlice() || target.isUnsizedArrayPointer())
        && source.getPointee().getElementType() == target.getElementType()) {
        return source;
    }

    if (source.isPointerType() && source.getPointee().isArrayType() && target.removeOptional().isPointerType()
        && isReinterpretible(source.getPointee().getElementType(), target.removeOptional().getPointee())) {
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

    if (source.isAnonymousStructType() && target.isAnonymousStructType()) {
        auto* anonymousStructExpr = llvm::dyn_cast_or_null<AnonymousStructExpr>(expr);
        auto sourceElements = source.getAnonymousStructElements();
        auto targetElements = target.getAnonymousStructElements();

        if (sourceElements.size() != targetElements.size()) {
            return Type();
        }

        for (size_t i = 0; i < sourceElements.size(); ++i) {
            if (sourceElements[i].name != targetElements[i].name) {
                return Type();
            }

            auto* elementValue = anonymousStructExpr ? anonymousStructExpr->elements[i].value : nullptr;

            if (!isImplicitlyConvertible(elementValue, sourceElements[i].type, targetElements[i].type, false, nullptr, diagnoseOutOfRange)) {
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
        for (GenericArg genericArg : type.getGenericArgs()) {
            if (genericArg.isType() && containsGenericParam(genericArg.type, genericParam)) {
                return true;
            }
        }
        return type.getName() == genericParam;

    case TypeKind::ArrayType:
        return type.getArraySizeParam() == genericParam || containsGenericParam(type.getElementType(), genericParam);

    case TypeKind::AnonymousStructType:
        return llvm::any_of(type.getAnonymousStructElements(),
                            [&](const AnonymousStructElement& element) { return containsGenericParam(element.type, genericParam); });

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

GenericArg Typechecker::findGenericArg(Type argType, Type paramType, llvm::StringRef genericParam, bool inFunctionType) {
    if (!inFunctionType && argType.isReferenceType() && !paramType.isReferenceType()) {
        // A bare borrow binds as its referent, so borrowing never leaks into inferred value types.
        // Inside function types the borrow is part of the callee's ABI and must be preserved.
        argType = argType.getPointee();
    }

    if (paramType.isBasicType() && paramType.getName() == genericParam) {
        return GenericArg(argType);
    }

    if (argType.isClosureType()) {
        // Infer from the user-visible signature; argument conversion still rejects
        // capturing lambdas where plain function pointers are expected.
        auto closureParams = argType.getClosureParamTypes();
        std::vector<Type> paramTypes(closureParams.begin(), closureParams.end());
        return findGenericArg(FunctionType::get(argType.getClosureReturnType(), std::move(paramTypes), false), paramType, genericParam, inFunctionType);
    }

    switch (argType.getKind()) {
    case TypeKind::BasicType:
        if (!argType.getGenericArgs().empty() && paramType.isBasicType() && paramType.getName() == argType.getName()) {
            ASSERT(argType.getGenericArgs().size() == paramType.getGenericArgs().size());
            for (auto&& [argTypeGenericArg, paramTypeGenericArg] : llvm::zip_first(argType.getGenericArgs(), paramType.getGenericArgs())) {
                if (paramTypeGenericArg.isType() && paramTypeGenericArg.type.isBasicType() && paramTypeGenericArg.type.getName() == genericParam) {
                    return argTypeGenericArg;
                }
                if (argTypeGenericArg.isType() && paramTypeGenericArg.isType()) {
                    if (GenericArg arg = findGenericArg(argTypeGenericArg.type, paramTypeGenericArg.type, genericParam, inFunctionType)) {
                        return arg;
                    }
                }
            }
        }
        break;

    case TypeKind::ArrayType:
        if (paramType.isArrayType()) {
            if (paramType.getArraySizeParam() == genericParam && argType.isConstantArray()) {
                return GenericArg::fromInt(argType.getArraySize(), argType.location);
            }
            return findGenericArg(argType.getElementType(), paramType.getElementType(), genericParam, inFunctionType);
        }
        break;

    case TypeKind::AnonymousStructType:
        if (paramType.isAnonymousStructType()) {
            for (auto&& [argTypeElement, paramTypeElement] : llvm::zip_first(argType.getAnonymousStructElements(), paramType.getAnonymousStructElements())) {
                if (GenericArg arg = findGenericArg(argTypeElement.type, paramTypeElement.type, genericParam, inFunctionType)) {
                    return arg;
                }
            }
        }
        break;

    case TypeKind::FunctionType:
        if (paramType.isFunctionType()) {
            for (auto&& [argTypeParamType, paramTypeParamTypes] : llvm::zip_first(argType.getParamTypes(), paramType.getParamTypes())) {
                if (GenericArg arg = findGenericArg(argTypeParamType, paramTypeParamTypes, genericParam, true)) {
                    return arg;
                }
            }
            return findGenericArg(argType.getReturnType(), paramType.getReturnType(), genericParam, true);
        }
        break;

    case TypeKind::PointerType:
        if (paramType.isPointerType()) {
            return findGenericArg(argType.getPointee(), paramType.getPointee(), genericParam, inFunctionType);
        }
        break;

    case TypeKind::UnresolvedType:
        llvm_unreachable("invalid unresolved type");
    }

    // TODO: Should probably try matching generic arg also with implicitly-converted values, instead duplicating the special cases here. This is bug prone.

    if (paramType.removeOptional().isPointerType()) {
        return findGenericArg(argType, paramType.removeOptional().getPointee(), genericParam, inFunctionType);
    }

    if (paramType.isSlice() && argType.removeOptional().removePointer().isArrayType()) {
        return findGenericArg(argType.removeOptional().removePointer().getElementType(), paramType.getElementType(), genericParam, inFunctionType);
    }

    return GenericArg();
}

static Type replaceUnresolvedGenericParamsWithPlaceholders(Type type, llvm::ArrayRef<GenericParamDecl> genericParams) {
    llvm::StringMap<GenericArg> placeholders;

    for (auto& genericParam : genericParams) {
        placeholders.try_emplace(genericParam.getName(), UnresolvedType::get());
    }

    return type.resolve(placeholders);
}

static bool containsUnresolvedType(Type type) {
    switch (type.getKind()) {
    case TypeKind::BasicType:
        for (GenericArg genericArg : type.getGenericArgs()) {
            if (genericArg.isType() && containsUnresolvedType(genericArg.type)) {
                return true;
            }
        }
        return false;

    case TypeKind::ArrayType:
        if (!type.getArraySizeParam().empty()) return true;
        return containsUnresolvedType(type.getElementType());

    case TypeKind::AnonymousStructType:
        for (auto& element : type.getAnonymousStructElements()) {
            if (containsUnresolvedType(element.type)) {
                return true;
            }
        }
        return false;

    case TypeKind::FunctionType:
        for (Type paramType : type.getParamTypes()) {
            if (containsUnresolvedType(paramType)) {
                return true;
            }
        }
        return containsUnresolvedType(type.getReturnType());

    case TypeKind::PointerType:
        return containsUnresolvedType(type.getPointee());

    case TypeKind::UnresolvedType:
        return true;
    }

    llvm_unreachable("all cases handled");
}

// True when the argument is a lambda with an uninferred parameter type that the expected type
// can't provide yet. Typechecking it now would pass unresolved placeholder types to the lambda's
// parameters; the argument is skipped until more generic arguments are known.
static bool isLambdaAwaitingInference(const Expr& arg, Type expectedType) {
    auto* lambda = llvm::dyn_cast<LambdaExpr>(&arg);
    if (!lambda || arg.hasType() || !expectedType.isFunctionType()) return false;

    auto params = lambda->functionDecl->getParams();
    auto expectedParamTypes = expectedType.getParamTypes();

    for (size_t i = 0; i < params.size() && i < expectedParamTypes.size(); ++i) {
        if (!params[i].type && containsUnresolvedType(expectedParamTypes[i])) {
            return true;
        }
    }

    return false;
}

std::vector<GenericArg> Typechecker::inferGenericArgsFromCallArgs(llvm::ArrayRef<GenericParamDecl> genericParams, CallExpr& call,
                                                                  llvm::ArrayRef<ParamDecl> params, bool returnOnError) {
    std::vector<int> argToParam, paramToArg;
    auto mappingError = computeArgParamMapping(call.args, params, false, argToParam, paramToArg);
    bool useMapping = !mappingError;
    if (mappingError) {
        if (returnOnError) return {};
        if (mappingError->error == ArgumentValidation::TooFew || mappingError->error == ArgumentValidation::TooMany) return {};
        if (call.args.size() > params.size()) return {};
        for (size_t i = call.args.size(); i < params.size(); ++i) {
            if (!params[i].defaultValue) return {};
        }
    }

    std::vector<GenericArg> inferredGenericArgs;
    llvm::StringMap<GenericArg> inferredArgsByName;

    for (auto& genericParam : genericParams) {
        GenericArg genericArg;
        Expr* genericArgValue = nullptr;

        for (size_t i = 0; i < call.args.size(); ++i) {
            auto& arg = call.args[i];
            Type paramType = useMapping ? params[size_t(argToParam[i])].type : params[i].type;

            if (containsGenericParam(paramType, genericParam.getName())) {
                // FIXME: The args will also be typechecked by validateAndConvertArguments() after this function. Get rid of this duplicated typechecking.
                auto* argValue = arg.value;
                // Substitute arguments inferred so far so lambdas get concrete expected parameter types when possible.
                auto knownArgs = inferredArgsByName;
                if (genericArg) knownArgs[genericParam.getName()] = genericArg;
                auto expectedType = replaceUnresolvedGenericParamsWithPlaceholders(paramType.resolve(knownArgs), genericParams);
                if (isLambdaAwaitingInference(*argValue, expectedType)) continue;
                // TODO: Should probably not typecheck here because it might change the expression's type?
                Type argType = argValue->hasType() ? argValue->type : typecheckExpr(*argValue, false, expectedType);
                GenericArg maybeGenericArg = findGenericArg(argType, paramType, genericParam.getName());
                if (!maybeGenericArg) continue;

                if (!genericArg) {
                    genericArg = maybeGenericArg;
                    genericArgValue = argValue;
                } else {
                    // Agreeing arguments need no convertibility check, which can't handle parameter types
                    // that still mention other uninferred generic parameters. Convertibility is rechecked
                    // with concrete types after inference.
                    if (maybeGenericArg == genericArg) continue;

                    Type paramTypeWithGenericArg = paramType.resolve({{genericParam.getName(), genericArg}});
                    Type paramTypeWithMaybeGenericArg = paramType.resolve({{genericParam.getName(), maybeGenericArg}});

                    if (isImplicitlyConvertible(argValue, argValue->type, paramTypeWithGenericArg, true, nullptr, false)) {
                        continue;
                    } else if (isImplicitlyConvertible(genericArgValue, genericArgValue->type, paramTypeWithMaybeGenericArg, true, nullptr, false)) {
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
            inferredArgsByName[genericParam.getName()] = genericArg;
        } else {
            return {};
        }
    }

    ASSERT(genericParams.size() == inferredGenericArgs.size());

    for (auto&& [genericParam, genericArg] : llvm::zip(genericParams, inferredGenericArgs)) {
        if (genericParam.isValueParam) continue;
        if (!genericParam.constraints.empty()) {
            ASSERT(genericParam.constraints.size() == 1, "cannot have multiple generic constraints yet");
            auto* interface = getTypeDecl(*llvm::cast<BasicType>(genericParam.constraints[0].typeBase));

            if (auto basicType = llvm::dyn_cast<BasicType>(genericArg.type.typeBase)) {
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

    std::vector<int> fixedArgForParam(fixedParams.size(), -1);
    std::vector<size_t> packArgIndices;
    size_t packCount = 0;

    bool hasNamed = llvm::any_of(call.args, [](auto& arg) { return !arg.name.empty(); });
    if (!hasNamed) {
        if (call.args.size() < fixedParams.size()) return std::nullopt;
        packCount = call.args.size() - fixedParams.size();
        for (size_t j = 0; j < fixedParams.size(); ++j)
            fixedArgForParam[j] = int(j);
        for (size_t j = 0; j < packCount; ++j)
            packArgIndices.push_back(fixedParams.size() + j);
    } else {
        std::vector<int> namedFixedToArg(fixedParams.size(), -1);
        bool mappingFailed = false;
        for (size_t i = 0; i < call.args.size() && !mappingFailed; ++i) {
            if (call.args[i].name.empty()) continue;
            int paramIndex = -1;
            for (size_t j = 0; j < fixedParams.size(); ++j) {
                if (!fixedParams[j].getName().empty() && fixedParams[j].getName() == call.args[i].name) {
                    paramIndex = int(j);
                    break;
                }
            }
            if (paramIndex == -1 || namedFixedToArg[size_t(paramIndex)] != -1) {
                mappingFailed = true;
            } else {
                namedFixedToArg[size_t(paramIndex)] = int(i);
            }
        }
        if (!mappingFailed) {
            fixedArgForParam = namedFixedToArg;
            for (size_t i = 0; i < call.args.size(); ++i) {
                if (!call.args[i].name.empty()) continue;
                auto unassigned = llvm::find(fixedArgForParam, -1);
                if (unassigned == fixedArgForParam.end()) {
                    packArgIndices.push_back(i);
                } else {
                    *unassigned = int(i);
                }
            }
            for (size_t j = 0; j < fixedParams.size(); ++j) {
                if (fixedArgForParam[j] == -1 && !fixedParams[j].defaultValue) {
                    mappingFailed = true;
                    break;
                }
            }
            packCount = packArgIndices.size();
        }
        if (mappingFailed) {
            if (call.args.size() < fixedParams.size()) return std::nullopt;
            packCount = call.args.size() - fixedParams.size();
            for (size_t i = fixedParams.size(); i < call.args.size(); ++i) {
                if (!call.args[i].name.empty()) {
                    if (returnOnError) return std::nullopt;
                    ERROR(call.args[i].location, "variadic arguments cannot have labels");
                }
            }
            for (size_t j = 0; j < fixedParams.size(); ++j)
                fixedArgForParam[j] = int(j);
            packArgIndices.clear();
            for (size_t j = 0; j < packCount; ++j)
                packArgIndices.push_back(fixedParams.size() + j);
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
        GenericArg genericArg;
        Expr* genericArgValue = nullptr;

        for (size_t j = 0; j < fixedParams.size(); ++j) {
            Type paramType = fixedParams[j].type;
            if (!paramType || !containsGenericParam(paramType, genericParam->getName())) continue;
            if (fixedArgForParam[j] == -1) continue;

            auto* argValue = call.args[size_t(fixedArgForParam[j])].value;
            auto expectedType = replaceUnresolvedGenericParamsWithPlaceholders(paramType, genericParams);
            Type argType = argValue->hasType() ? argValue->type : typecheckExpr(*argValue, false, expectedType);
            GenericArg maybeGenericArg = findGenericArg(argType, paramType, genericParam->getName());
            if (!maybeGenericArg) continue;

            if (!genericArg) {
                genericArg = maybeGenericArg;
                genericArgValue = argValue;
            } else {
                Type paramTypeWithGenericArg = paramType.resolve({{genericParam->getName(), genericArg}});
                Type paramTypeWithMaybeGenericArg = paramType.resolve({{genericParam->getName(), maybeGenericArg}});

                if (isImplicitlyConvertible(argValue, argValue->type, paramTypeWithGenericArg, true, nullptr, false)) {
                    continue;
                } else if (isImplicitlyConvertible(genericArgValue, genericArgValue->type, paramTypeWithMaybeGenericArg, true, nullptr, false)) {
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
        auto* argValue = call.args[packArgIndices[j]].value;
        auto expectedType = replaceUnresolvedGenericParamsWithPlaceholders(packType, genericParams);
        Type argType = argValue->hasType() ? argValue->type : typecheckExpr(*argValue, false, expectedType);

        for (auto* genericParam : packGenerics) {
            GenericArg genericArg = findGenericArg(argType, packType, genericParam->getName());
            if (!genericArg) return std::nullopt;
            result.packArgs[j][genericParam->getName()] = genericArg;
        }
    }

    auto checkConstraint = [&](const GenericParamDecl& genericParam, GenericArg genericArg) -> bool {
        if (genericParam.isValueParam || genericParam.constraints.empty()) return true;
        ASSERT(genericParam.constraints.size() == 1, "cannot have multiple generic constraints yet");
        auto* interface = getTypeDecl(*llvm::cast<BasicType>(genericParam.constraints[0].typeBase));

        if (auto basicType = llvm::dyn_cast<BasicType>(genericArg.type.typeBase)) {
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

std::string cx::narrowingHint(Type source, Type target) {
    if (target.removeOptional().isPointerType() && !target.removeOptional().isReferenceType()
        && source.equalsIgnoreTopLevelMutable(target.removeOptional().getPointee())) {
        return " (use '&' to take the address explicitly)";
    }
    if (source.removeOptional().isPointerType() && source.removeOptional().getPointee().equalsIgnoreTopLevelMutable(target.removeOptional())) {
        return " (use '*' to dereference explicitly)";
    }
    auto isNumeric = [](Type type) { return type.isInteger() || type.isFloatingPoint() || type.isChar(); };
    if (!isNumeric(source) || !isNumeric(target)) return "";
    return " (use '" + target.toString() + "(...)' to convert explicitly)";
}

// Suggests '&' when a call would match a concrete candidate by taking addresses.
static std::string addressOfHintForCall(const CallExpr& expr, llvm::ArrayRef<Decl*> candidates) {
    for (Decl* candidate : candidates) {
        auto* functionDecl = llvm::dyn_cast<FunctionDecl>(candidate);
        if (!functionDecl || functionDecl->isVariadic()) continue;
        auto params = functionDecl->getParams();
        if (params.size() != expr.args.size()) continue;
        if (llvm::any_of(expr.args, [](auto& arg) { return !arg.name.empty(); })) continue;

        bool anyAddress = false;
        const Expr* firstAddressArg = nullptr;
        for (size_t i = 0; i < expr.args.size(); ++i) {
            Type argType = expr.args[i].value->type;
            Type paramType = params[i].type;
            if (argType == paramType) continue;
            if (paramType.isPointerType() && !paramType.isReferenceType() && argType.equalsIgnoreTopLevelMutable(paramType.getPointee())) {
                anyAddress = true;
                if (!firstAddressArg) firstAddressArg = expr.args[i].value;
                continue;
            }
            anyAddress = false;
            break;
        }
        if (!anyAddress) continue;

        if (firstAddressArg) {
            if (auto* var = llvm::dyn_cast<VarExpr>(firstAddressArg)) {
                return (" (did you mean '&" + var->identifier + "'?)").str();
            }
        }
        return " (use '&' to pass arguments by pointer)";
    }
    return "";
}

void cx::validateGenericArgCount(size_t genericParamCount, llvm::ArrayRef<GenericArg> genericArgs, llvm::StringRef name, Location location) {
    if (genericArgs.size() < genericParamCount) {
        REPORT_ERROR(location, "too few generic arguments to '" << name << "', expected " << genericParamCount);
    } else if (genericArgs.size() > genericParamCount) {
        REPORT_ERROR(location, "too many generic arguments to '" << name << "', expected " << genericParamCount);
    }
}

// A bare type name may reference an integer parameter of an enclosing template,
// whose kind can't be checked until instantiation.
static bool isBareName(GenericArg arg) {
    return arg.isType() && arg.type.isBasicType() && arg.type.getGenericArgs().empty();
}

// A bare name that denotes a known type or value cannot be an integer parameter reference.
bool Typechecker::genericArgsMatch(llvm::ArrayRef<GenericParamDecl> genericParams, llvm::ArrayRef<GenericArg> genericArgs) {
    if (genericArgs.size() != genericParams.size()) return false;
    for (auto&& [genericParam, genericArg] : llvm::zip(genericParams, genericArgs)) {
        if (genericParam.isValueParam) {
            if (genericArg.isInt()) continue;
            if (!isBareName(genericArg)) return false;
            if (genericArg.type.isBuiltinType() || !findDecls(genericArg.type.getName()).empty()) return false;
        } else if (genericArg.isInt()) {
            return false;
        }
    }
    return true;
}

bool Typechecker::validateGenericArgs(llvm::ArrayRef<GenericParamDecl> genericParams, llvm::ArrayRef<GenericArg> genericArgs, llvm::StringRef name,
                                      Location location) {
    if (genericArgs.size() < genericParams.size()) {
        REPORT_ERROR(location, "too few generic arguments to '" << name << "', expected " << genericParams.size());
        return false;
    }
    if (genericArgs.size() > genericParams.size()) {
        REPORT_ERROR(location, "too many generic arguments to '" << name << "', expected " << genericParams.size());
        return false;
    }
    bool valid = true;
    for (auto&& [genericParam, genericArg] : llvm::zip(genericParams, genericArgs)) {
        if (genericParam.isValueParam && !genericArg.isInt()) {
            if (!isBareName(genericArg) || genericArg.type.isBuiltinType() || !findDecls(genericArg.type.getName()).empty()) {
                REPORT_ERROR(genericArg.location, "expected integer generic argument for parameter '" << genericParam.getName() << "' of '" << name << "'");
                valid = false;
            }
        } else if (!genericParam.isValueParam && genericArg.isInt()) {
            REPORT_ERROR(genericArg.location, "expected type generic argument for parameter '" << genericParam.getName() << "' of '" << name << "'");
            valid = false;
        }
    }
    return valid;
}

// Whether the call's generic arguments can be taken from the expected type. The callee must belong to the same type
// as the expected type, or to the template pattern it was instantiated from. For non-constructor calls the return type
// must additionally mention a generic parameter; a concrete return type carries no information about the callee's parameters.
static bool canInferFromExpectedType(llvm::ArrayRef<GenericParamDecl> genericParams, FunctionDecl* decl, Type expectedType) {
    if (!expectedType || !expectedType.isBasicType() || expectedType.getGenericArgs().empty()
        || llvm::any_of(expectedType.getGenericArgs(), [](GenericArg arg) { return arg.isType() && arg.type.isUnresolvedType(); })) {
        return false;
    }
    auto* expectedDecl = expectedType.getDecl();
    if (!expectedDecl) return false;
    if (decl->isConstructorDecl()) {
        auto* pattern = decl->getTypeDecl();
        return expectedDecl == pattern || expectedDecl->instantiatedFrom == pattern;
    }
    auto* returnDecl = decl->getReturnType().getDecl();
    if (expectedDecl != returnDecl && expectedDecl->instantiatedFrom != returnDecl) return false;
    return llvm::any_of(genericParams, [&](auto& genericParam) { return containsGenericParam(decl->getReturnType(), genericParam.getName()); });
}

llvm::StringMap<GenericArg> Typechecker::getGenericArgsForCall(llvm::ArrayRef<GenericParamDecl> genericParams, CallExpr& call, FunctionDecl* decl,
                                                               bool returnOnError, Type expectedType) {
    ASSERT(!genericParams.empty());
    std::vector<GenericArg> inferredGenericArgs;
    llvm::ArrayRef<GenericArg> genericArgTypes;

    if (call.genericArgs.empty()) {
        if (canInferFromExpectedType(genericParams, decl, expectedType)) {
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
        for (GenericArg arg : call.genericArgs) {
            if (arg.isType() && arg.type.storesBorrow()) {
                ERROR(arg.location, "reference type '" << arg.type << "' may only appear as a function parameter type");
            }
        }
        if (!genericArgsMatch(genericParams, call.genericArgs)) return {};
        genericArgTypes = call.genericArgs;
    }

    llvm::StringMap<GenericArg> genericArgs;
    auto genericArg = genericArgTypes.begin();

    for (const GenericParamDecl& genericParam : genericParams) {
        genericArgs.try_emplace(genericParam.getName(), *genericArg++);
    }

    return genericArgs;
}

Type Typechecker::typecheckBuiltinConversion(CallExpr& expr) {
    if (expr.args.size() != 1) {
        ERROR_RANGE(expr.location, expr.endLocation, "expected single argument to converting constructor");
    }
    if (!expr.genericArgs.empty()) {
        ERROR_RANGE(expr.location, expr.endLocation, "expected no generic arguments to converting constructor");
    }
    if (!expr.args.front().name.empty()) {
        ERROR_RANGE(expr.location, expr.endLocation, "expected unnamed argument to converting constructor");
    }

    auto sourceType = typecheckExpr(*expr.args.front().value);
    auto targetType = BasicType::get(expr.getFunctionName(), {});

    if (sourceType.isReferenceType()) {
        // Borrows convert as their referent; builtin conversions always copy into a fresh scalar.
        expr.args.front().value = makeAST<ImplicitCastExpr>(expr.args.front().value, sourceType.getPointee(), ImplicitCastExpr::AutoDereference);
        sourceType = sourceType.getPointee();
    }

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

static std::vector<ParamDecl> getMatchParams(const Match& match) {
    if (auto functionDecl = llvm::dyn_cast<FunctionDecl>(match.decl)) {
        return functionDecl->getParams();
    } else if (auto variableDecl = llvm::dyn_cast<VariableDecl>(match.decl)) {
        return llvm::cast<FunctionType>(variableDecl->type.typeBase)->getParamDecls();
    } else {
        llvm_unreachable("unhandled callee decl");
    }
}

static const Match* findMatchByPredicate(llvm::ArrayRef<Match> matches, const CallExpr& call, llvm::function_ref<bool(Type param, Type arg)> predicate) {
    const Match* result = nullptr;

    for (auto& match : matches) {
        auto params = getMatchParams(match);

        if (params.size() == call.args.size()) {
            std::vector<int> argToParam, paramToArg;
            if (computeArgParamMapping(call.args, params, false, argToParam, paramToArg)) continue;
            bool allMatch = true;
            for (size_t i = 0; i < call.args.size(); ++i) {
                if (!predicate(params[size_t(argToParam[i])].type, call.args[i].value->type)) {
                    allMatch = false;
                    break;
                }
            }
            if (allMatch) {
                if (result) return nullptr;
                result = &match;
            }
        }
    }

    return result;
}

// Returns the only candidate with the most exactly matching arguments, or null
// when tied. Subsumes the all-exact rule: a unique all-exact candidate is also
// the unique most-exact one.
static const Match* findMatchWithMostExactArgs(llvm::ArrayRef<Match> matches, const CallExpr& call) {
    const Match* result = nullptr;
    auto bestCount = -1;

    for (auto& match : matches) {
        auto params = getMatchParams(match);
        if (params.size() != call.args.size()) continue;
        std::vector<int> argToParam, paramToArg;
        if (computeArgParamMapping(call.args, params, false, argToParam, paramToArg)) continue;

        int count = 0;
        for (size_t i = 0; i < call.args.size(); ++i) {
            if (params[size_t(argToParam[i])].type == call.args[i].value->type) ++count;
        }
        if (count > bestCount) {
            bestCount = count;
            result = &match;
        } else if (count == bestCount) {
            result = nullptr;
        }
    }

    return result;
}

static bool isStdlibDecl(const Match& match) {
    return match.decl->getModule() && match.decl->getModule()->name == "std";
}

static bool isCHeaderDecl(const Match& match) {
    return match.decl->getModule() && match.decl->getModule()->isCHeaderImport;
}

static const Match* resolveAmbiguousOverload(llvm::ArrayRef<Match> matches, const CallExpr& call) {
    // An explicitly imported C header takes precedence over the implicit prelude:
    // importing a header must actually provide its declarations, including for
    // names the standard library also declares (those are module-private).
    if (llvm::count_if(matches, isCHeaderDecl) == 1 && llvm::all_of(matches, [](auto& match) { return isStdlibDecl(match) || isCHeaderDecl(match); })) {
        return llvm::find_if(matches, isCHeaderDecl);
    } else if (llvm::all_of(matches, isCHeaderDecl)) {
        // Redeclarations in multiple C headers are considered the same declaration, so just return one of them.
        return &matches[0];
    } else if (llvm::count_if(matches, [](auto& match) { return match.didConvertArguments == false; }) == 1) {
        return llvm::find_if(matches, [](auto& match) { return match.didConvertArguments == false; });
    } else if (llvm::count_if(matches, [](auto& match) { return match.didUnwrapOptional == false; }) == 1) {
        // Implicit unwrapping discards nullability; prefer the overload that preserves it.
        return llvm::find_if(matches, [](auto& match) { return match.didUnwrapOptional == false; });
    } else if (llvm::count_if(matches, [](auto& match) { return match.didWrapOptional == false; }) == 1) {
        // Implicit wrapping adds nullability; prefer the overload that binds directly.
        return llvm::find_if(matches, [](auto& match) { return match.didWrapOptional == false; });
    } else if (auto match = findMatchWithMostExactArgs(matches, call)) {
        return match;
    } else if (auto match = findMatchByPredicate(matches, call, [](Type param, Type arg) { return param == arg.getPointerTo(); })) {
        return match;
    } else {
        return nullptr;
    }
}

static bool equals(const llvm::StringMap<GenericArg>& a, const llvm::StringMap<GenericArg>& b) {
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

// True for operators with fallback resolution: == and != retry with swapped operands,
// and !=, >, >=, and <= derive from == and <. A single non-matching overload must not
// fail hard for these; it falls through to the fallbacks below like a non-match.
static bool hasComparisonFallback(const CallExpr& expr) {
    auto* binaryExpr = llvm::dyn_cast<BinaryExpr>(&expr);
    if (!binaryExpr) return false;
    switch (binaryExpr->op) {
    case Token::Equal:
    case Token::NotEqual:
    case Token::Greater:
    case Token::GreaterOrEqual:
    case Token::LessOrEqual:
        return true;
    default:
        return false;
    }
}

Decl* Typechecker::resolveOverload(llvm::ArrayRef<Decl*> decls, CallExpr& expr, llvm::StringRef callee, Type expectedType, bool allowCommutativeRetry) {
    if (auto* binaryExpr = llvm::dyn_cast<BinaryExpr>(&expr); binaryExpr && expr.calleeDecl) {
        // Argument validation probes typecheck arguments and then clear their types, so
        // operators resolve twice. Derivation swaps operands in place, which would toggle
        // back on the second pass. BinaryExpr resolution ignores expectedType, so the
        // first result is final.
        return expr.calleeDecl;
    }

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
                        ERROR_RANGE(expr.location, expr.endLocation,
                                    "cannot specify generic arguments explicitly for variadic function '" << expr.getFunctionName() << "'");
                    }
                    continue;
                }

                auto variadicArgs = inferVariadicGenericArgs(genericParams, expr, functionTemplate->functionDecl->getParams(), decls.size() != 1);
                if (!variadicArgs) continue;

                auto* functionDecl = functionTemplate->instantiateVariadic(variadicArgs->fixedArgs, variadicArgs->packArgs, std::move(variadicArgs->cacheKey));

                if (decls.size() == 1) {
                    if (!matchArguments(expr, functionDecl) && hasComparisonFallback(expr)) continue;
                    validateAndConvertArguments(expr, *functionDecl, callee, expr.callee->location);
                    deferTypechecking(functionDecl);
                    return functionDecl;
                }
                if (auto match = matchArguments(expr, functionDecl)) {
                    templateMatches.push_back(*match);
                }
                break;
            }

            if (!expr.genericArgs.empty() && decls.size() == 1) {
                if (!validateGenericArgs(genericParams, expr.genericArgs, expr.getFunctionName(), expr.location)) continue;
            } else if (!expr.genericArgs.empty() && expr.genericArgs.size() != genericParams.size()) {
                continue;
            }

            llvm::StringMap<GenericArg> genericArgs;
            try {
                genericArgs = getGenericArgsForCall(genericParams, expr, functionTemplate->functionDecl, decls.size() != 1, expectedType);
            } catch (const CompileError&) {
                // Derivable comparison operators (e.g. > from <) fall back below; don't fail hard on inference errors.
                Token::Kind op = Token::None;
                if (auto* binaryExpr = llvm::dyn_cast<BinaryExpr>(&expr)) op = binaryExpr->op;
                if (op != Token::NotEqual && op != Token::Greater && op != Token::GreaterOrEqual && op != Token::LessOrEqual) throw;
            }
            if (genericArgs.empty()) continue; // Couldn't infer generic arguments.

            auto* functionDecl = functionTemplate->instantiate(genericArgs);

            if (decls.size() == 1) {
                if (!matchArguments(expr, functionDecl) && hasComparisonFallback(expr)) continue;
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
                validateGenericArgs({}, expr.genericArgs, expr.getFunctionName(), expr.location);
                if (!matchArguments(expr, functionDecl) && hasComparisonFallback(expr)) continue;
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
            validateGenericArgs({}, expr.genericArgs, expr.getFunctionName(), expr.location);

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

            // With explicit generic arguments and a single candidate, report argument problems once here;
            // overload probing below stays silent so retries don't duplicate the error.
            if (!expr.genericArgs.empty() && decls.size() == 1 && constructorDecls.size() == 1
                && !validateGenericArgs(typeTemplate->genericParams, expr.genericArgs, expr.getFunctionName(), expr.location)) {
                throw CompileError::dependentError();
            }

            std::vector<llvm::StringMap<GenericArg>> genericArgSets;

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
                    validateAndConvertArguments(expr, paramDecls, false, callee, expr.callee->location, variableDecl);
                    return variableDecl;
                }
                if (auto match = matchArguments(expr, variableDecl, paramDecls)) {
                    matches.push_back(*match);
                }
            }
            break;
        }
        case DeclKind::DestructorDecl:
            matches.push_back({decl, false, false});
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

    if (matches.empty() && allowCommutativeRetry) {
        if (auto* binaryExpr = llvm::dyn_cast<BinaryExpr>(&expr)) {
            if ((binaryExpr->op == Token::Equal || binaryExpr->op == Token::NotEqual) && expr.args.size() == 2) {
                // == and != commute: retry with swapped operands so only one parameter order needs an overload.
                // The matched overload runs with its declared parameter order.
                std::swap(expr.args[0], expr.args[1]);
                try {
                    return resolveOverload(decls, expr, callee, expectedType, false);
                } catch (const CompileError&) {
                    // Restore the written order and fall through to the error
                    // below so the diagnostic shows the user's operand order.
                    std::swap(expr.args[0], expr.args[1]);
                }
            }
        }
    }

    if (matches.empty()) {
        if (auto* binaryExpr = llvm::dyn_cast<BinaryExpr>(&expr)) {
            // Derive missing comparison operators from their counterparts so only == and < need overloads.
            Token::Kind derivedOp = Token::None;
            bool swapOperands = false;
            bool negateResult = false;
            switch (binaryExpr->op) {
            case Token::NotEqual:
                derivedOp = Token::Equal;
                negateResult = true;
                break;
            case Token::Greater:
                derivedOp = Token::Less;
                swapOperands = true;
                break;
            case Token::GreaterOrEqual:
                derivedOp = Token::Less;
                negateResult = true;
                break;
            case Token::LessOrEqual:
                derivedOp = Token::Less;
                swapOperands = true;
                negateResult = true;
                break;
            default:
                break;
            }
            if (derivedOp != Token::None && expr.args.size() == 2) {
                if (auto* calleeVar = llvm::dyn_cast<VarExpr>(expr.callee)) {
                    auto savedOp = binaryExpr->op;
                    auto savedCallee = calleeVar->identifier;
                    binaryExpr->op = derivedOp;
                    calleeVar->identifier = internString(getFunctionName(derivedOp));
                    if (swapOperands) std::swap(expr.args[0], expr.args[1]);
                    try {
                        auto derivedCallee = std::string(expr.getFunctionName());
                        auto derivedDecls = findCalleeCandidates(expr, derivedCallee);
                        auto* decl = resolveOverload(derivedDecls, expr, derivedCallee, expectedType, allowCommutativeRetry);
                        binaryExpr->op = savedOp;
                        calleeVar->identifier = savedCallee;
                        binaryExpr->negateResult = negateResult;
                        return decl;
                    } catch (const CompileError&) {
                        // Restore the written form and fall through to the error below.
                        binaryExpr->op = savedOp;
                        calleeVar->identifier = savedCallee;
                        if (swapOperands) std::swap(expr.args[0], expr.args[1]);
                    }
                }
            }
        }
    }

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
        try {
            for (auto& arg : expr.args) {
                if (!arg.value->hasType()) {
                    typecheckExpr(*arg.value);
                }
            }
        } catch (const CompileError&) {
            // Args like `[]` need expected types to infer. Multiple applicable overloads
            // means the call is ambiguous; report that instead of the inference error.
            ERROR_WITH_NOTES(expr.callee->location, getCandidateNotes(map(matches, [](auto& match) { return match.decl; }), expr),
                             "ambiguous reference to '" << calleeWithGenericArgs << "'" << (isConstructorCall ? " constructor" : ""));
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
        if (expr.getFunctionName() == "[]" || expr.getFunctionName() == "[]=") {
            ERROR(expr.callee->location, "'" << expr.receiverType << "' doesn't provide an operator" << expr.getFunctionName());
        }
        ERROR(expr.callee->location, "unknown identifier '" << callee << "'");
    }

    bool atLeastOneFunction =
        llvm::any_of(decls, [](Decl* decl) { return decl->isFunctionDecl() || decl->isFunctionTemplate() || decl->isTypeDecl() || decl->isTypeTemplate(); });

    if (atLeastOneFunction) {
        if (auto binaryExpr = llvm::dyn_cast<BinaryExpr>(&expr)) {
            // Don't list candidate functions for operators; they're usually irrelevant stdlib overloads that drown out the actual error.
            ERROR(expr.callee->location, "no matching operator '" << binaryExpr->op << "' with arguments '" << binaryExpr->getLHS().type << "' and '"
                                                                  << binaryExpr->getRHS().type << "'" << mixedPointerOperandHint(*binaryExpr));
        } else {
            auto argTypes = map(expr.args, [&](const NamedValue& arg) { return typecheckExpr(*arg.value).toString(); });
            ERROR_WITH_NOTES(expr.callee->location, getCandidateNotes(candidates, expr),
                             (isConstructorCall ? "no matching constructor '" : "no matching function '")
                                 << calleeWithGenericArgs << "(" << llvm::join(argTypes, ", ") << ")'" << addressOfHintForCall(expr, candidates));
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
        ERROR_RANGE(expr.location, expr.endLocation, "anonymous function calls not implemented yet");
    }

    if (Type::isBuiltinScalar(expr.getFunctionName())) {
        return typecheckBuiltinConversion(expr);
    }

    if (expr.isBuiltinCast()) {
        return typecheckBuiltinCast(expr);
    }

    if (expr.getFunctionName() == "assert") {
        llvm::SmallVector<ParamDecl, 2> assertParams;
        assertParams.emplace_back(Type::getBool(), "", false, Location());
        assertParams.emplace_back(BasicType::get("string", {}), "message", false, Location());
        assertParams.back().defaultValue = makeAST<StringLiteralExpr>(std::string("Assertion failed"), expr.location);
        validateAndConvertArguments(expr, assertParams, false, expr.getFunctionName(), expr.location);
        validateGenericArgCount(0, expr.genericArgs, expr.getFunctionName(), expr.location);
        for (size_t i = 0; i < expr.args.size(); ++i) {
            if (expr.argParamIndices[i] == 1 && !llvm::isa<StringLiteralExpr>(expr.args[i].value)) {
                ERROR(expr.args[i].location, "assert message must be a string literal");
            }
        }
        return Type::getVoid();
    }

    Decl* decl;

    if (auto* enumCase = getEnumCase(*expr.callee, expectedType, &expr)) {
        decl = enumCase;
        llvm::cast<MemberExpr>(*expr.callee).decl = decl;
    } else if (expr.callee->isMemberExpr()) {
        Type receiverType = typecheckExpr(*expr.getReceiver());
        expr.receiverType = receiverType;

        if (receiverType.removeOptional().removePointer().isArrayType()) {
            // TODO: Move these member functions to a 'struct Array' declaration in stdlib.
            if (expr.getFunctionName() == "data") {
                validateAndConvertArguments(expr, {}, false, expr.getFunctionName(), expr.location);
                validateGenericArgs({}, expr.genericArgs, expr.getFunctionName(), expr.location);
                return ArrayType::get(receiverType.removePointer().getElementType(), ArrayType::UnknownSize);
            }
            if (expr.getFunctionName() == "size") {
                validateAndConvertArguments(expr, {}, false, expr.getFunctionName(), expr.location);
                validateGenericArgs({}, expr.genericArgs, expr.getFunctionName(), expr.location);
                return ArrayType::getIndexType();
            }
            if (expr.getFunctionName() == "iterator") {
                validateAndConvertArguments(expr, {}, false, expr.getFunctionName(), expr.location);
                validateGenericArgCount(0, expr.genericArgs, expr.getFunctionName(), expr.location);
                return BasicType::get("ArrayIterator", GenericArg(receiverType.removePointer().getElementType()));
            }

            ERROR(expr.getReceiver()->location, "type '" << receiverType.removePointer() << "' has no member function '" << expr.getFunctionName() << "'");
        } else if (receiverType.removeOptional().removePointer().isBuiltinType() && expr.getFunctionName() == "deinit") {
            return Type::getVoid();
        }

        if (expr.args.size() == 1 && expr.getFunctionName() == "init") {
            typecheckExpr(*expr.args[0].value);

            if (expr.isMoveInit()) {
                if (!expr.args[0].value->type.removeReference().isImplicitlyCopyable()) {
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

        // An explicit deinit consumes the value like a move, suppressing the scope-exit destructor call.
        if (llvm::isa<DestructorDecl>(decl)) {
            setMoved(expr.getReceiver(), true);
        }
    } else {
        auto callee = expr.getFunctionName();
        auto decls = findCalleeCandidates(expr, callee);

        if (decls.empty()) {
            if (auto* varExpr = llvm::dyn_cast<VarExpr>(expr.callee)) {
                if (auto* enumCase = getExpectedEnumCase(varExpr->identifier, expectedType)) {
                    // An unqualified `Ok(...)` mirrors the qualified `Result.Ok(...)`, so desugar to it.
                    expr.callee =
                        makeAST<MemberExpr>(makeAST<VarExpr>(enumCase->getEnumDecl()->getName(), varExpr->location), varExpr->identifier, varExpr->location);
                    return typecheckCallExpr(expr, expectedType);
                }
            }
        }

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
        if (type) {
            params = map(type.getAnonymousStructElements(), [&](auto& e) { return ParamDecl(e.type, e.name, false, decl->getLocation()); });
        }
        validateAndConvertArguments(expr, params, false, decl->getName(), expr.location);
    }

    if (expr.argParamIndices.size() == expr.args.size()) {
        for (size_t i = 0; i < expr.args.size(); ++i) {
            int paramIndex = expr.argParamIndices[i];
            const ParamDecl* param = (paramIndex != -1 && size_t(paramIndex) < params.size()) ? &params[size_t(paramIndex)] : nullptr;
            if (!expr.args[i].value->type.removeReference().isImplicitlyCopyable() && (!param || !param->type.isImplicitlyCopyable())) {
                setMoved(expr.args[i].value, true);
            }
        }
    } else {
        for (auto&& [param, arg] : llvm::zip_longest(params, expr.args)) {
            if (arg && !arg->value->type.removeReference().isImplicitlyCopyable() && (!param || !param->type.isImplicitlyCopyable())) {
                setMoved(arg->value, true);
            }
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
    std::vector<int> argToParam, paramToArg;
    if (auto mappingError = computeArgParamMapping(expr.args, params, isVariadic, argToParam, paramToArg)) return *mappingError;

    bool didConvertArguments = false;
    bool didUnwrapOptional = false;
    bool didWrapOptional = false;

    for (size_t i = 0; i < expr.args.size(); ++i) {
        auto& arg = expr.args[i];
        int paramIndex = argToParam[i];
        if (paramIndex == -1) continue;
        auto& param = params[size_t(paramIndex)];

        bool hadType = arg.value->hasType();

        if (!arg.value->hasType()) {
            try {
                typecheckExpr(*arg.value, false, param.type);
            } catch (const CompileError&) {
                arg.value->removeTypes();
                return ArgumentValidation::invalidType(i);
            }
        }

        bool invalidType = false;
        std::optional<ImplicitCastExpr::Kind> implicitCastKind;
        // Probing: other overload candidates are still untried, so don't diagnose yet.
        if (Type convertedType = isImplicitlyConvertible(arg.value, arg.value->type, param.type, true, &implicitCastKind, false)) {
            didConvertArguments = didConvertArguments || convertedType != arg.value->type || implicitCastKind.has_value();
            didUnwrapOptional = didUnwrapOptional || implicitCastKind == ImplicitCastExpr::OptionalUnwrap;
            didWrapOptional = didWrapOptional || implicitCastKind == ImplicitCastExpr::OptionalWrap || arg.value->isNullLiteralExpr();
        } else {
            invalidType = true;
        }

        if (!hadType) arg.value->removeTypes();
        if (invalidType) return ArgumentValidation::invalidType(i);
    }

    return ArgumentValidation::success(didConvertArguments, didUnwrapOptional, didWrapOptional);
}

std::optional<Match> Typechecker::matchArguments(CallExpr& expr, Decl* calleeDecl, llvm::ArrayRef<ParamDecl> params) {
    bool isVariadic = false;
    if (auto functionDecl = llvm::dyn_cast<FunctionDecl>(calleeDecl)) {
        params = functionDecl->getParams();
        isVariadic = functionDecl->isVariadic();
    }
    auto result = getArgumentValidationResult(expr, params, isVariadic);
    if (result.error) return std::nullopt;
    return Match{calleeDecl, result.didConvertArguments, result.didUnwrapOptional, result.didWrapOptional};
}

void Typechecker::validateAndConvertArguments(CallExpr& expr, const Decl& calleeDecl, llvm::StringRef functionName, Location location) {
    if (auto functionDecl = llvm::dyn_cast<FunctionDecl>(&calleeDecl)) {
        validateAndConvertArguments(expr, functionDecl->getParams(), functionDecl->isVariadic(), functionName, location, functionDecl);
    } else {
        auto paramDecls = getVariableCalleeParams(llvm::cast<VariableDecl>(calleeDecl));
        validateAndConvertArguments(expr, paramDecls, false, functionName, location, &calleeDecl);
    }
}

void Typechecker::validateAndConvertArguments(CallExpr& expr, llvm::ArrayRef<ParamDecl> params, bool isVariadic, llvm::StringRef callee, Location location,
                                              const Decl* calleeDecl) {
    auto result = getArgumentValidationResult(expr, params, isVariadic);

    std::vector<int> argToParam, paramToArg;
    // Mapping is available in every case except arity errors, which return before filling it fully.
    // Recompute for error messages; success always has a complete mapping.
    computeArgParamMapping(expr.args, params, isVariadic, argToParam, paramToArg);

    // Arguments are type-checked here for error messages, but type-converted only in the success case below
    // (they might not convert properly in the case of error).
    for (size_t i = 0; i < expr.args.size(); ++i) {
        auto& arg = expr.args[i];
        Type expectedType;
        if (i < argToParam.size() && argToParam[i] != -1) expectedType = params[size_t(argToParam[i])].type;
        if (!arg.value->hasType()) typecheckExpr(*arg.value, false, expectedType);
    }

    // Point arity errors at the declaration so the source excerpt shows the expected prototype.
    std::vector<Note> declNote;
    if (calleeDecl && calleeDecl->getLocation().isValid()) {
        declNote.push_back(Note{calleeDecl->getLocation(), ("'" + callee + "' declared here").str()});
    }

    switch (result.error) {
    case ArgumentValidation::None: {
        for (size_t i = 0; i < expr.args.size(); ++i) {
            int paramIndex = argToParam[i];
            if (paramIndex != -1) expr.args[i].value = convert(expr.args[i].value, params[size_t(paramIndex)].type, true);
        }
        for (size_t j = 0; j < params.size(); ++j) {
            if (paramToArg[j] != -1) continue;
            const ParamDecl& param = params[j];
            ASSERT(param.defaultValue);
            Expr* defaultArg = param.defaultValue->instantiate({});
            if (!defaultArg->hasType()) typecheckExpr(*defaultArg, false, param.type);
            if (Expr* converted = convert(defaultArg, param.type, true)) {
                defaultArg = converted;
            } else {
                ERROR_RANGE(expr.location, expr.endLocation,
                            "cannot assign '" << defaultArg->type << "' to '" << param.type << "'" << narrowingHint(defaultArg->type, param.type));
            }
            argToParam.push_back(int(j));
            expr.args.emplace_back(std::string(param.getName()), defaultArg, expr.location);
        }
        expr.argParamIndices = std::move(argToParam);
        break;
    }
    case ArgumentValidation::TooFew: {
        size_t requiredParamCount = 0;
        for (auto& param : params) {
            if (!param.defaultValue) ++requiredParamCount;
        }
        bool hasOptionalParams = requiredParamCount != params.size();
        REPORT_ERROR_WITH_NOTES(location, declNote,
                                "too few arguments to '" << callee << "', expected " << ((isVariadic || hasOptionalParams) ? "at least " : "")
                                                         << (hasOptionalParams ? requiredParamCount : params.size()));
        break;
    }
    case ArgumentValidation::TooMany:
        REPORT_ERROR_WITH_NOTES(location, declNote, "too many arguments to '" << callee << "', expected " << params.size());
        break;
    case ArgumentValidation::InvalidName: {
        auto& arg = expr.args[result.index];
        ERROR_WITH_NOTES(arg.location, std::move(declNote), "invalid argument name '" << arg.name << "'");
        break;
    }
    case ArgumentValidation::DuplicateName: {
        auto& arg = expr.args[result.index];
        ERROR_WITH_NOTES(arg.location, std::move(declNote), "duplicate argument for parameter '" << arg.name << "'");
        break;
    }
    case ArgumentValidation::InvalidType: {
        auto& arg = expr.args[result.index];
        auto& param = params[size_t(argToParam[size_t(result.index)])];
        diagnoseClosureConversion(arg.value->type, param.type, arg.location);
        // Validation probed without diagnosing; re-run once so an out-of-range literal still
        // reports the range instead of a generic mismatch. This either throws or returns null,
        // since probing already failed, so discarding the result is safe.
        (void)convert(arg.value, param.type, true);
        ERROR_WITH_NOTES(arg.location, std::move(declNote),
                         "invalid argument #" << (result.index + 1) << " type '" << arg.value->type << "' to '" << callee << "', expected '" << param.type
                                              << "'" << narrowingHint(arg.value->type, param.type));
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

    case TypeKind::AnonymousStructType:
    case TypeKind::FunctionType:
        return false;

    case TypeKind::PointerType: {
        Type sourcePointee = sourceType.getPointee();

        if (targetType.isPointerType()) {
            Type targetPointee = targetType.getPointee();

            // Reinterpretation between any two pointer types is allowed; only dropping const is rejected.
            if (!targetPointee.isMutable() || sourcePointee.isMutable()) {
                return true;
            }
        } else if (targetType.isUnsizedArrayPointer()) {
            if (!targetType.getElementType().isMutable() || sourcePointee.isMutable()) {
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
    validateGenericArgCount(1, expr.genericArgs, expr.getFunctionName(), expr.location);
    if (!expr.genericArgs.front().isType()) {
        ERROR(expr.location, "expected type generic argument for 'cast'");
    }
    Type targetType = expr.genericArgs.front().type;
    ParamDecl param(sourceType, "", false, expr.location);

    validateAndConvertArguments(expr, param, false, expr.getFunctionName(), expr.location);

    if (!isValidCast(sourceType, targetType) && !isValidCast(sourceType.removeOptional(), targetType)) {
        ERROR(expr.callee->location, "illegal cast from '" << sourceType << "' to '" << targetType << "'");
    }

    return targetType;
}

Type Typechecker::typecheckSizeofExpr(SizeofExpr& expr) {
    // `sizeof` accepts a variable as well as a type, e.g. `sizeof(x)`. A type
    // with the same name takes precedence, so previously valid `sizeof(T)`
    // expressions are unaffected even if a variable shadows the type name.
    // Name lookup failures and non-value declarations fall through to the
    // normal type path below.
    if (expr.operandType.isBasicType() && !expr.operandType.isBuiltinType()) {
        auto* basicType = llvm::cast<BasicType>(expr.operandType.typeBase);
        if (basicType->genericArgs.empty()) {
            auto decls = findDecls(basicType->name);
            bool namesType = llvm::any_of(decls, [](Decl* decl) { return decl->isTypeDecl() || decl->isTypeTemplate(); });
            bool varHadError = false;
            if (!namesType) {
                try {
                    Decl* decl = findDecl(basicType->name, expr.location);
                    Type varType;
                    if (auto* varDecl = llvm::dyn_cast<VarDecl>(decl)) {
                        varType = varDecl->type;
                    } else if (auto* paramDecl = llvm::dyn_cast<ParamDecl>(decl)) {
                        varType = paramDecl->type;
                    } else if (auto* fieldDecl = llvm::dyn_cast<FieldDecl>(decl)) {
                        varType = fieldDecl->type;
                    } else {
                        decl = nullptr;
                    }
                    if (decl && !varType) {
                        varHadError = true;
                    } else if (varType) {
                        checkHasAccess(*decl, expr.location, AccessLevel::None);
                        decl->referenced = true;
                        expr.operandType = varType;
                    }
                } catch (const CompileError&) {
                    // Fall through to report the type error below.
                }
            }
            if (varHadError) {
                throw CompileError::dependentError(); // Declaration had an error, don't cascade.
            }
        }
    }
    typecheckType(expr.operandType, AccessLevel::None, /*recheckGenericArgs=*/true, /*allowReference=*/true);
    return Type::getUInt64();
}

Type Typechecker::typecheckMemberExpr(MemberExpr& expr, Type expectedType, bool useIsWriteOnly) {
    if (auto* enumCase = getEnumCase(expr, expectedType)) {
        checkHasAccess(*enumCase->getEnumDecl(), expr.base->location, AccessLevel::None);
        expr.decl = enumCase;
        return enumCase->type;
    }

    if (VarDecl* staticConst = getStaticConst(expr)) {
        checkHasAccess(*staticConst, expr.location, AccessLevel::None);
        expr.decl = staticConst;
        return staticConst->type;
    }

    Type baseType = typecheckExpr(*expr.base, useIsWriteOnly);
    if (!expr.base->isThis()) baseType = baseType.removeOptional();
    baseType = baseType.removePointer();

    if (baseType.isArrayType()) {
        if (llvm::is_contained({"count", "length", "size"}, expr.member)) {
            ERROR_RANGE(expr.location, expr.endLocation, "use the '.size()' member function to get the number of elements in an array");
        }
    } else if (baseType.isAnonymousStructType()) {
        for (auto& element : baseType.getAnonymousStructElements()) {
            if (element.name == expr.member) {
                return element.type.withMutability(baseType.mutability);
            }
        }
    } else if (auto* baseDecl = baseType.getDecl()) {
        // Instance fields cannot be accessed via the type name (e.g. `S.x`);
        // only static constants are. Without this, field access via a type
        // would typecheck but crash codegen which expects an instance.
        bool baseIsType = false;
        if (auto* varBase = llvm::dyn_cast<VarExpr>(expr.base)) {
            baseIsType = varBase->decl && (varBase->decl->isTypeDecl() || varBase->decl->kind == DeclKind::TypeTemplate);
        } else if (auto* memberBase = llvm::dyn_cast<MemberExpr>(expr.base)) {
            baseIsType = memberBase->decl && (memberBase->decl->isTypeDecl() || memberBase->decl->kind == DeclKind::TypeTemplate);
        }
        for (auto& field : baseDecl->fields) {
            if (field.getName() == expr.member) {
                if (baseIsType) break;
                checkHasAccess(field, expr.location, AccessLevel::None);
                expr.decl = &field;
                return field.type.withMutability(baseType.mutability);
            }
        }

        for (auto* staticConst : baseDecl->staticConsts) {
            if (staticConst->getName() == expr.member) {
                checkHasAccess(*staticConst, expr.location, AccessLevel::None);
                expr.decl = staticConst;
                return staticConst->type;
            }
        }
    }

    ERROR_RANGE(expr.location, expr.endLocation, "no member named '" << expr.member << "' in '" << baseType << "'");
}

Type Typechecker::typecheckIndexExpr(IndexExpr& expr, bool baseIsWriteOnly) {
    Type lhsType = typecheckExpr(*expr.getBase(), baseIsWriteOnly);
    Type arrayType;

    if (lhsType.removeOptional().isArrayType()) {
        arrayType = lhsType.removeOptional();
    } else if (lhsType.isPointerType() && lhsType.getPointee().isArrayType()) {
        arrayType = lhsType.getPointee();
    } else if (lhsType.removeOptional().removePointer().isBuiltinType()) {
        ERROR_RANGE(expr.location, expr.endLocation, "'" << lhsType << "' doesn't provide an index operator");
    } else {
        return typecheckCallExpr(expr).removePointer();
    }

    Expr* indexExpr = expr.getIndex();
    bool indexChecked = false;

    // A from-end index 'base[-offset]' accesses 'base[size - offset]'.
    // Desugar constant-size arrays here while the offset is still unchecked,
    // so it is checked exactly once as part of the difference. Other types
    // resolve the operator[-] call built by the parser. (Non-array types
    // returned above, so arrayType is always set here.)
    if (expr.fromEnd) {
        if (!arrayType.isConstantArray()) {
            ERROR_RANGE(indexExpr->location, indexExpr->endLocation, "from-end index '[-]' is not supported for arrays of unknown size");
        }
        llvm::APSInt sizeValue(64, false);
        sizeValue = arrayType.getArraySize();
        auto* sizeLiteral = makeAST<IntLiteralExpr>(std::move(sizeValue), indexExpr->location);
        sizeLiteral->endLocation = indexExpr->location;
        auto* difference = makeAST<BinaryExpr>(BinaryOperator(Token::Minus), sizeLiteral, indexExpr, indexExpr->location);
        difference->endLocation = indexExpr->endLocation;
        typecheckExpr(*difference);
        expr.setIndex(difference);
        indexExpr = difference;
        // Clear so re-checks (e.g. overload probing checking arguments per
        // candidate) see a plain index instead of desugaring again.
        expr.fromEnd = false;
        indexChecked = true;
    }

    if (!indexChecked) {
        Type indexType = typecheckExpr(*indexExpr);

        if (auto converted = convert(indexExpr, ArrayType::getIndexType())) {
            expr.setIndex(converted);
            indexExpr = converted;
        } else if (!indexType.isInteger()) {
            ERROR(indexExpr->location, "illegal index type '" << indexType << "', expected '" << ArrayType::getIndexType() << "'");
        }
        // Wider integer indexes pass through unconverted; both backends accept any integer index type.
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
    auto elementType = typecheckIndexExpr(expr, true);

    if (!expr.getBase()->type.removeOptional().removePointer().isArrayType()) {
        if (auto* baseVarExpr = getAssignmentBaseVarExpr(*expr.getBase())) {
            if (auto* baseVarDecl = llvm::dyn_cast<VarDecl>(baseVarExpr->decl)) {
                if (!baseVarDecl->isGlobal() && !baseVarDecl->initializer && !definitelyAssignedDecls.count(baseVarDecl)) {
                    ERROR(baseVarExpr->location, "use of uninitialized variable '" << baseVarExpr->identifier << "'");
                }
            }
        }
        typecheckCallExpr(expr);
        return Type::getVoid();
    }

    typecheckExpr(*expr.getValue());

    if (auto converted = convert(expr.getValue(), elementType)) {
        expr.setValue(converted);
    } else {
        ERROR(expr.getValue()->location,
              "cannot assign '" << expr.getValue()->type << "' to '" << elementType << "'" << narrowingHint(expr.getValue()->type, elementType));
    }

    if (auto* varExpr = getAssignmentBaseVarExpr(*expr.getBase())) {
        if (varExpr->decl->isVarDecl()) {
            definitelyAssignedDecls.insert(varExpr->decl);
        }
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
        fnParamTypes.push_back(captured->getCaptureType());
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
        makeAST<TypeDecl>(TypeTag::Struct, std::move(name), std::vector<GenericArg>(), std::move(interfaces), AccessLevel::Default, module, nullptr, location);
    closureDecl->addField(FieldDecl(fnType, "__fn", nullptr, *closureDecl, AccessLevel::Private, location));
    for (auto* captured : lambdaDecl.captures) {
        closureDecl->addField(
            FieldDecl(captured->getCaptureType(), ("__capture_" + captured->getName()).str(), nullptr, *closureDecl, AccessLevel::Private, location));
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
        VarExpr use(captured->getName(), expr.location);
        checkNotMoved(*captured, use);
        if (!captured->type.isImplicitlyCopyable()) {
            movedDecls.insert(captured);
        }
    }

    return createClosureType(*expr.functionDecl, expr.location);
}

Type Typechecker::typecheckNullCoalescingExpr(BinaryExpr& expr) {
    Type leftType = typecheckExpr(expr.getLHS());
    if (!leftType.isOptionalType()) {
        ERROR(expr.getLHS().location, "left operand of '" << toString(Token::QuestionQuestion) << "' must be an optional, got '" << leftType << "'");
    }
    auto wrappedType = leftType.getWrappedType();

    // The right side only executes when the left side is null, so only narrowings valid on both paths survive.
    auto outerNarrowings = narrowedTypes;
    auto afterLHSAssignedDecls = definitelyAssignedDecls;
    applyNarrowings(expr.getLHS(), false);
    Type rightType = typecheckExpr(expr.getRHS());
    intersectNarrowings(outerNarrowings);
    definitelyAssignedDecls = afterLHSAssignedDecls;

    // Prefer the unwrapped left type, but never implicitly unwrap the right side: `o1 ?? o2`
    // must stay null when both are null, not trap unwrapping `o2`.
    if (auto* convertedRHS = convert(&expr.getRHS(), wrappedType, false, false)) {
        auto* current = convertedRHS;
        bool unwrapsRHS = false;
        while (auto* cast = llvm::dyn_cast<ImplicitCastExpr>(current)) {
            unwrapsRHS |= cast->castKind == ImplicitCastExpr::OptionalUnwrap;
            current = cast->operand;
        }
        if (!unwrapsRHS) {
            expr.setRHS(convertedRHS);
            return wrappedType;
        }
    }

    // Otherwise the unwrapped value widens to the right side's type (e.g. `char? ?? 0.5` is a float).
    if (!rightType.isOptionalType() && isSafeNumericWidening(wrappedType, rightType)) {
        return rightType;
    }

    // Otherwise both sides stay optional (e.g. `int? ?? int?` is an `int?`).
    if (auto* convertedRHS = convert(&expr.getRHS(), leftType, false, false)) {
        expr.setRHS(convertedRHS);
        return leftType;
    }

    ERROR_RANGE(expr.location, expr.endLocation, "incompatible operand types ('" << leftType << "' and '" << rightType << "')");
}

Type Typechecker::typecheckIfExpr(IfExpr& expr) {
    auto conditionType = typecheckExpr(*expr.condition);
    typecheckImplicitlyBoolConvertibleExpr(conditionType, expr.condition->location, expr.condition->endLocation);
    auto outerNarrowings = narrowedTypes;
    auto outerAssignedDecls = definitelyAssignedDecls;
    applyNarrowings(*expr.condition, true);
    auto thenType = typecheckExpr(*expr.thenExpr);
    auto thenNarrowings = narrowedTypes;
    auto thenAssignedDecls = definitelyAssignedDecls;
    narrowedTypes = outerNarrowings;
    definitelyAssignedDecls = outerAssignedDecls;
    applyNarrowings(*expr.condition, false);
    auto elseType = typecheckExpr(*expr.elseExpr);
    intersectNarrowings(thenNarrowings);
    if (!thenType.isNeverType() && elseType.isNeverType()) {
        definitelyAssignedDecls = thenAssignedDecls;
    } else if (!thenType.isNeverType() && !elseType.isNeverType()) {
        auto elseAssignedDecls = definitelyAssignedDecls;
        definitelyAssignedDecls = thenAssignedDecls;
        for (auto* decl : llvm::to_vector(definitelyAssignedDecls)) {
            if (!elseAssignedDecls.count(decl)) {
                definitelyAssignedDecls.erase(decl);
            }
        }
    }

    if (auto convertedElse = convert(expr.elseExpr, thenType, false, false)) {
        expr.elseExpr = convertedElse;
        return thenType;
    } else if (auto convertedThen = convert(expr.thenExpr, elseType, false, false)) {
        expr.thenExpr = convertedThen;
        return elseType;
    } else {
        ERROR_RANGE(expr.location, expr.endLocation, "incompatible operand types ('" << thenType << "' and '" << elseType << "')");
    }
}

Type Typechecker::typecheckExpr(Expr& expr, bool useIsWriteOnly, Type expectedType) {
    Type type;

    switch (expr.kind) {
    case ExprKind::VarExpr:
        type = typecheckVarExpr(llvm::cast<VarExpr>(expr), useIsWriteOnly, expectedType);
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
    case ExprKind::AnonymousStructExpr:
        type = typecheckAnonymousStructExpr(llvm::cast<AnonymousStructExpr>(expr));
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
        type = typecheckMemberExpr(llvm::cast<MemberExpr>(expr), expectedType, useIsWriteOnly);
        break;
    case ExprKind::IndexExpr:
        type = typecheckIndexExpr(llvm::cast<IndexExpr>(expr), useIsWriteOnly);
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
    case ExprKind::SwitchExpr:
        type = typecheckSwitchExpr(llvm::cast<SwitchExpr>(expr), expectedType);
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
        // Top-level borrows are legitimate transient expression types (e.g. a bare borrow
        // parameter); stored borrows are still rejected in nested positions.
        typecheckType(type, AccessLevel::None, /*recheckGenericArgs=*/false, /*allowReference=*/true);
    }

    return expr.type;
}

static bool enumTemplateMatchesExpectedType(TypeTemplate& typeTemplate, Type expectedType) {
    if (auto* expectedDecl = expectedType.getDecl()) {
        if (expectedDecl->instantiatedFrom == typeTemplate.typeDecl) return true;
    }
    return BasicType::get(expectedType.getName(), {}).getDecl() == typeTemplate.typeDecl;
}

// Returns the expected type to take generic arguments from: the expected type itself,
// or the type it wraps if the expected type is an optional of another instantiation.
// The latter lets e.g. `Opt<int>? x = Opt.None` resolve through the outer optional.
static Type matchEnumTemplateExpectedType(TypeTemplate& typeTemplate, Type expectedType) {
    Type candidates[] = {expectedType, expectedType ? expectedType.removeOptional() : Type()};
    for (Type candidate : candidates) {
        if (candidate && candidate.isBasicType() && !candidate.getGenericArgs().empty()
            && llvm::none_of(candidate.getGenericArgs(), [](GenericArg arg) { return arg.isType() && arg.type.isUnresolvedType(); })
            && enumTemplateMatchesExpectedType(typeTemplate, candidate)) {
            return candidate;
        }
    }
    return Type();
}

// If the expected type names an enum with a case called `name`, returns that case.
EnumCase* Typechecker::getExpectedEnumCase(llvm::StringRef name, Type expectedType) {
    if (!expectedType) return nullptr;
    Type candidates[] = {expectedType, expectedType.removeOptional()};
    for (Type candidate : candidates) {
        if (candidate.isEnumType()) {
            if (auto* enumCase = llvm::cast<EnumDecl>(candidate.getDecl())->getCaseByName(name)) {
                return enumCase;
            }
        }
    }
    return nullptr;
}

EnumCase* Typechecker::getEnumCase(const Expr& expr, Type expectedType, CallExpr* call) {
    auto* memberExpr = llvm::dyn_cast<MemberExpr>(&expr);
    if (!memberExpr) return nullptr;
    auto* varExpr = llvm::dyn_cast<VarExpr>(memberExpr->base);
    if (!varExpr) return nullptr;
    auto decls = findDecls(varExpr->identifier);

    Decl* enumDeclOrTemplate = nullptr;
    if (decls.size() == 1) {
        enumDeclOrTemplate = decls.front();
    } else {
        // A same-named type or function doesn't prevent enum case access, but a same-named variable takes precedence.
        for (Decl* decl : decls) {
            if (decl->isEnumDecl() || (decl->isTypeTemplate() && llvm::cast<TypeTemplate>(decl)->typeDecl->isEnumDecl())) {
                if (enumDeclOrTemplate) return nullptr; // Ambiguous.
                enumDeclOrTemplate = decl;
            } else if (decl->kind != DeclKind::TypeDecl && decl->kind != DeclKind::TypeTemplate && decl->kind != DeclKind::FunctionDecl
                       && decl->kind != DeclKind::FunctionTemplate) {
                return nullptr;
            }
        }
        if (!enumDeclOrTemplate) return nullptr;
    }

    if (auto* enumDecl = llvm::dyn_cast<EnumDecl>(enumDeclOrTemplate)) {
        if (call) validateGenericArgCount(0, call->genericArgs, enumDecl->getName(), call->location);
        auto* enumCase = enumDecl->getCaseByName(memberExpr->member);
        if (!enumCase) {
            for (auto* staticConst : enumDecl->staticConsts) {
                if (staticConst->getName() == memberExpr->member) return nullptr;
            }
            ERROR_RANGE(expr.location, expr.endLocation, "enum '" << enumDecl->getName() << "' has no case named '" << memberExpr->member << "'");
        }
        return enumCase;
    }

    if (auto* typeTemplate = llvm::dyn_cast<TypeTemplate>(enumDeclOrTemplate)) {
        if (!llvm::isa<EnumDecl>(typeTemplate->typeDecl)) return nullptr;
        return instantiateEnumCase(*typeTemplate, memberExpr->member, *memberExpr, call, expectedType);
    }

    return nullptr;
}

VarDecl* Typechecker::getStaticConst(const Expr& expr) {
    auto* memberExpr = llvm::dyn_cast<MemberExpr>(&expr);
    if (!memberExpr) return nullptr;
    auto* varExpr = llvm::dyn_cast<VarExpr>(memberExpr->base);
    if (!varExpr) return nullptr;
    auto decls = findDecls(varExpr->identifier);

    Decl* typeDeclOrNull = nullptr;
    if (decls.size() == 1) {
        typeDeclOrNull = decls.front();
    } else {
        // A same-named type doesn't prevent static access, but a same-named variable takes precedence.
        for (Decl* decl : decls) {
            if (decl->isTypeDecl()) {
                if (typeDeclOrNull) return nullptr; // Ambiguous.
                typeDeclOrNull = decl;
            } else if (decl->kind != DeclKind::TypeTemplate && decl->kind != DeclKind::FunctionDecl && decl->kind != DeclKind::FunctionTemplate) {
                return nullptr;
            }
        }
        if (!typeDeclOrNull) return nullptr;
    }

    auto* typeDecl = llvm::dyn_cast<TypeDecl>(typeDeclOrNull);
    if (!typeDecl) return nullptr;

    for (auto* staticConst : typeDecl->staticConsts) {
        if (staticConst->getName() == memberExpr->member) {
            return staticConst;
        }
    }
    return nullptr;
}

EnumCase* Typechecker::instantiateEnumCase(TypeTemplate& typeTemplate, llvm::StringRef caseName, const MemberExpr& memberExpr, CallExpr* call,
                                           Type expectedType) {
    auto* templateDecl = llvm::cast<EnumDecl>(typeTemplate.typeDecl);
    auto* templateCase = templateDecl->getCaseByName(caseName);
    if (!templateCase) {
        ERROR(memberExpr.location, "enum '" << templateDecl->getName() << "' has no case named '" << caseName << "'");
    }

    std::vector<GenericArg> inferredGenericArgs;
    llvm::ArrayRef<GenericArg> genericArgTypes;
    Type matchedExpectedType = matchEnumTemplateExpectedType(typeTemplate, expectedType);

    if (call && !call->genericArgs.empty()) {
        if (!validateGenericArgs(typeTemplate.genericParams, call->genericArgs, templateDecl->getName(), call->location)) {
            throw CompileError::dependentError();
        }
        genericArgTypes = call->genericArgs;
    } else if (matchedExpectedType) {
        genericArgTypes = matchedExpectedType.getGenericArgs();
    } else if (call && templateCase->associatedType) {
        auto params = map(templateCase->associatedType.getAnonymousStructElements(),
                          [&](const AnonymousStructElement& element) { return ParamDecl(element.type, element.name, false, templateCase->getLocation()); });
        if (call->args.size() != params.size()) {
            // Report the count error; inference can't proceed without matching arguments.
            validateAndConvertArguments(*call, params, false, templateCase->getName(), call->location);
            throw CompileError::dependentError();
        }
        inferredGenericArgs = inferGenericArgsFromCallArgs(typeTemplate.genericParams, *call, params, /*returnOnError=*/false);
        if (inferredGenericArgs.empty()) {
            ERROR(call->location, "can't infer generic parameters, please specify them explicitly");
        }
        genericArgTypes = inferredGenericArgs;
    } else {
        ERROR(memberExpr.location, "can't infer generic parameters, please specify them explicitly");
    }

    llvm::StringMap<GenericArg> genericArgs;
    auto genericArg = genericArgTypes.begin();
    for (const GenericParamDecl& genericParam : typeTemplate.genericParams) {
        genericArgs.try_emplace(genericParam.getName(), *genericArg++);
    }

    auto orderedArgs = map(typeTemplate.genericParams, [&](const GenericParamDecl& genericParam) { return genericArgs.find(genericParam.getName())->second; });
    auto qualifiedName = getQualifiedTypeName(templateDecl->getName(), orderedArgs);
    // Same-named instantiations resolve to the first one, mirroring generic struct constructor calls.
    auto existingDecls = findDecls(qualifiedName);
    EnumDecl* enumDecl = nullptr;
    for (Decl* decl : existingDecls) {
        if (auto* existing = llvm::dyn_cast<EnumDecl>(decl)) {
            enumDecl = existing;
            break;
        }
    }
    if (!enumDecl) {
        if (!existingDecls.empty()) {
            ERROR(memberExpr.location, "ambiguous reference to '" << qualifiedName << "'");
        }
        enumDecl = llvm::cast<EnumDecl>(typeTemplate.instantiate(genericArgs));
        currentModule->addToSymbolTable(*enumDecl);
        deferTypechecking(enumDecl);
    }

    auto* enumCase = enumDecl->getCaseByName(caseName);
    ASSERT(enumCase);
    return enumCase;
}

void Typechecker::setMoved(Expr* expr, bool isMoved) {
    if (auto* cast = llvm::dyn_cast<ImplicitCastExpr>(expr)) {
        if (cast->castKind == ImplicitCastExpr::OptionalWrap) {
            setMoved(cast->operand, isMoved);
        }
        return;
    }

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
        ERROR_RANGE(expr.location, expr.endLocation, "use of moved value '" << expr.identifier << "'");
    }
}
