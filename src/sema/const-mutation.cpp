#include "../ast/arena.h"
#include "../ast/ast.h"
#include "../ast/decl.h"
#include "../ast/expr.h"
#include "../ast/module.h"
#include "../ast/type.h"
#include "typecheck.h"

using namespace cx;

// Infers whether a method may mutate its receiver, so calls on constants to
// mutating methods can be rejected while reads stay legal. A method mutates
// when its body (transitively) writes through `this`, passes `this`-derived
// values to channels the callee may write (borrow/pointer/view bindings), or
// calls a method that does. The callee's own body is the oracle: a borrow of
// `this` handed to a function that never writes through it is not a mutation,
// so view-taking constructors (iterators) and read-only helpers infer clean.
//
// Roots: `this` plus parameters bound to caller-side roots (transparent: the
// parameter IS the caller's storage) are storage roots; locals holding fresh
// wrappers derived from roots (iterators) are view roots. Writes through
// either count, but a call only propagates the callee's self-mutation when
// its receiver is a storage root: mutating a fresh temp (toList().push()) is
// not our mutation. Storing into a bare binding is never a write through,
// except for a field of a rooted receiver.
//
// Known limits: writes through a method's own view fields reached via a
// local view, methods that copy an alias-capable temp and then mutate the
// copy (the temp still counts as rooted), and lambdas stored but never
// called (their bodies still count). Copies are assumed to disconnect
// unless the type structurally holds a view: cx has no shared-ownership
// interior mutability (Rc/Cell), so a by-value copy cannot write back.

