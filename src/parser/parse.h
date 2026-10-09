#pragma once

#include <utility>
#include <vector>
#pragma warning(push, 0)
#include <llvm/Support/MemoryBuffer.h>
#pragma warning(pop)
#include "../ast/arena.h"
#include "../ast/checks.h"
#include "../ast/type.h"
#include "lex.h"

namespace llvm {
class StringRef;
template<typename T> class ArrayRef;
} // namespace llvm

namespace cx {

struct NamedValue;
struct Module;
struct SourceFile;
struct Expr;
struct VarExpr;
struct StringLiteralExpr;
struct CharacterLiteralExpr;
struct IntLiteralExpr;
struct FloatLiteralExpr;
struct BoolLiteralExpr;
struct NullLiteralExpr;
struct UndefinedLiteralExpr;
struct ArrayLiteralExpr;
struct AnonymousStructExpr;
struct SizeofExpr;
struct MemberExpr;
struct IndexExpr;
struct UnwrapExpr;
struct CallExpr;
struct UnaryExpr;
struct LambdaExpr;
struct IfExpr;
struct SwitchExpr;
struct Stmt;
struct ReturnStmt;
struct VarStmt;
struct ExprStmt;
struct DeferStmt;
struct IfStmt;
struct WhileStmt;
struct DoWhileStmt;
struct ForStmt;
struct ForEachStmt;
struct SwitchStmt;
struct BreakStmt;
struct ContinueStmt;
struct Decl;
struct ParamDecl;
struct GenericParamDecl;
struct FunctionDecl;
struct FunctionTemplate;
struct ConstructorDecl;
struct DestructorDecl;
struct TypeTemplate;
struct TypeDecl;
struct TypeAliasDecl;
struct EnumDecl;
struct VarDecl;
struct FieldDecl;
struct ImportDecl;
struct Location;
struct Token;
struct Type;
struct CompileError;
enum class AccessLevel;
struct CompileOptions;

struct Parser {
    Parser(llvm::MemoryBufferRef input, Module& module, const CompileOptions& options);
    void parse();

private:
    Token currentToken();
    Location getCurrentLocation();
    Token lookAhead(int offset);
    Token consumeToken();
    Location getLastTokenEndLocation();
    template<typename T, typename... Args> T* makeExpr(Args&&... args) {
        T* expr = makeAST<T>(std::forward<Args>(args)...);
        expr->endLocation = getLastTokenEndLocation();
        return expr;
    }
    Token parse(llvm::ArrayRef<Token::Kind> expected, const char* contextInfo = nullptr);
    void parseStmtTerminator(const char* contextInfo = nullptr);
    AstVector<NamedValue> parseArgumentList(bool allowEmpty);
    VarExpr* parseVarExpr();
    VarExpr* parseThis();
    StringLiteralExpr* parseStringLiteral();
    Expr* parseInterpolationRest(Expr* acc);
    CharacterLiteralExpr* parseCharacterLiteral();
    IntLiteralExpr* parseIntLiteral();
    FloatLiteralExpr* parseFloatLiteral();
    BoolLiteralExpr* parseBoolLiteral();
    NullLiteralExpr* parseNullLiteral();
    UndefinedLiteralExpr* parseUndefinedLiteral();
    ArrayLiteralExpr* parseArrayLiteral();
    Expr* parseAnonymousStructLiteralOrParenExpr();
    AstVector<Type> parseNonEmptyTypeList();
    AstVector<GenericArg> parseGenericArgumentList();
    Type parseArrayType(Type elementType);
    bool resolveSizeExprDecls(Expr& expr, std::vector<VarDecl*>& resolutionStack);
    // Reparses a function return type and name under the binder guard once `<`
    // reveals the function is generic; generic parameters may shadow globals
    // the first parse folded against. `returnTypeIndex` is the token index
    // from before the first parse. Reparsing is side-effect-free (pure token
    // consumption plus arena garbage), so rewinding is safe.
    void reparseGenericReturnType(Type& type, Location& location, llvm::StringRef& name, size_t returnTypeIndex, TypeDecl* receiver);
    Type parseSimpleType();
    Type parseAnonymousStructType();
    Type parseFunctionType(Type returnType);
    // `const` is rejected in types; declarators consume a leading `const` themselves.
    Type parseType();
    SizeofExpr* parseSizeofExpr();
    MemberExpr* parseMemberExpr(Expr* lhs);
    Expr* parseIndexExprOrIndexAssignmentExpr(Expr* base);
    UnwrapExpr* parseUnwrapExpr(Expr* operand);
    CallExpr* parseCallExpr(Expr* callee);
    LambdaExpr* parseLambdaExpr();
    IfExpr* parseIfExpr(Expr* condition);
    SwitchExpr* parseSwitchExpr();
    IfExpr* parseIfThenElseExpr();
    // Errors when `condition` is an `is` expression followed by a binding name.
    // `context` completes "not <context>" (for example "while loops").
    void rejectIsBinding(Expr* condition, const char* context);
    // Parses an optional parenthesized loop condition, rejecting `is` bindings
    // both inside the parentheses and after them.
    Expr* parseLoopCondition(Decl* parent, bool allowVarDecl);
    bool shouldParseVarStmt();
    void splitRightShiftIfPresent();
    bool isTightLessThan(int lessOffset);
    bool shouldParseGenericArgumentList();
    bool shouldParseGenericArgumentListAfterMember();
    bool lambdaAfterParentheses();
    Expr* parsePostfixExpr();
    UnaryExpr* parsePrefixExpr();
    Expr* parsePreOrPostfixExpr();
    UnaryExpr* parseIncrementOrDecrementExpr(Expr* operand);
    Expr* parseBinaryExpr(int minPrecedence);
    Expr* parseExpr();
    Expr* parseExprOrVarDecl(Decl* parent);
    AstVector<Expr*> parseExprList();
    ReturnStmt* parseReturnStmt();
    VarDecl* parseVarDecl(Decl* parent, AccessLevel accessLevel, bool requireTerminator = true);
    VarDecl* parseVarDeclAfterName(Decl* parent, AccessLevel accessLevel, Type type, llvm::StringRef name, Location nameLocation, bool isConst,
                                   bool requireTerminator = true);
    VarStmt* parseVarStmt(Decl* parent);
    ExprStmt* parseExprStmt();
    DeferStmt* parseDeferStmt();
    Stmt* parseIfStmt(Decl* parent);
    WhileStmt* parseWhileStmt(Decl* parent);
    DoWhileStmt* parseDoWhileStmt(Decl* parent);
    Stmt* parseForOrForEachStmt(Decl* parent);
    SwitchStmt* parseSwitchStmt(Decl* parent);
    std::pair<Expr*, VarDecl*> parseSwitchCaseHeader(Decl* parent);
    BreakStmt* parseBreakStmt();
    ContinueStmt* parseContinueStmt();
    Stmt* parseStmt(Decl* parent);
    AstVector<Stmt*> parseBlock(Decl* parent);
    AstVector<Stmt*> parseBlockOrStmt(Decl* parent);
    AstVector<Stmt*> parseStmtsUntilOneOf(Token::Kind end1, Token::Kind end2, Token::Kind end3, Decl* parent);
    ParamDecl parseParam(bool requireType, bool allowCxxConst);
    AstVector<ParamDecl> parseParamList(bool* isVariadic, bool requireTypes = true, bool allowCxxConst = false);
    void parseGenericParamList(AstVector<GenericParamDecl>& genericParams);
    llvm::StringRef parseFunctionName(TypeDecl* receiverTypeDecl);
    FunctionDecl* parseFunctionProto(bool isExtern, TypeDecl* receiverTypeDecl, AccessLevel accessLevel, AstVector<GenericParamDecl>* genericParams,
                                     Type returnType, llvm::StringRef name, Location location, bool cppLinkage = false);
    FunctionTemplate* parseFunctionTemplateProto(TypeDecl* receiverTypeDecl, AccessLevel accessLevel, Type type, llvm::StringRef name, Location location);
    FunctionDecl* parseFunctionDecl(TypeDecl* receiverTypeDecl, AccessLevel accessLevel, bool requireBody, Type type, llvm::StringRef name, Location location,
                                    bool isImplicit = false);
    void parseOptionalFunctionBody(FunctionDecl& decl, bool requireBody);
    FunctionTemplate* parseFunctionTemplate(TypeDecl* receiverTypeDecl, AccessLevel accessLevel, Type type, llvm::StringRef name, Location location);
    FunctionDecl* parseExternFunctionDecl(AccessLevel accessLevel, Type type, llvm::StringRef name, Location location, bool cppLinkage = false);
    Decl* parseConstructorDecl(TypeDecl& receiverTypeDecl, AccessLevel accessLevel, bool isImplicit = false);
    // True when the '<' after the current identifier closes with a '>' that is
    // immediately followed by '(', i.e. `Name<...>(...)`.
    bool genericParamListFollowedByParen();
    DestructorDecl* parseDestructorDecl(TypeDecl& receiverTypeDecl);
    FieldDecl parseFieldDecl(TypeDecl& typeDecl, AccessLevel accessLevel, Type type, llvm::StringRef name, Location location, bool isManuallyDestroy);
    void parsePrivateSpecifier(AccessLevel& accessLevel);
    // Rejects `@test` and `@manuallyDestroy` on a declaration that cannot carry them.
    void rejectMisplacedDeclAttributes(bool isTest, bool isManuallyDestroy, Location manuallyDestroyLocation, DisabledChecks disabledChecks,
                                       Location checksLocation, const char* testMessage = "only functions can be marked as tests",
                                       const char* checksMessage = "only functions, statements and expressions can disable safety checks");
    // Records function-level check attributes, or rejects them on a non-function declaration.
    void applyFunctionChecks(Decl* decl, DisabledChecks disabledChecks, Location checksLocation);
    void rejectMisplacedChecks(DisabledChecks disabledChecks, Location checksLocation);
    // Records `@discardableResult` on a function declaration, or rejects it on anything else.
    // Bodyless functions qualify, extern ones included: unlike `@test` and the
    // check attributes, this one constrains callers, not the body.
    void applyDiscardableResult(Decl* decl, bool isDiscardableResult, Location discardableResultLocation);
    void rejectMisplacedDiscardableResult(bool isDiscardableResult, Location discardableResultLocation);
    void rejectGenericStaticConst(const AstVector<GenericParamDecl>* genericParams);
    // Current token is `=`. Parses the initializer and adds a static constant.
    void addParsedStaticConst(TypeDecl& typeDecl, Type type, llvm::StringRef name, Location location, AccessLevel accessLevel);
    // Current token is `const`, already known to introduce `const name = expr`.
    void parseKeywordStaticConst(TypeDecl& typeDecl, AccessLevel accessLevel, const AstVector<GenericParamDecl>* genericParams);
    TypeTemplate* parseTypeTemplate(AccessLevel accessLevel);
    Token parseTypeHeader(AstVector<Type>& interfaces, AstVector<GenericParamDecl>* genericParams);
    TypeDecl* parseTypeDecl(AstVector<GenericParamDecl>* genericParams, AccessLevel typeAccessLevel);
    TypeAliasDecl* parseTypeAliasDecl(AccessLevel accessLevel);
    TypeTemplate* parseEnumTemplate(AccessLevel accessLevel);
    EnumDecl* parseEnumDecl(AstVector<GenericParamDecl>* genericParams, AccessLevel typeAccessLevel);
    ImportDecl* parseImportDecl();
    void parseIfdefBody(std::vector<Decl*>* activeDecls);
    void parseIfdef(std::vector<Decl*>* activeDecls);
    Decl* parseTopLevelDecl(bool addToSymbolTable);
    // Reports a parse error and skips to the next recovery point (recovery
    // mode only). False when parsing cannot continue: end of file, the
    // recovery cap, or a second error while skipping.
    bool recoverFromParseError(const CompileError& error, llvm::ArrayRef<Token::Kind> endTokens, bool consumeClosingBrace);
    // Skips to an end token or a later line at nesting depth zero (resumed
    // without consuming; end tokens exit the caller's loop), or past one ';'
    // or '}' closing the broken construct. Every other outcome consumes a
    // token or reports EOF, so recovery loops terminate.
    bool skipToRecoveryPoint(llvm::ArrayRef<Token::Kind> endTokens, bool consumeClosingBrace);
    Decl* parseTopLevelFunctionOrVariable(bool isExtern, bool addToSymbolTable, AccessLevel accessLevel, bool cppLinkage = false);
    void parseAttributes(bool& isTest, Location& testLocation, bool& isManuallyDestroy, Location& manuallyDestroyLocation, DisabledChecks& disabledChecks,
                         Location& checksLocation, bool& isDiscardableResult, Location& discardableResultLocation);
    [[noreturn]] void errorMisplacedManuallyDestroy(Location location);

private:
    Lexer lexer;
    Module* currentModule;
    std::vector<Token> tokenBuffer;
    size_t currentTokenIndex;
    const CompileOptions& options;
    bool allowBlockLambda = true;
    // True inside scopes binding names that shadow top-level constants
    // (function signatures and bodies, lambdas, generic declarations), where
    // parser-time size-expression resolution stays off so an array size can
    // never fold against a shadowed global.
    bool inBinderScope = false;
    // Errors recovered from in this parse() call; capped so pathological
    // input cannot stall the language server.
    int parseRecoveryErrors = 0;
    // Recovery gave up (EOF, cap, or a stuck lexer): outer loops unwind
    // quietly instead of re-reporting the same failure per nesting level.
    bool parseRecoveryBailed = false;
};

} // namespace cx
