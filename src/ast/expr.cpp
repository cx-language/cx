#include "expr.h"
#include <llvm/Support/ErrorHandling.h>
#include "decl.h"
#include "token.h"
#include "mangle.h"

using namespace delta;

bool Expr::isConstant() const {
    switch (getKind()) {
        case ExprKind::VarExpr: {
            auto* decl = llvm::cast<VarExpr>(this)->getDecl();

            if (auto* varDecl = llvm::dyn_cast<VarDecl>(decl)) {
                if (!varDecl->getType().isMutable() && varDecl->getInitializer()) {
                    return varDecl->getInitializer()->isConstant();
                }
            }

            return false;
        }

        case ExprKind::StringLiteralExpr:
        case ExprKind::CharacterLiteralExpr:
        case ExprKind::IntLiteralExpr:
        case ExprKind::FloatLiteralExpr:
        case ExprKind::BoolLiteralExpr:
        case ExprKind::NullLiteralExpr:
            return true;

        case ExprKind::ArrayLiteralExpr:
            for (auto& element : llvm::cast<ArrayLiteralExpr>(this)->getElements()) {
                if (!element->isConstant()) {
                    return false;
                }
            }
            return true;

        case ExprKind::TupleExpr:
            for (auto& element : llvm::cast<TupleExpr>(this)->getElements()) {
                if (!element->isConstant()) {
                    return false;
                }
            }
            return true;

        case ExprKind::PrefixExpr:
            return llvm::cast<PrefixExpr>(this)->getOperand().isConstant();

        case ExprKind::BinaryExpr:
            return llvm::cast<BinaryExpr>(this)->getLHS().isConstant()
                && llvm::cast<BinaryExpr>(this)->getRHS().isConstant();

        case ExprKind::CallExpr:
            return false;

        case ExprKind::CastExpr:
            return llvm::cast<CastExpr>(this)->getExpr().isConstant();

        case ExprKind::SizeofExpr:
            return false; // TODO: sizeof should be a constant expression.

        case ExprKind::MemberExpr:
        case ExprKind::SubscriptExpr:
        case ExprKind::UnwrapExpr:
        case ExprKind::LambdaExpr:
            return false;

        case ExprKind::IfExpr:
            return llvm::cast<IfExpr>(this)->getCondition()->isConstant()
                && llvm::cast<IfExpr>(this)->getThenExpr()->isConstant()
                && llvm::cast<IfExpr>(this)->getElseExpr()->isConstant();
    }
}

int64_t Expr::getConstantIntegerValue() const {
    switch (getKind()) {
        case ExprKind::VarExpr: {
            auto* decl = llvm::cast<VarExpr>(this)->getDecl();

            if (auto* varDecl = llvm::dyn_cast<VarDecl>(decl)) {
                if (!varDecl->getType().isMutable() && varDecl->getInitializer()) {
                    return varDecl->getInitializer()->getConstantIntegerValue();
                }
            }

            llvm_unreachable("not a constant integer");
        }

        case ExprKind::CharacterLiteralExpr:
            return static_cast<unsigned char>(llvm::cast<CharacterLiteralExpr>(this)->getValue());

        case ExprKind::IntLiteralExpr:
            return llvm::cast<IntLiteralExpr>(this)->getValue();

        case ExprKind::PrefixExpr:
            return llvm::cast<PrefixExpr>(this)->getConstantIntegerValue();

        case ExprKind::BinaryExpr:
            return llvm::cast<BinaryExpr>(this)->getConstantIntegerValue();

        case ExprKind::SizeofExpr:
        case ExprKind::CastExpr:
        case ExprKind::IfExpr:
            llvm_unreachable("unimplemented");

        default:
            llvm_unreachable("not a constant integer");
    }
}

bool Expr::isLvalue() const {
    switch (getKind()) {
        case ExprKind::VarExpr: case ExprKind::StringLiteralExpr: case ExprKind::CharacterLiteralExpr:
        case ExprKind::ArrayLiteralExpr: case ExprKind::TupleExpr: case ExprKind::MemberExpr:
        case ExprKind::SubscriptExpr: case ExprKind::IfExpr:
            return true;
        case ExprKind::IntLiteralExpr: case ExprKind::FloatLiteralExpr: case ExprKind::SizeofExpr:
        case ExprKind::BoolLiteralExpr: case ExprKind::CastExpr: case ExprKind::UnwrapExpr:
        case ExprKind::NullLiteralExpr: case ExprKind::BinaryExpr: case ExprKind::CallExpr:
        case ExprKind::LambdaExpr:
            return false;
        case ExprKind::PrefixExpr:
            return llvm::cast<PrefixExpr>(this)->getOperator() == STAR;
    }
    llvm_unreachable("all cases handled");
}