namespace {

struct RootCtx {
    bool thisRooted;
    // Whether returning a rooted value counts: set when the walked value must
    // not escape (a callee parameter), clear when returns are governed
    // elsewhere (a method result over a constant receiver). Lexical only:
    // nested calls judge their returns where they land, so the flag does not
    // propagate into them.
    bool countReturns;
    // Whether the receiver is under construction: stores into its fields land
    // in the fresh product, which the caller judges, so they never escape.
    bool thisIsFresh;
    llvm::SmallPtrSet<const Decl*, 16>& storage;
    llvm::SmallPtrSet<const Decl*, 16>& views;
};

bool stmtMayWrite(Stmt& stmt, RootCtx& ctx, ConstMutationQuery& query);
bool exprMayWrite(Expr& expr, RootCtx& ctx, ConstMutationQuery& query);
bool aliasesRoots(const Expr& expr, const RootCtx& ctx, bool storageOnly);
bool mayWrite(FunctionDecl& func, bool thisRooted, bool countReturns, std::vector<const Decl*> marked, std::vector<const Decl*> markedViews,
              ConstMutationQuery& query);
bool isTransparentAliasType(Type type);

// IndexAssignmentExpr extends IndexExpr but has its own kind, so casting to
// IndexExpr would assert; test both spellings explicitly.
const Expr* getIndexBase(const Expr& expr) {
    if (auto* indexAssign = llvm::dyn_cast<IndexAssignmentExpr>(&expr)) return indexAssign->getBase();
    if (auto* index = llvm::dyn_cast<IndexExpr>(&expr)) return index->getBase();
    return nullptr;
}

Expr* getIndexBase(Expr& expr) {
    if (auto* indexAssign = llvm::dyn_cast<IndexAssignmentExpr>(&expr)) return indexAssign->getBase();
    if (auto* index = llvm::dyn_cast<IndexExpr>(&expr)) return index->getBase();
    return nullptr;
}

// The variable ultimately designated by an assignment target, peeling member,
// index, and unwrap projections; null for dereferences and other shapes.
VarExpr* assignedBaseVar(Expr& lhs) {
    Expr* current = &lhs;
    while (true) {
        if (auto* var = llvm::dyn_cast<VarExpr>(current)) return var;
        if (auto* member = llvm::dyn_cast<MemberExpr>(current)) {
            current = member->base;
            continue;
        }
        if (auto* unwrap = llvm::dyn_cast<UnwrapExpr>(current)) {
            current = unwrap->getReceiver();
            continue;
        }
        if (auto* base = getIndexBase(*current)) {
            current = base;
            continue;
        }
        return nullptr;
    }
}

// Whether storing rhs into lhs leaks caller-side roots out of the analyzed
// body: rhs is rooted and alias-capable, and lhs designates storage the
// caller can reach afterwards (a global, a borrow parameter's referent, a
// dereference, or a field of a caller-owned receiver). Stores into locals
// and by-value parameters are tracked by marking instead (see
// collectViewRoots), as are stores into a fresh product under construction.
bool storeMayEscape(Expr& lhs, Expr& rhs, const RootCtx& ctx) {
    if (!rhs.hasType() || !typeMayAliasStorageDeep(rhs.type)) return false;
    if (!aliasesRoots(rhs, ctx, false)) return false;
    if (auto* unary = llvm::dyn_cast<UnaryExpr>(&lhs); unary && unary->op == Token::Star) return true;
    auto* base = assignedBaseVar(lhs);
    if (!base || !base->decl) return true;
    if (base->identifier == "this" || llvm::isa<FieldDecl>(base->decl)) return !ctx.thisIsFresh;
    if (auto* param = llvm::dyn_cast<ParamDecl>(base->decl)) return param->type.isBorrowOrOptionalBorrow();
    if (auto* var = llvm::dyn_cast<VarDecl>(base->decl)) return var->isGlobal();
    return true;
}

// Whether the expression may designate root storage: chains over member,
// index, unwrap, dereference, address-of, and cast projections, plus calls
// whose alias-capable result derives from a rooted receiver or argument.
// storageOnly restricts to storage roots: transparent-typed members,
// elements, and calls of rooted inputs designate storage, while fresh
// wrappers (iterators) are views, never storage. Dereference, unwrap,
// and casts preserve the mode into their operand.
bool bindingDesignatesStorage(const Decl* decl, const RootCtx& ctx, bool storageOnly) {
    if (ctx.storage.contains(decl)) return true;
    if (!ctx.views.contains(decl)) return false;
    if (!storageOnly) return true;
    auto* var = llvm::dyn_cast<VariableDecl>(decl);
    return var && var->type && isTransparentAliasType(var->type);
}

bool aliasesRoots(const Expr& expr, const RootCtx& ctx, bool storageOnly) {
    const Expr* current = &expr;
    while (true) {
        if (auto* varExpr = llvm::dyn_cast<VarExpr>(current)) {
            if (varExpr->identifier == "this") return ctx.thisRooted;
            if (!varExpr->decl) return false;
            // A bare field names the receiver's own storage.
            if (ctx.thisRooted && llvm::isa<FieldDecl>(varExpr->decl)) return true;
            return bindingDesignatesStorage(varExpr->decl, ctx, storageOnly);
        }
        if (auto* varDeclExpr = llvm::dyn_cast<VarDeclExpr>(current)) {
            if (!varDeclExpr->varDecl) return false;
            return bindingDesignatesStorage(varDeclExpr->varDecl, ctx, storageOnly);
        }
        // Explicit kind check: CallExpr::classof also matches Unary, Binary,
        // Index, and Unwrap expressions, which are handled on their own.
        if (current->kind == ExprKind::CallExpr) {
            auto* call = llvm::cast<CallExpr>(current);
            if (!call->hasType() || !typeMayAliasStorageDeep(call->type)) return false;
            if ((!call->isMethodCall() || !aliasesRoots(*call->getReceiver(), ctx, false))
                && !llvm::any_of(call->args, [&](const NamedValue& arg) { return aliasesRoots(*arg.value, ctx, false); }))
                return false;
            if (isTransparentAliasType(call->type)) return true;
            return !storageOnly;
        }
        if (auto* memberExpr = llvm::dyn_cast<MemberExpr>(current)) {
            // A transparent-typed member of a rooted base designates storage.
            if (storageOnly && memberExpr->hasType() && isTransparentAliasType(memberExpr->type)) return aliasesRoots(*memberExpr->base, ctx, false);
            current = memberExpr->base;
            continue;
        }
        if (const Expr* indexBase = getIndexBase(*current)) {
            // A transparent-typed element of a rooted base designates storage.
            if (storageOnly && current->hasType() && isTransparentAliasType(current->type)) return aliasesRoots(*indexBase, ctx, false);
            current = indexBase;
            continue;
        }
        if (auto* unwrapExpr = llvm::dyn_cast<UnwrapExpr>(current)) {
            current = unwrapExpr->getReceiver();
            continue;
        }
        if (auto* unaryExpr = llvm::dyn_cast<UnaryExpr>(current)) {
            if (unaryExpr->op == Token::Star) {
                current = &unaryExpr->getOperand();
                continue;
            }
            if (unaryExpr->op == Token::And) return aliasesRoots(unaryExpr->getOperand(), ctx, storageOnly);
            return false;
        }
        if (auto* castExpr = llvm::dyn_cast<ImplicitCastExpr>(current)) {
            current = castExpr->operand;
            continue;
        }
        if (auto* ifExpr = llvm::dyn_cast<IfExpr>(current)) {
            return aliasesRoots(*ifExpr->thenExpr, ctx, storageOnly) || aliasesRoots(*ifExpr->elseExpr, ctx, storageOnly);
        }
        if (auto* switchExpr = llvm::dyn_cast<SwitchExpr>(current)) {
            return llvm::any_of(switchExpr->arms, [&](const SwitchExprArm& arm) { return aliasesRoots(*arm.expr, ctx, storageOnly); })
                || (switchExpr->defaultExpr && aliasesRoots(*switchExpr->defaultExpr, ctx, storageOnly));
        }
        if (auto* arrayLit = llvm::dyn_cast<ArrayLiteralExpr>(current)) {
            return llvm::any_of(arrayLit->elements, [&](const Expr* element) { return aliasesRoots(*element, ctx, storageOnly); });
        }
        if (auto* structExpr = llvm::dyn_cast<AnonymousStructExpr>(current)) {
            return llvm::any_of(structExpr->elements, [&](const NamedValue& element) { return aliasesRoots(*element.value, ctx, storageOnly); });
        }
        return false;
    }
}

// Whether a type structurally holds an alias channel: borrows, pointers, and
// views, including inside struct fields, fixed-array elements, anonymous
// structs, and enum payloads. Copies of such values (slices, strings) still
// share their target, unlike copies of plain data.
bool typeHasViewFields(Type type, llvm::SmallPtrSetImpl<const TypeDecl*>& visiting) {
    if (!type) return false;
    if (typeMayAliasStorage(type)) return true;
    Type stripped = type.removeOptional();
    if (stripped.isFixedArray()) return typeHasViewFields(stripped.getElementType(), visiting);
    if (stripped.isAnonymousStructType()) {
        return llvm::any_of(stripped.getAnonymousStructElements(),
                            [&](const AnonymousStructElement& element) { return typeHasViewFields(element.type, visiting); });
    }
    // An owned List aliases only through its elements, like a fixed array.
    if (stripped.isBasicType() && stripped.getName() == "List" && stripped.getGenericArgs().size() == 1 && stripped.getGenericArgs()[0].isType()) {
        return typeHasViewFields(stripped.getGenericArgs()[0].getType(), visiting);
    }
    auto* typeDecl = stripped.removePointer().getDecl();
    if (!typeDecl) return false;
    // An interface-typed value boxes an unknown implementation, which may alias.
    if (typeDecl->isInterface()) return true;
    if (!visiting.insert(typeDecl).second) return false;
    bool result = false;
    if (auto* enumDecl = llvm::dyn_cast<EnumDecl>(typeDecl)) {
        result = llvm::any_of(enumDecl->cases, [&](const EnumCase& enumCase) { return typeHasViewFields(enumCase.associatedType, visiting); });
    } else if (auto* structDecl = llvm::dyn_cast<TypeDecl>(typeDecl)) {
        result = llvm::any_of(structDecl->fields, [&](const FieldDecl& field) { return typeHasViewFields(field.type, visiting); });
    }
    visiting.erase(typeDecl);
    return result;
}

// Whether the type is a transparent alias: a borrow, pointer, or builtin
// view, or a copyable view struct worked through a subscript operator (a
// slice copy shares its buffer, so indexing it reaches the source). Other
// alias-capable values (iterators) are fresh wrappers: their own storage is
// new even though their contents reach back.
bool isTransparentAliasType(Type type) {
    if (!type) return false;
    Type stripped = type.removeOptional();
    if (stripped.isReferenceType() || stripped.isPointerOrArrayPointer() || stripped.isSlice()) return true;
    auto* typeDecl = stripped.removePointer().getDecl();
    auto* structDecl = llvm::dyn_cast_or_null<TypeDecl>(typeDecl);
    if (!structDecl) return false;
    bool hasSubscript = llvm::any_of(structDecl->methods, [](Decl* method) {
        auto name = method->getName();
        return name == "[]" || name == "[]=" || name == "[-]" || name == "[-]=";
    });
    return hasSubscript && typeMayAliasStorageDeep(type);
}

// Whether binding an argument to a parameter may let the callee write the
// caller's storage: either side is an alias channel (a view passed by value
// still shares its target). Value copies of plain data disconnect.
bool bindingMayWrite(Type argType, Type paramType) {
    return typeMayAliasStorageDeep(argType) || typeMayAliasStorageDeep(paramType);
}

// Whether a parameter bound to caller-side roots designates that storage as
// itself (a borrow, pointer, or builtin view) rather than wrapping it (a
// struct or union object): only the former propagates the callee's
// self-mutation, since mutating a fresh temp is harmless.
bool bindingIsStorage(Type paramType) {
    if (!paramType) return false;
    Type stripped = paramType.removeOptional();
    if (stripped.isReferenceType() || stripped.isPointerType()) return true;
    if (isTransparentAliasType(stripped)) return true;
    if (auto* structDecl = llvm::dyn_cast_or_null<TypeDecl>(stripped.removePointer().getDecl())) {
        return !structDecl->isStruct() && !structDecl->isUnion();
    }
    return true;
}

// Whether writing to an assignment or increment target may write root
// storage. Storing into a bare binding only replaces the callee-owned slot,
// except for a field of a rooted receiver; other targets designate storage
// only through transparent chains into storage roots (a field of a fresh
// wrapper is itself callee-owned).
bool writeTargetMayWrite(const Expr& target, const RootCtx& ctx) {
    if (auto* varExpr = llvm::dyn_cast<VarExpr>(&target)) {
        return ctx.thisRooted && varExpr->decl && llvm::isa<FieldDecl>(varExpr->decl);
    }
    if (llvm::isa<VarDeclExpr>(&target)) return false;
    // Slot question: peel the outermost member; the slot is caller storage
    // iff the base designates it. A transparent-typed outermost member
    // still names a slot of the base, not the referent.
    if (auto* memberExpr = llvm::dyn_cast<MemberExpr>(&target)) return aliasesRoots(*memberExpr->base, ctx, true);
    return aliasesRoots(target, ctx, true);
}

// Parameters of a call through a function-typed value (local, parameter, or
// field): the type carries the signature since no declaration body exists.
llvm::ArrayRef<Type> variableCalleeParamTypes(const VariableDecl& callee) {
    if (!callee.type) return {};
    if (callee.type.isClosureType()) return callee.type.getClosureParamTypes();
    if (callee.type.isFunctionType()) return callee.type.getParamTypes();
    return {};
}

// Marks for the callee's parameters: which rooted arguments bind through
// an alias channel. Variadic extras have no parameter; an alias-capable one
// reports directly since C varargs pass the pointer itself.
bool collectArgMarks(CallExpr& call, llvm::ArrayRef<ParamDecl> params, const RootCtx& ctx, std::vector<const Decl*>& marked,
                     std::vector<const Decl*>& markedViews) {
    for (size_t i = 0; i < call.args.size(); ++i) {
        if (!aliasesRoots(*call.args[i].value, ctx, false)) continue;
        int paramIndex = call.paramIndexForArg(i);
        if (paramIndex < 0 || size_t(paramIndex) >= params.size()) {
            if (typeMayAliasStorageDeep(call.args[i].value->type)) return true;
            continue;
        }
        Type argType = call.args[i].value->hasType() ? call.args[i].value->type : Type();
        if (bindingMayWrite(argType, params[size_t(paramIndex)].type)) {
            if (bindingIsStorage(params[size_t(paramIndex)].type))
                marked.push_back(&params[size_t(paramIndex)]);
            else
                markedViews.push_back(&params[size_t(paramIndex)]);
        }
    }
    return false;
}

// Signature-only verdict for calls with no body to walk (opaque function
// values, extern declarations): an argument bound through an alias channel
// may be written.
bool opaqueCallMayWrite(CallExpr& call, llvm::ArrayRef<Type> paramTypes, const RootCtx& ctx) {
    for (size_t i = 0; i < call.args.size(); ++i) {
        if (!aliasesRoots(*call.args[i].value, ctx, false)) continue;
        int paramIndex = call.paramIndexForArg(i);
        if (paramIndex < 0 || size_t(paramIndex) >= paramTypes.size()) {
            if (typeMayAliasStorageDeep(call.args[i].value->type)) return true;
            continue;
        }
        Type argType = call.args[i].value->hasType() ? call.args[i].value->type : Type();
        if (bindingMayWrite(argType, paramTypes[size_t(paramIndex)])) return true;
    }
    return false;
}

// Walks a call's receiver and arguments for nested effects. Lambda arguments
// called with rooted input (rooted receiver or co-argument) bind their
// alias-capable parameters to root storage while their body walks.
bool walkCallOperands(CallExpr& call, Expr* receiver, RootCtx& ctx, ConstMutationQuery& query) {
    bool receiverRooted = receiver && aliasesRoots(*receiver, ctx, false);
    std::vector<char> argRooted(call.args.size());
    for (size_t i = 0; i < call.args.size(); ++i)
        argRooted[i] = aliasesRoots(*call.args[i].value, ctx, false);
    if (receiver && exprMayWrite(*receiver, ctx, query)) return true;
    for (size_t i = 0; i < call.args.size(); ++i) {
        auto* lambda = llvm::dyn_cast<LambdaExpr>(call.args[i].value);
        if (!lambda || !lambda->functionDecl || !lambda->functionDecl->body) {
            if (exprMayWrite(*call.args[i].value, ctx, query)) return true;
            continue;
        }
        bool rootedInput = receiverRooted;
        for (size_t j = 0; j < call.args.size() && !rootedInput; ++j)
            rootedInput = j != i && argRooted[j];
        std::vector<const Decl*> addedStorage;
        std::vector<const Decl*> addedViews;
        if (rootedInput) {
            for (auto& param : lambda->functionDecl->getParams()) {
                if (!typeMayAliasStorageDeep(param.type)) continue;
                if (isTransparentAliasType(param.type)) {
                    if (ctx.storage.insert(&param).second) addedStorage.push_back(&param);
                } else if (ctx.views.insert(&param).second) {
                    addedViews.push_back(&param);
                }
            }
        }
        bool writes = llvm::any_of(*lambda->functionDecl->body, [&](Stmt* stmt) { return stmtMayWrite(*stmt, ctx, query); });
        for (auto* decl : addedStorage)
            ctx.storage.erase(decl);
        for (auto* decl : addedViews)
            ctx.views.erase(decl);
        if (writes) return true;
    }
    return false;
}

// Whether an unresolved call is a pure compiler intrinsic: slice data() and
// sizeof size() return early from call checking without resolving (see
// typecheckCallExpr), and unwrap only asserts and projects. New intrinsics
// fail safe: unknown unresolved calls still report.
bool isPureIntrinsic(CallExpr& call, Expr* receiver) {
    if (call.kind == ExprKind::UnwrapExpr) return true;
    if (!receiver || !receiver->hasType()) return false;
    Type receiverType = receiver->type.removeOptional();
    if (call.getFunctionName() == "data" && receiverType.isArrayType() && !receiverType.isFixedArray()) return true;
    if (call.getFunctionName() == "size" && receiverType.removePointer().hasSizeofArraySize()) return true;
    return false;
}

// Shared callee dispatch for calls, operator overloads, and unwraps:
// methods recurse with this rooted when the receiver is storage-rooted,
// constructors build fresh objects (a storage-rooted one reinitializes in
// place, which is a write), and anything opaque falls back to signatures.
bool callMayWrite(CallExpr& call, Expr* receiver, RootCtx& ctx, ConstMutationQuery& query) {
    Decl* callee = call.calleeDecl;
    bool storageReceiver = receiver && aliasesRoots(*receiver, ctx, true);
    if (!callee) {
        // Unresolved calls are builtins, which neither resolve nor mutate, or
        // pure compiler intrinsics (slice data(), sizeof size(), unwrap);
        // anything else unresolved is already broken.
        if (!call.isBuiltinConversion() && !call.isBuiltinCast() && !isPureIntrinsic(call, receiver)) {
            if (storageReceiver) return true;
            for (auto& arg : call.args) {
                if (aliasesRoots(*arg.value, ctx, false) && typeMayAliasStorageDeep(arg.value->type)) return true;
            }
        }
        return walkCallOperands(call, receiver, ctx, query);
    }
    if (llvm::isa<ConstructorDecl>(callee)) {
        if (storageReceiver) return true;
        auto* func = llvm::cast<FunctionDecl>(callee);
        if (!func->body) return walkCallOperands(call, receiver, ctx, query);
        std::vector<const Decl*> marked;
        std::vector<const Decl*> markedViews;
        if (collectArgMarks(call, func->getParams(), ctx, marked, markedViews)) return true;
        if (mayWrite(*func, false, /*countReturns=*/false, std::move(marked), std::move(markedViews), query)) return true;
        return walkCallOperands(call, receiver, ctx, query);
    }
    if (auto* func = llvm::dyn_cast<FunctionDecl>(callee)) {
        bool isMethod = func->isMethodDecl();
        if (!func->body) {
            if (isMethod && storageReceiver) return true;
            std::vector<Type> paramTypes;
            for (auto& param : func->getParams())
                paramTypes.push_back(param.type);
            if (opaqueCallMayWrite(call, paramTypes, ctx)) return true;
            return walkCallOperands(call, receiver, ctx, query);
        }
        std::vector<const Decl*> marked;
        std::vector<const Decl*> markedViews;
        if (collectArgMarks(call, func->getParams(), ctx, marked, markedViews)) return true;
        if (mayWrite(*func, isMethod && storageReceiver, /*countReturns=*/false, std::move(marked), std::move(markedViews), query)) return true;
        return walkCallOperands(call, receiver, ctx, query);
    }
    if (auto* varCallee = llvm::dyn_cast<VariableDecl>(callee)) {
        // A function-typed field may capture root storage; calling through a
        // storage-rooted one may run it.
        if (storageReceiver) return true;
        if (opaqueCallMayWrite(call, variableCalleeParamTypes(*varCallee), ctx)) return true;
        return walkCallOperands(call, receiver, ctx, query);
    }
    // Enum-case construction and other value forms build fresh values.
    if (storageReceiver) return true;
    return walkCallOperands(call, receiver, ctx, query);
}

bool exprMayWrite(Expr& expr, RootCtx& ctx, ConstMutationQuery& query) {
    switch (expr.kind) {
    case ExprKind::CallExpr: {
        auto& call = llvm::cast<CallExpr>(expr);
        Expr* receiver = call.isMethodCall() ? call.getReceiver() : nullptr;
        return callMayWrite(call, receiver, ctx, query);
    }
    case ExprKind::BinaryExpr: {
        auto& binary = llvm::cast<BinaryExpr>(expr);
        if (isAssignmentOperator(binary.op.kind)) {
            if (writeTargetMayWrite(binary.getLHS(), ctx)) return true;
            if (storeMayEscape(binary.getLHS(), binary.getRHS(), ctx)) return true;
            return exprMayWrite(binary.getLHS(), ctx, query) || exprMayWrite(binary.getRHS(), ctx, query);
        }
        // Overloaded operators resolve to free functions of both operands.
        if (binary.calleeDecl) {
            Expr* receiver = llvm::isa<MethodDecl>(binary.calleeDecl) ? &binary.getLHS() : nullptr;
            return callMayWrite(binary, receiver, ctx, query);
        }
        return exprMayWrite(binary.getLHS(), ctx, query) || exprMayWrite(binary.getRHS(), ctx, query);
    }
    case ExprKind::UnaryExpr: {
        auto& unary = llvm::cast<UnaryExpr>(expr);
        // No unary operator overloads exist; the callee never resolves.
        if ((unary.op == Token::Increment || unary.op == Token::Decrement) && writeTargetMayWrite(unary.getOperand(), ctx)) return true;
        return exprMayWrite(unary.getOperand(), ctx, query);
    }
    case ExprKind::IndexExpr: {
        auto& index = llvm::cast<IndexExpr>(expr);
        if (index.calleeDecl) return callMayWrite(index, index.getBase(), ctx, query);
        return exprMayWrite(*index.getBase(), ctx, query) || exprMayWrite(*index.getIndex(), ctx, query);
    }
    case ExprKind::IndexAssignmentExpr: {
        auto& indexAssign = llvm::cast<IndexAssignmentExpr>(expr);
        // The callee is always a `[]=` overload (a missing one is a check
        // error); its body decides, with the value already an argument.
        if (indexAssign.calleeDecl) return callMayWrite(indexAssign, indexAssign.getBase(), ctx, query);
        // The base designates storage only through transparent chains (a
        // local array holding roots is itself callee-owned).
        if (aliasesRoots(*indexAssign.getBase(), ctx, true)) return true;
        return exprMayWrite(*indexAssign.getBase(), ctx, query) || exprMayWrite(*indexAssign.getIndex(), ctx, query)
            || exprMayWrite(*indexAssign.getValue(), ctx, query);
    }
    case ExprKind::UnwrapExpr: {
        auto& unwrap = llvm::cast<UnwrapExpr>(expr);
        return callMayWrite(unwrap, unwrap.getReceiver(), ctx, query);
    }
    case ExprKind::MemberExpr:
        return exprMayWrite(*llvm::cast<MemberExpr>(expr).base, ctx, query);
    case ExprKind::LambdaExpr: {
        // Direct call arguments are walked with marks by the call itself;
        // elsewhere the body walks under the current roots (a stored writing
        // lambda counts even if never invoked).
        auto* functionDecl = llvm::cast<LambdaExpr>(expr).functionDecl;
        if (!functionDecl || !functionDecl->body) return false;
        return llvm::any_of(*functionDecl->body, [&](Stmt* stmt) { return stmtMayWrite(*stmt, ctx, query); });
    }
    case ExprKind::IfExpr: {
        auto& ifExpr = llvm::cast<IfExpr>(expr);
        return exprMayWrite(*ifExpr.condition, ctx, query) || exprMayWrite(*ifExpr.thenExpr, ctx, query) || exprMayWrite(*ifExpr.elseExpr, ctx, query);
    }
    case ExprKind::SwitchExpr: {
        auto& switchExpr = llvm::cast<SwitchExpr>(expr);
        if (exprMayWrite(*switchExpr.condition, ctx, query)) return true;
        for (auto& arm : switchExpr.arms) {
            if (exprMayWrite(*arm.value, ctx, query) || exprMayWrite(*arm.expr, ctx, query)) return true;
        }
        return switchExpr.defaultExpr && exprMayWrite(*switchExpr.defaultExpr, ctx, query);
    }
    case ExprKind::ImplicitCastExpr:
        return exprMayWrite(*llvm::cast<ImplicitCastExpr>(expr).operand, ctx, query);
    case ExprKind::ArrayLiteralExpr:
        return llvm::any_of(llvm::cast<ArrayLiteralExpr>(expr).elements, [&](Expr* element) { return exprMayWrite(*element, ctx, query); });
    case ExprKind::AnonymousStructExpr:
        return llvm::any_of(llvm::cast<AnonymousStructExpr>(expr).elements,
                            [&](const NamedValue& element) { return exprMayWrite(*element.value, ctx, query); });
    case ExprKind::VarExpr:
    case ExprKind::VarDeclExpr:
    case ExprKind::StringLiteralExpr:
    case ExprKind::CharacterLiteralExpr:
    case ExprKind::IntLiteralExpr:
    case ExprKind::FloatLiteralExpr:
    case ExprKind::BoolLiteralExpr:
    case ExprKind::NullLiteralExpr:
    case ExprKind::UndefinedLiteralExpr:
    case ExprKind::SizeofExpr:
        return false;
    }
    llvm_unreachable("all cases handled");
}

bool stmtMayWrite(Stmt& stmt, RootCtx& ctx, ConstMutationQuery& query) {
    switch (stmt.kind) {
    case StmtKind::ReturnStmt: {
        auto* value = llvm::cast<ReturnStmt>(stmt).value;
        if (value && ctx.countReturns && aliasesRoots(*value, ctx, false)) return true;
        return value && exprMayWrite(*value, ctx, query);
    }
    case StmtKind::VarStmt:
        for (auto* decl : llvm::cast<VarStmt>(stmt).decls) {
            if (decl->initializer && exprMayWrite(*decl->initializer, ctx, query)) return true;
        }
        return false;
    case StmtKind::ExprStmt:
        return exprMayWrite(*llvm::cast<ExprStmt>(stmt).expr, ctx, query);
    case StmtKind::DeferStmt:
        return exprMayWrite(*llvm::cast<DeferStmt>(stmt).expr, ctx, query);
    case StmtKind::IfStmt: {
        auto& ifStmt = llvm::cast<IfStmt>(stmt);
        if (exprMayWrite(*ifStmt.condition, ctx, query)) return true;
        return llvm::any_of(ifStmt.thenBody, [&](Stmt* s) { return stmtMayWrite(*s, ctx, query); })
            || llvm::any_of(ifStmt.elseBody, [&](Stmt* s) { return stmtMayWrite(*s, ctx, query); });
    }
    case StmtKind::SwitchStmt: {
        auto& switchStmt = llvm::cast<SwitchStmt>(stmt);
        if (exprMayWrite(*switchStmt.condition, ctx, query)) return true;
        for (auto& switchCase : switchStmt.cases) {
            if (exprMayWrite(*switchCase.value, ctx, query)) return true;
            if (llvm::any_of(switchCase.stmts, [&](Stmt* s) { return stmtMayWrite(*s, ctx, query); })) return true;
        }
        return llvm::any_of(switchStmt.defaultStmts, [&](Stmt* s) { return stmtMayWrite(*s, ctx, query); });
    }
    case StmtKind::WhileStmt: {
        auto& whileStmt = llvm::cast<WhileStmt>(stmt);
        if (exprMayWrite(*whileStmt.condition, ctx, query)) return true;
        return llvm::any_of(whileStmt.body, [&](Stmt* s) { return stmtMayWrite(*s, ctx, query); });
    }
    case StmtKind::DoWhileStmt: {
        auto& doWhile = llvm::cast<DoWhileStmt>(stmt);
        if (exprMayWrite(*doWhile.condition, ctx, query)) return true;
        return llvm::any_of(doWhile.body, [&](Stmt* s) { return stmtMayWrite(*s, ctx, query); });
    }
    case StmtKind::ForStmt: {
        auto& forStmt = llvm::cast<ForStmt>(stmt);
        if (forStmt.variable && stmtMayWrite(*forStmt.variable, ctx, query)) return true;
        if (forStmt.condition && exprMayWrite(*forStmt.condition, ctx, query)) return true;
        if (llvm::any_of(forStmt.increments, [&](Expr* e) { return exprMayWrite(*e, ctx, query); })) return true;
        return llvm::any_of(forStmt.body, [&](Stmt* s) { return stmtMayWrite(*s, ctx, query); });
    }
    case StmtKind::ForEachStmt: {
        // Checked bodies lower for-in loops away; walk the unlowered form defensively.
        auto& forEach = llvm::cast<ForEachStmt>(stmt);
        if (exprMayWrite(*forEach.range, ctx, query)) return true;
        return llvm::any_of(forEach.body, [&](Stmt* s) { return stmtMayWrite(*s, ctx, query); });
    }
    case StmtKind::BreakStmt:
    case StmtKind::ContinueStmt:
        return false;
    case StmtKind::CompoundStmt:
        return llvm::any_of(llvm::cast<CompoundStmt>(stmt).body, [&](Stmt* s) { return stmtMayWrite(*s, ctx, query); });
    }
    llvm_unreachable("all cases handled");
}

bool collectViewRootsInExpr(Expr& expr, const RootCtx& ctx, llvm::SmallPtrSet<const Decl*, 16>& views, llvm::SmallPtrSet<const Decl*, 16>& storage);
bool collectViewRootsInStmt(Stmt& stmt, const RootCtx& ctx, llvm::SmallPtrSet<const Decl*, 16>& views, llvm::SmallPtrSet<const Decl*, 16>& storage);

// Marks a binding holding a rooted alias-capable value: transparent aliases
// join storage, fresh wrappers join views. Returns whether anything was added.
bool markValueHolder(const Decl* holder, Type valueType, bool rooted, llvm::SmallPtrSet<const Decl*, 16>& views, llvm::SmallPtrSet<const Decl*, 16>& storage) {
    if (!holder || !rooted || !typeMayAliasStorageDeep(valueType)) return false;
    if (isTransparentAliasType(valueType)) return storage.insert(holder).second;
    return views.insert(holder).second;
}

// Re-marks the base of a store holding a rooted alias-capable value:
// projection stores taint the whole local by its holder type, so a later
// projection through a transparent member or element still flags while
// the slot store itself does not. Escaping targets are flagged by
// storeMayEscape instead. Returns whether anything was added.
bool markStoreBase(Expr& target, Expr& value, const RootCtx& ctx, llvm::SmallPtrSet<const Decl*, 16>& views, llvm::SmallPtrSet<const Decl*, 16>& storage) {
    auto* base = assignedBaseVar(target);
    if (!base || base->identifier == "this" || !base->decl) return false;
    bool markable = false;
    Type holderType;
    if (auto* var = llvm::dyn_cast<VarDecl>(base->decl)) {
        markable = !var->isGlobal();
        holderType = var->type;
    }
    if (auto* param = llvm::dyn_cast<ParamDecl>(base->decl)) {
        markable = !param->type.isBorrowOrOptionalBorrow();
        holderType = param->type;
    }
    if (!markable) return false;
    return markValueHolder(base->decl, holderType, aliasesRoots(value, ctx, false), views, storage);
}

// One prepass round: marks locals (and payload bindings) assigned rooted
// alias-capable values. Returns whether anything was added.
bool collectViewRootsInExpr(Expr& expr, const RootCtx& ctx, llvm::SmallPtrSet<const Decl*, 16>& views, llvm::SmallPtrSet<const Decl*, 16>& storage) {
    bool changed = false;
    if (auto* binary = llvm::dyn_cast<BinaryExpr>(&expr)) {
        if (isAssignmentOperator(binary->op.kind)) changed |= markStoreBase(binary->getLHS(), binary->getRHS(), ctx, views, storage);
    }
    if (auto* indexAssign = llvm::dyn_cast<IndexAssignmentExpr>(&expr)) {
        changed |= markStoreBase(*indexAssign->getBase(), *indexAssign->getValue(), ctx, views, storage);
    }
    switch (expr.kind) {
    case ExprKind::CallExpr:
    case ExprKind::BinaryExpr:
    case ExprKind::UnaryExpr:
    case ExprKind::IndexExpr:
    case ExprKind::IndexAssignmentExpr:
    case ExprKind::UnwrapExpr:
        for (auto& arg : llvm::cast<CallExpr>(expr).args)
            changed |= collectViewRootsInExpr(*arg.value, ctx, views, storage);
        if (expr.kind == ExprKind::UnwrapExpr) changed |= collectViewRootsInExpr(*llvm::cast<UnwrapExpr>(expr).getReceiver(), ctx, views, storage);
        if (auto* indexBase = getIndexBase(expr)) changed |= collectViewRootsInExpr(*indexBase, ctx, views, storage);
        break;
    case ExprKind::MemberExpr:
        changed |= collectViewRootsInExpr(*llvm::cast<MemberExpr>(expr).base, ctx, views, storage);
        break;
    case ExprKind::LambdaExpr: {
        // Lambda borrow parameters count as views: the callee binds them to
        // the caller's storage, and view marks only widen.
        auto* functionDecl = llvm::cast<LambdaExpr>(expr).functionDecl;
        if (!functionDecl) break;
        for (auto& param : functionDecl->getParams())
            if (typeMayAliasStorageDeep(param.type)) changed |= views.insert(&param).second;
        if (functionDecl->body) {
            for (auto* stmt : *functionDecl->body)
                changed |= collectViewRootsInStmt(*stmt, ctx, views, storage);
        }
        break;
    }
    case ExprKind::IfExpr: {
        auto& ifExpr = llvm::cast<IfExpr>(expr);
        changed |= collectViewRootsInExpr(*ifExpr.condition, ctx, views, storage);
        changed |= collectViewRootsInExpr(*ifExpr.thenExpr, ctx, views, storage);
        changed |= collectViewRootsInExpr(*ifExpr.elseExpr, ctx, views, storage);
        break;
    }
    case ExprKind::SwitchExpr: {
        auto& switchExpr = llvm::cast<SwitchExpr>(expr);
        changed |= collectViewRootsInExpr(*switchExpr.condition, ctx, views, storage);
        for (auto& arm : switchExpr.arms) {
            changed |= collectViewRootsInExpr(*arm.value, ctx, views, storage);
            changed |= collectViewRootsInExpr(*arm.expr, ctx, views, storage);
        }
        if (switchExpr.defaultExpr) changed |= collectViewRootsInExpr(*switchExpr.defaultExpr, ctx, views, storage);
        break;
    }
    case ExprKind::ImplicitCastExpr:
        changed |= collectViewRootsInExpr(*llvm::cast<ImplicitCastExpr>(expr).operand, ctx, views, storage);
        break;
    case ExprKind::ArrayLiteralExpr:
        for (auto* element : llvm::cast<ArrayLiteralExpr>(expr).elements)
            changed |= collectViewRootsInExpr(*element, ctx, views, storage);
        break;
    case ExprKind::AnonymousStructExpr:
        for (auto& element : llvm::cast<AnonymousStructExpr>(expr).elements)
            changed |= collectViewRootsInExpr(*element.value, ctx, views, storage);
        break;
    default:
        break;
    }
    return changed;
}

bool collectViewRootsInStmt(Stmt& stmt, const RootCtx& ctx, llvm::SmallPtrSet<const Decl*, 16>& views, llvm::SmallPtrSet<const Decl*, 16>& storage) {
    switch (stmt.kind) {
    case StmtKind::ReturnStmt: {
        auto* value = llvm::cast<ReturnStmt>(stmt).value;
        return value && collectViewRootsInExpr(*value, ctx, views, storage);
    }
    case StmtKind::VarStmt: {
        bool changed = false;
        for (auto* decl : llvm::cast<VarStmt>(stmt).decls) {
            if (!decl->initializer) continue;
            changed |= markValueHolder(decl, decl->initializer->type, aliasesRoots(*decl->initializer, ctx, false), views, storage);
            changed |= collectViewRootsInExpr(*decl->initializer, ctx, views, storage);
        }
        return changed;
    }
    case StmtKind::ExprStmt:
        return collectViewRootsInExpr(*llvm::cast<ExprStmt>(stmt).expr, ctx, views, storage);
    case StmtKind::DeferStmt:
        return collectViewRootsInExpr(*llvm::cast<DeferStmt>(stmt).expr, ctx, views, storage);
    case StmtKind::IfStmt: {
        auto& ifStmt = llvm::cast<IfStmt>(stmt);
        bool changed = collectViewRootsInExpr(*ifStmt.condition, ctx, views, storage);
        changed |=
            markValueHolder(ifStmt.isBinding, ifStmt.isBinding ? ifStmt.isBinding->type : Type(), aliasesRoots(*ifStmt.condition, ctx, false), views, storage);
        for (auto* s : ifStmt.thenBody)
            changed |= collectViewRootsInStmt(*s, ctx, views, storage);
        for (auto* s : ifStmt.elseBody)
            changed |= collectViewRootsInStmt(*s, ctx, views, storage);
        return changed;
    }
    case StmtKind::SwitchStmt: {
        auto& switchStmt = llvm::cast<SwitchStmt>(stmt);
        bool changed = collectViewRootsInExpr(*switchStmt.condition, ctx, views, storage);
        bool condRooted = aliasesRoots(*switchStmt.condition, ctx, false);
        for (auto& switchCase : switchStmt.cases) {
            changed |= collectViewRootsInExpr(*switchCase.value, ctx, views, storage);
            changed |=
                markValueHolder(switchCase.associatedValue, switchCase.associatedValue ? switchCase.associatedValue->type : Type(), condRooted, views, storage);
            for (auto* s : switchCase.stmts)
                changed |= collectViewRootsInStmt(*s, ctx, views, storage);
        }
        for (auto* s : switchStmt.defaultStmts)
            changed |= collectViewRootsInStmt(*s, ctx, views, storage);
        return changed;
    }
    case StmtKind::WhileStmt: {
        auto& whileStmt = llvm::cast<WhileStmt>(stmt);
        bool changed = collectViewRootsInExpr(*whileStmt.condition, ctx, views, storage);
        for (auto* s : whileStmt.body)
            changed |= collectViewRootsInStmt(*s, ctx, views, storage);
        return changed;
    }
    case StmtKind::DoWhileStmt: {
        auto& doWhile = llvm::cast<DoWhileStmt>(stmt);
        bool changed = collectViewRootsInExpr(*doWhile.condition, ctx, views, storage);
        for (auto* s : doWhile.body)
            changed |= collectViewRootsInStmt(*s, ctx, views, storage);
        return changed;
    }
    case StmtKind::ForStmt: {
        auto& forStmt = llvm::cast<ForStmt>(stmt);
        bool changed = false;
        if (forStmt.variable) changed |= collectViewRootsInStmt(*forStmt.variable, ctx, views, storage);
        if (forStmt.condition) changed |= collectViewRootsInExpr(*forStmt.condition, ctx, views, storage);
        for (auto* e : forStmt.increments)
            changed |= collectViewRootsInExpr(*e, ctx, views, storage);
        for (auto* s : forStmt.body)
            changed |= collectViewRootsInStmt(*s, ctx, views, storage);
        return changed;
    }
    case StmtKind::ForEachStmt: {
        auto& forEach = llvm::cast<ForEachStmt>(stmt);
        bool changed = collectViewRootsInExpr(*forEach.range, ctx, views, storage);
        for (auto* s : forEach.body)
            changed |= collectViewRootsInStmt(*s, ctx, views, storage);
        return changed;
    }
    case StmtKind::BreakStmt:
    case StmtKind::ContinueStmt:
        return false;
    case StmtKind::CompoundStmt: {
        bool changed = false;
        for (auto* s : llvm::cast<CompoundStmt>(stmt).body)
            changed |= collectViewRootsInStmt(*s, ctx, views, storage);
        return changed;
    }
    }
    llvm_unreachable("all cases handled");
}

bool mayWrite(FunctionDecl& func, bool thisRooted, bool countReturns, std::vector<const Decl*> marked, std::vector<const Decl*> markedViews,
              ConstMutationQuery& query) {
    std::sort(marked.begin(), marked.end());
    std::sort(markedViews.begin(), markedViews.end());
    ConstMutationQuery::Key key{&func, thisRooted, countReturns, marked, markedViews};
    if (auto cached = query.cache.find(key); cached != query.cache.end()) return cached->second;
    // Cycles answer false: the least fixpoint is exact for may-write, since a
    // derivation that never reaches a direct write writes nothing.
    if (!query.inProgress.insert(key).second) return false;
    llvm::SmallPtrSet<const Decl*, 16> storage;
    for (auto* decl : marked)
        storage.insert(decl);
    llvm::SmallPtrSet<const Decl*, 16> views;
    for (auto* decl : markedViews)
        views.insert(decl);
    if (!func.body) {
        query.inProgress.erase(key);
        query.cache[key] = false;
        return false;
    }
    RootCtx ctx{thisRooted, countReturns, llvm::isa<ConstructorDecl>(&func), storage, views};
    while (llvm::any_of(*func.body, [&](Stmt* stmt) { return collectViewRootsInStmt(*stmt, ctx, views, storage); })) {
    }
    bool writes = llvm::any_of(*func.body, [&](Stmt* stmt) { return stmtMayWrite(*stmt, ctx, query); });
    query.inProgress.erase(key);
    query.cache[key] = writes;
    return writes;
}

} // namespace

bool cx::typeMayAliasStorageDeep(Type type) {
    if (!type) return false;
    llvm::SmallPtrSet<const TypeDecl*, 8> visiting;
    return typeHasViewFields(type, visiting);
}

bool cx::methodMayMutateReceiver(FunctionDecl& method, ConstMutationQuery& query) {
    // A bodyless method (interface dispatch, extern C++) cannot be proven
    // read-only, so calling it on a constant is rejected.
    if (!method.body) return true;
    return mayWrite(method, true, /*countReturns=*/false, {}, {}, query);
}

bool cx::functionMayWriteThroughParam(FunctionDecl& func, const ParamDecl& param, ConstMutationQuery& query) {
    // A bodyless callee (interface dispatch, extern C++) cannot be proven
    // read-only, so passing a constant-derived iterator is rejected.
    if (!func.body) return true;
    return mayWrite(func, false, /*countReturns=*/true, {}, {&param}, query);
}
