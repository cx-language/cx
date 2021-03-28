#pragma once

#include <vector>
#pragma warning(push, 0)
#include <llvm/Support/MemoryBuffer.h>
#pragma warning(pop)
#include "lex.h"
#include "../ast/type.h"

namespace llvm {
class StringRef;
template<typename T>
class ArrayRef;
} // namespace llvm

namespace delta {

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
struct AddressofExpr;
struct MemberExpr;
struct IndexExpr;
struct UnwrapExpr;
struct CallExpr;
struct UnaryExpr;
struct LambdaExpr;
struct IfExpr;
struct Stmt;
struct ReturnStmt;
struct VarStmt;
struct ExprStmt;
struct DeferStmt;
struct IfStmt;
struct WhileStmt;
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
struct SourceLocation;
struct Token;
struct Type;
enum class AccessLevel;
struct CompileOptions;

struct Parser {
    Parser(llvm::StringRef filePath, Module& module, const CompileOptions& options);
    void parse();

private:
    Token currentToken();
    SourceLocation getCurrentLocation();
    Token lookAhead(int offset);
    Token consumeToken();
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
    int64_t parseArraySizeInBrackets();
    Type parseSimpleType(Mutability mutability);
    Type parseTupleType();
    Type parseFunctionType(Type returnType);
    Type parseType();
    SizeofExpr* parseSizeofExpr();
    AddressofExpr* parseAddressofExpr();
    MemberExpr* parseMemberExpr(Expr* lhs);
    Expr* parseIndexExprOrIndexAssignmentExpr(Expr* base);
    UnwrapExpr* parseUnwrapExpr(Expr* operand);
    CallExpr* parseCallExpr(Expr* callee);
    LambdaExpr* parseLambdaExpr();
    IfExpr* parseIfExpr(Expr* condition);
    bool shouldParseVarStmt();
    bool shouldParseGenericArgumentList();
    bool arrowAfterParentheses();
    Expr* parsePostfixExpr();
    UnaryExpr* parsePrefixExpr();
    Expr* parsePreOrPostfixExpr();
    UnaryExpr* parseIncrementOrDecrementExpr(Expr* operand);
    Expr* parseBinaryExpr(int minPrecedence);
    Expr* parseExpr();
    std::vector<Expr*> parseExprList();
    ReturnStmt* parseReturnStmt();
    VarDecl* parseVarDecl(Decl* parent, AccessLevel accessLevel);
    VarDecl* parseVarDeclAfterName(Decl* parent, AccessLevel accessLevel, Type type, llvm::StringRef name, SourceLocation nameLocation);
    VarStmt* parseVarStmt(Decl* parent);
    ExprStmt* parseExprStmt();
    DeferStmt* parseDeferStmt();
    IfStmt* parseIfStmt(Decl* parent);
    WhileStmt* parseWhileStmt(Decl* parent);
    Stmt* parseForOrForEachStmt(Decl* parent);
    SwitchStmt* parseSwitchStmt(Decl* parent);
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
                                     Type returnType, llvm::StringRef name, SourceLocation location);
    FunctionTemplate* parseFunctionTemplateProto(TypeDecl* receiverTypeDecl, AccessLevel accessLevel, Type type, llvm::StringRef name, SourceLocation location);
    FunctionDecl* parseFunctionDecl(TypeDecl* receiverTypeDecl, AccessLevel accessLevel, bool requireBody, Type type, llvm::StringRef name,
                                    SourceLocation location);
    FunctionTemplate* parseFunctionTemplate(TypeDecl* receiverTypeDecl, AccessLevel accessLevel, Type type, llvm::StringRef name, SourceLocation location);
    FunctionDecl* parseExternFunctionDecl(Type type, llvm::StringRef name, SourceLocation location);
    ConstructorDecl* parseConstructorDecl(TypeDecl& receiverTypeDecl, AccessLevel accessLevel);
    DestructorDecl* parseDestructorDecl(TypeDecl& receiverTypeDecl);
    FieldDecl parseFieldDecl(TypeDecl& typeDecl, AccessLevel accessLevel, Type type, llvm::StringRef name, SourceLocation location);
    TypeTemplate* parseTypeTemplate(AccessLevel accessLevel);
    Token parseTypeHeader(std::vector<Type>& interfaces, std::vector<GenericParamDecl>* genericParams);
    TypeDecl* parseTypeDecl(std::vector<GenericParamDecl>* genericParams, AccessLevel typeAccessLevel);
    EnumDecl* parseEnumDecl(AccessLevel typeAccessLevel);
    ImportDecl* parseImportDecl();
    void parseIfdefBody(std::vector<Decl*>* activeDecls);
    void parseIfdef(std::vector<Decl*>* activeDecls);
    Decl* parseTopLevelDecl(bool addToSymbolTable);
    Decl* parseTopLevelFunctionOrVariable(bool isExtern, bool addToSymbolTable, AccessLevel accessLevel);
    ConstructorDecl* createAutogeneratedConstructor(TypeDecl* typeDecl) const;

private:
    Lexer lexer;
    Module* currentModule;
    std::vector<Token> tokenBuffer;
    size_t currentTokenIndex;
    const CompileOptions& options;
};

} // namespace delta