void Expr::setMoved(bool moved) {
    if (auto* varExpr = llvm::dyn_cast<VarExpr>(this)) {
        switch (varExpr->getDecl()->getKind()) {
            case DeclKind::ParamDecl:
                llvm::cast<ParamDecl>(varExpr->getDecl())->setMoved(moved);
                break;
            case DeclKind::VarDecl:
                llvm::cast<VarDecl>(varExpr->getDecl())->setMoved(moved);
                break;
            default:
                break;
        }
    }
}

std::unique_ptr<Expr> Expr::instantiate(const llvm::StringMap<Type>& genericArgs) const {
    std::unique_ptr<Expr> instantiation;

    switch (getKind()) {
        case ExprKind::VarExpr: {
            auto* varExpr = llvm::cast<VarExpr>(this);
            std::string identifier;

            auto it = genericArgs.find(varExpr->getIdentifier());
            if (it != genericArgs.end()) {
                identifier = it->second.getName();
            } else {
                identifier = varExpr->getIdentifier();
            }

            instantiation = llvm::make_unique<VarExpr>(std::move(identifier), varExpr->getLocation());
            llvm::cast<VarExpr>(*instantiation).setDecl(varExpr->getDecl());
            break;
        }
        case ExprKind::StringLiteralExpr: {
            auto* stringLiteralExpr = llvm::cast<StringLiteralExpr>(this);
            instantiation = llvm::make_unique<StringLiteralExpr>(stringLiteralExpr->getValue(),
                                                                 stringLiteralExpr->getLocation());
            break;
        }
        case ExprKind::CharacterLiteralExpr: {
            auto* characterLiteralExpr = llvm::cast<CharacterLiteralExpr>(this);
            instantiation = llvm::make_unique<CharacterLiteralExpr>(characterLiteralExpr->getValue(),
                                                                    characterLiteralExpr->getLocation());
            break;
        }
        case ExprKind::IntLiteralExpr: {
            auto* intLiteralExpr = llvm::cast<IntLiteralExpr>(this);
            instantiation = llvm::make_unique<IntLiteralExpr>(intLiteralExpr->getValue(),
                                                              intLiteralExpr->getLocation());
            break;
        }
        case ExprKind::FloatLiteralExpr: {
            auto* floatLiteralExpr = llvm::cast<FloatLiteralExpr>(this);
            instantiation = llvm::make_unique<FloatLiteralExpr>(floatLiteralExpr->getValue(),
                                                                floatLiteralExpr->getLocation());
            break;
        }
        case ExprKind::BoolLiteralExpr: {
            auto* boolLiteralExpr = llvm::cast<BoolLiteralExpr>(this);
            instantiation = llvm::make_unique<BoolLiteralExpr>(boolLiteralExpr->getValue(),
                                                               boolLiteralExpr->getLocation());
            break;
        }
        case ExprKind::NullLiteralExpr: {
            auto* nullLiteralExpr = llvm::cast<NullLiteralExpr>(this);
            instantiation = llvm::make_unique<NullLiteralExpr>(nullLiteralExpr->getLocation());
            break;
        }
        case ExprKind::ArrayLiteralExpr: {
            auto* arrayLiteralExpr = llvm::cast<ArrayLiteralExpr>(this);
            auto elements = ::instantiate(arrayLiteralExpr->getElements(), genericArgs);
            instantiation = llvm::make_unique<ArrayLiteralExpr>(std::move(elements),
                                                                arrayLiteralExpr->getLocation());
            break;
        }
        case ExprKind::TupleExpr: {
            auto* tupleExpr = llvm::cast<TupleExpr>(this);
            auto elements = ::instantiate(tupleExpr->getElements(), genericArgs);
            instantiation = llvm::make_unique<TupleExpr>(std::move(elements), tupleExpr->getLocation());
            break;
        }
        case ExprKind::PrefixExpr: {
            auto* prefixExpr = llvm::cast<PrefixExpr>(this);
            auto operand = prefixExpr->getOperand().instantiate(genericArgs);
            instantiation = llvm::make_unique<PrefixExpr>(prefixExpr->getOperator(), std::move(operand),
                                                          prefixExpr->getLocation());
            break;
        }
        case ExprKind::BinaryExpr: {
            auto* binaryExpr = llvm::cast<BinaryExpr>(this);
            auto lhs = binaryExpr->getLHS().instantiate(genericArgs);
            auto rhs = binaryExpr->getRHS().instantiate(genericArgs);
            instantiation = llvm::make_unique<BinaryExpr>(binaryExpr->getOperator(), std::move(lhs),
                                                          std::move(rhs), binaryExpr->getLocation());
            break;
        }
        case ExprKind::CallExpr: {
            auto* callExpr = llvm::cast<CallExpr>(this);
            auto callee = callExpr->getCallee().instantiate(genericArgs);
            auto args = map(callExpr->getArgs(), [&](const Argument& arg) {
                return Argument(arg.getName(), arg.getValue()->instantiate(genericArgs));
            });
            auto callGenericArgs = map(callExpr->getGenericArgs(), [&](Type type) {
                return type.resolve(genericArgs);
            });
            instantiation = llvm::make_unique<CallExpr>(std::move(callee), std::move(args),
                                                        std::move(callGenericArgs),
                                                        callExpr->getLocation());
            if (auto* callee = callExpr->getCalleeDecl()) {
                llvm::cast<CallExpr>(*instantiation).setCalleeDecl(callee);
            }
            Type receiverType = callExpr->getReceiverType().resolve(genericArgs);
            llvm::cast<CallExpr>(*instantiation).setReceiverType(receiverType);
            break;
        }
        case ExprKind::CastExpr: {
            auto* castExpr = llvm::cast<CastExpr>(this);
            auto targetType = castExpr->getTargetType().resolve(genericArgs);
            auto expr = castExpr->getExpr().instantiate(genericArgs);
            instantiation = llvm::make_unique<CastExpr>(targetType, std::move(expr),
                                                        castExpr->getLocation());
            break;
        }
        case ExprKind::SizeofExpr: {
            auto* sizeofExpr = llvm::cast<SizeofExpr>(this);
            auto type = sizeofExpr->getType().resolve(genericArgs);
            instantiation = llvm::make_unique<SizeofExpr>(type, sizeofExpr->getLocation());
            break;
        }
        case ExprKind::MemberExpr: {
            auto* memberExpr = llvm::cast<MemberExpr>(this);
            auto base = memberExpr->getBaseExpr()->instantiate(genericArgs);
            instantiation = llvm::make_unique<MemberExpr>(std::move(base), memberExpr->getMemberName(),
                                                          memberExpr->getLocation());
            break;
        }
        case ExprKind::SubscriptExpr: {
            auto* subscriptExpr = llvm::cast<SubscriptExpr>(this);
            auto base = subscriptExpr->getBaseExpr()->instantiate(genericArgs);
            auto index = subscriptExpr->getIndexExpr()->instantiate(genericArgs);
            instantiation = llvm::make_unique<SubscriptExpr>(std::move(base), std::move(index),
                                                             subscriptExpr->getLocation());
            break;
        }
        case ExprKind::UnwrapExpr: {
            auto* unwrapExpr = llvm::cast<UnwrapExpr>(this);
            auto operand = unwrapExpr->getOperand().instantiate(genericArgs);
            instantiation = llvm::make_unique<UnwrapExpr>(std::move(operand), unwrapExpr->getLocation());
            break;
        }
        case ExprKind::LambdaExpr: {
            auto* lambdaExpr = llvm::cast<LambdaExpr>(this);
            auto params = map(lambdaExpr->getParams(), [&](const ParamDecl& p) {
                return ParamDecl(p.getType().resolve(genericArgs), p.getName(), p.getLocation());
            });
            auto body = lambdaExpr->getBody()->instantiate(genericArgs);
            instantiation = llvm::make_unique<LambdaExpr>(std::move(params), std::move(body),
                                                          lambdaExpr->getLocation());
            break;
        }
        case ExprKind::IfExpr: {
            auto* ifExpr = llvm::cast<IfExpr>(this);
            auto condition = ifExpr->getCondition()->instantiate(genericArgs);
            auto thenExpr = ifExpr->getThenExpr()->instantiate(genericArgs);
            auto elseExpr = ifExpr->getElseExpr()->instantiate(genericArgs);
            instantiation = llvm::make_unique<IfExpr>(std::move(condition), std::move(thenExpr),
                                                      std::move(elseExpr), ifExpr->getLocation());
        }
    }

    if (hasType()) {
        instantiation->setType(getType());
    }

    return instantiation;
}

