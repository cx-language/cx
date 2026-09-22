#pragma once

#include <utility>
#include <vector>
#pragma warning(push, 0)
#include <llvm/Support/MemoryBuffer.h>
#pragma warning(pop)
#include "../ast/arena.h"
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
struct TupleExpr;
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
struct FunctionDecl;
struct FunctionTemplate;
struct FunctionDecl;
struct ConstructorDecl;
struct DestructorDecl;
struct TypeTemplate;
struct TypeDecl;
struct EnumDecl;
struct VarDecl;
struct FieldDecl;
struct ImportDecl;
struct Location;
struct Token;
struct Type;
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
    std::vector<NamedValue> parseArgumentList(bool allowEmpty);
    VarExpr* parseVarExpr();
    VarExpr* parseThis();
    StringLiteralExpr* parseStringLiteral();
    CharacterLiteralExpr* parseCharacterLiteral();
    IntLiteralExpr* parseIntLiteral();
    FloatLiteralExpr* parseFloatLiteral();
    BoolLiteralExpr* parseBoolLiteral();
    NullLiteralExpr* parseNullLiteral();
    UndefinedLiteralExpr* parseUndefinedLiteral();
    ArrayLiteralExpr* parseArrayLiteral();
    Expr* parseTupleLiteralOrParenExpr();
    std::vector<Type> parseNonEmptyTypeList();
    std::vector<Type> parseGenericArgumentList();
    Type parseArrayType(Type elementType);
    Type parseSimpleType(Mutability mutability);
    Type parseTupleType();
    Type parseFunctionType(Type returnType);
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
    bool shouldParseVarStmt();
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
    std::vector<Expr*> parseExprList();
    ReturnStmt* parseReturnStmt();
    VarDecl* parseVarDecl(Decl* parent, AccessLevel accessLevel, bool requireTerminator = true);
    VarDecl* parseVarDeclAfterName(Decl* parent, AccessLevel accessLevel, Type type, llvm::StringRef name, Location nameLocation,
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
    std::vector<Stmt*> parseBlock(Decl* parent);
    std::vector<Stmt*> parseBlockOrStmt(Decl* parent);
    std::vector<Stmt*> parseStmtsUntilOneOf(Token::Kind end1, Token::Kind end2, Token::Kind end3, Decl* parent);
    ParamDecl parseParam(bool requireType);
    std::vector<ParamDecl> parseParamList(bool* isVariadic, bool requireTypes = true);
    void parseGenericParamList(std::vector<GenericParamDecl>& genericParams);
    llvm::StringRef parseFunctionName(TypeDecl* receiverTypeDecl);
    FunctionDecl* parseFunctionProto(bool isExtern, TypeDecl* receiverTypeDecl, AccessLevel accessLevel, std::vector<GenericParamDecl>* genericParams,
                                     Type returnType, llvm::StringRef name, Location location);
    FunctionTemplate* parseFunctionTemplateProto(TypeDecl* receiverTypeDecl, AccessLevel accessLevel, Type type, llvm::StringRef name, Location location);
    FunctionDecl* parseFunctionDecl(TypeDecl* receiverTypeDecl, AccessLevel accessLevel, bool requireBody, Type type, llvm::StringRef name, Location location);
    FunctionTemplate* parseFunctionTemplate(TypeDecl* receiverTypeDecl, AccessLevel accessLevel, Type type, llvm::StringRef name, Location location);
    FunctionDecl* parseExternFunctionDecl(AccessLevel accessLevel, Type type, llvm::StringRef name, Location location);
    ConstructorDecl* parseConstructorDecl(TypeDecl& receiverTypeDecl, AccessLevel accessLevel);
    DestructorDecl* parseDestructorDecl(TypeDecl& receiverTypeDecl);
    FieldDecl parseFieldDecl(TypeDecl& typeDecl, AccessLevel accessLevel, Type type, llvm::StringRef name, Location location);
    TypeTemplate* parseTypeTemplate(AccessLevel accessLevel);
    Token parseTypeHeader(std::vector<Type>& interfaces, std::vector<GenericParamDecl>* genericParams);
    TypeDecl* parseTypeDecl(std::vector<GenericParamDecl>* genericParams, AccessLevel typeAccessLevel);
    TypeTemplate* parseEnumTemplate(AccessLevel accessLevel);
    EnumDecl* parseEnumDecl(std::vector<GenericParamDecl>* genericParams, AccessLevel typeAccessLevel);
    ImportDecl* parseImportDecl();
    void parseIfdefBody(std::vector<Decl*>* activeDecls);
    void parseIfdef(std::vector<Decl*>* activeDecls);
    Decl* parseTopLevelDecl(bool addToSymbolTable);
    Decl* parseTopLevelFunctionOrVariable(bool isExtern, bool addToSymbolTable, AccessLevel accessLevel);

private:
    Lexer lexer;
    Module* currentModule;
    std::vector<Token> tokenBuffer;
    size_t currentTokenIndex;
    const CompileOptions& options;
    bool allowBlockLambda = true;
};

} // namespace cx