llvm::StringRef CallExpr::getFunctionName() const {
    switch (getCallee().getKind()) {
        case ExprKind::VarExpr: return llvm::cast<VarExpr>(getCallee()).getIdentifier();
        case ExprKind::MemberExpr: return llvm::cast<MemberExpr>(getCallee()).getMemberName();
        default: return "(anonymous function)";
    }
}

static Type getReceiverType(const CallExpr& call) {
    if (call.getCallee().isMemberExpr()) {
        return call.getReceiver()->getType().removeOptional().removePointer();
    }
    return nullptr;
}

std::string CallExpr::getMangledFunctionName() const {
    return mangleFunctionDecl(::getReceiverType(*this), getFunctionName());
}

bool CallExpr::isMoveInit() const {
    if (getFunctionName() != "init") return false;
    if (getArgs().size() != 1) return false;

    if (Type receiverType = ::getReceiverType(*this)) {
        return getArgs()[0].getValue()->getType().asImmutable() == receiverType.asImmutable();
    }

    return false;
}

const Expr* CallExpr::getReceiver() const {
    if (!isMethodCall()) return nullptr;
    return llvm::cast<MemberExpr>(getCallee()).getBaseExpr();
}

Expr* CallExpr::getReceiver() {
    if (!isMethodCall()) return nullptr;
    return llvm::cast<MemberExpr>(getCallee()).getBaseExpr();
}

int64_t PrefixExpr::getConstantIntegerValue() const {
    auto operand = getOperand().getConstantIntegerValue();

    switch (getOperator()) {
        case PLUS: return operand;
        case MINUS: return -operand;
        case COMPL: return ~operand;
        default: llvm_unreachable("invalid constant integer prefix operator");
    }
}

bool BinaryExpr::isBuiltinOp() const {
    if (op == DOTDOT || op == DOTDOTDOT) return false;
    if (getLHS().getType().isPointerType() && getRHS().getType().isPointerType()) {
        return getLHS().getType().getPointee().isVoid() && getRHS().getType().getPointee().isVoid();
    }
    if (getLHS().getType().isEnumType() && getLHS().getType() == getRHS().getType()) return true;
    return getLHS().getType().isBuiltinType() && getRHS().getType().isBuiltinType();
}

int64_t BinaryExpr::getConstantIntegerValue() const {
    // TODO: Add overflow checks.
    // TODO: Handle signedness for '>>' operator;

    auto lhs = getLHS().getConstantIntegerValue();
    auto rhs = getRHS().getConstantIntegerValue();

    switch (getOperator()) {
        case PLUS: return lhs + rhs;
        case MINUS: return lhs - rhs;
        case STAR: return lhs * rhs;
        case SLASH: return lhs / rhs;
        case MOD: return lhs % rhs;
        case AND: return lhs & rhs;
        case OR: return lhs | rhs;
        case XOR: return lhs ^ rhs;
        case LSHIFT: return lhs << rhs;
        case RSHIFT: return lhs >> rhs;
        default: llvm_unreachable("invalid constant integer binary operator");
    }
}

std::unique_ptr<FunctionDecl> LambdaExpr::lower(Module& module) const {
    static uint64_t nameCounter = 0;

    FunctionProto proto("__lambda" + std::to_string(nameCounter++), std::vector<ParamDecl>(params),
                        body->getType(), false, false);
    auto functionDecl = llvm::make_unique<FunctionDecl>(std::move(proto), std::vector<Type>(),
                                                        module, getLocation());
    std::vector<std::unique_ptr<Stmt>> body;
    auto returnValue = getBody()->instantiate({});
    body.push_back(llvm::make_unique<ReturnStmt>(std::move(returnValue), getBody()->getLocation()));
    functionDecl->setBody(std::move(body));
    return functionDecl;
}
