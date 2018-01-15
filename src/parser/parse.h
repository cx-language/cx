#pragma once

#include <memory>
#include <string>
#include <vector>
#pragma warning(push, 0)
#include <llvm/ADT/ArrayRef.h>
#include <llvm/Support/MemoryBuffer.h>
#pragma warning(pop)
#include "lex.h"

namespace llvm {
class StringRef;
}

namespace delta {

class Argument;
class Module;
class SourceFile;
class Expr;
class VarExpr;
class StringLiteralExpr;
class CharacterLiteralExpr;
class IntLiteralExpr;
class FloatLiteralExpr;
class BoolLiteralExpr;
class NullLiteralExpr;
class ArrayLiteralExpr;
class CastExpr;
class SizeofExpr;
class AddressofExpr;
class MemberExpr;
class SubscriptExpr;
class UnwrapExpr;
class CallExpr;
class PrefixExpr;
class LambdaExpr;
class IfExpr;
class Stmt;
class ReturnStmt;
class VarStmt;
class ExprStmt;
class IncrementStmt;
class DecrementStmt;
class DeferStmt;
class IfStmt;
class WhileStmt;
class SwitchStmt;
class BreakStmt;
class AssignStmt;
class Decl;
class ParamDecl;
class GenericParamDecl;
class FunctionDecl;
class FunctionTemplate;
class FunctionDecl;
class FunctionTemplate;
class FunctionDecl;
class InitDecl;
class DeinitDecl;
class TypeTemplate;
class TypeDecl;
class EnumDecl;
class VarDecl;
class FieldDecl;
class ImportDecl;
struct SourceLocation;
struct Token;
struct Type;

class Parser {
public:
    Parser(llvm::StringRef filePath, Module& module,
           llvm::ArrayRef<std::string> importSearchPaths,
           llvm::ArrayRef<std::string> frameworkSearchPaths);
    Parser(std::unique_ptr<llvm::MemoryBuffer> input, Module& module,
           llvm::ArrayRef<std::string> importSearchPaths,
           llvm::ArrayRef<std::string> frameworkSearchPaths);
    void parse();
    std::unique_ptr<Expr> parseExpr();

private:
    Token currentToken();
    SourceLocation getCurrentLocation();
    Token lookAhead(int offset);
    Token consumeToken();
    void expect(llvm::ArrayRef<Token::Kind> expected, const char* contextInfo);
    Token parse(llvm::ArrayRef<Token::Kind> expected, const char* contextInfo = nullptr);
    void checkStmtTerminatorConsistency(Token::Kind currentTerminator,
                                        llvm::function_ref<SourceLocation()> getLocation);
    void parseStmtTerminator(const char* contextInfo = nullptr);
    std::vector<Argument> parseArgumentList();
    std::unique_ptr<VarExpr> parseVarExpr();
    std::unique_ptr<VarExpr> parseThis();
    std::unique_ptr<StringLiteralExpr> parseStringLiteral();
    std::unique_ptr<CharacterLiteralExpr> parseCharacterLiteral();
    std::unique_ptr<IntLiteralExpr> parseIntLiteral();
    std::unique_ptr<FloatLiteralExpr> parseFloatLiteral();
    std::unique_ptr<BoolLiteralExpr> parseBoolLiteral();
    std::unique_ptr<NullLiteralExpr> parseNullLiteral();
    std::unique_ptr<ArrayLiteralExpr> parseArrayLiteral();
    std::vector<Type> parseNonEmptyTypeList();
    std::vector<Type> parseGenericArgumentList();
    int64_t parseArraySizeInBrackets();
    Type parseSimpleType(bool isMutable);
    Type parseFunctionType();
    Type parseType();
    std::unique_ptr<CastExpr> parseCastExpr();
    std::unique_ptr<SizeofExpr> parseSizeofExpr();
    std::unique_ptr<AddressofExpr> parseAddressofExpr();
    std::unique_ptr<MemberExpr> parseMemberExpr(std::unique_ptr<Expr> lhs);
    std::unique_ptr<SubscriptExpr> parseSubscript(std::unique_ptr<Expr> operand);
    std::unique_ptr<UnwrapExpr> parseUnwrapExpr(std::unique_ptr<Expr> operand);
    std::unique_ptr<CallExpr> parseCallExpr(std::unique_ptr<Expr> callee);
    std::unique_ptr<LambdaExpr> parseLambdaExpr();
    std::unique_ptr<Expr> parseParenExpr();
    std::unique_ptr<IfExpr> parseIfExpr(std::unique_ptr<Expr> condition);
    bool shouldParseGenericArgumentList();
    std::unique_ptr<Expr> parsePostfixExpr();
    std::unique_ptr<PrefixExpr> parsePrefixExpr();
    std::unique_ptr<Expr> parsePreOrPostfixExpr();
    std::unique_ptr<Expr> parseBinaryExpr(std::unique_ptr<Expr> lhs, int minPrecedence);
    std::unique_ptr<AssignStmt> parseAssignStmt(std::unique_ptr<Expr> lhs);
    std::unique_ptr<AssignStmt> parseCompoundAssignStmt(std::unique_ptr<Expr> lhs);
    std::vector<std::unique_ptr<Expr>> parseExprList();
    std::unique_ptr<ReturnStmt> parseReturnStmt();
    std::unique_ptr<VarDecl> parseVarDecl(bool requireInitialValue, Decl* parent);
    std::unique_ptr<VarStmt> parseVarStmt(Decl* parent);
    std::unique_ptr<ExprStmt> parseCallStmt(std::unique_ptr<Expr> callExpr);
    std::unique_ptr<IncrementStmt> parseIncrementStmt(std::unique_ptr<Expr> operand);
    std::unique_ptr<DecrementStmt> parseDecrementStmt(std::unique_ptr<Expr> operand);
    std::unique_ptr<DeferStmt> parseDeferStmt();
    std::unique_ptr<IfStmt> parseIfStmt(Decl* parent);
    std::unique_ptr<WhileStmt> parseWhileStmt(Decl* parent);
    std::unique_ptr<Stmt> parseForStmt(Decl* parent);
    std::unique_ptr<SwitchStmt> parseSwitchStmt(Decl* parent);
    std::unique_ptr<BreakStmt> parseBreakStmt();
    std::unique_ptr<Stmt> parseStmt(Decl* parent);
    std::vector<std::unique_ptr<Stmt>> parseStmtsUntil(Token::Kind end, Decl* parent);
    std::vector<std::unique_ptr<Stmt>> parseStmtsUntilOneOf(Token::Kind end1, Token::Kind end2,
                                                            Token::Kind end3, Decl* parent);
    ParamDecl parseParam(bool withType);
    std::vector<ParamDecl> parseParamList(bool* isVariadic, bool withTypes);
    void parseGenericParamList(std::vector<GenericParamDecl>& genericParams);
    std::unique_ptr<FunctionDecl> parseFunctionProto(bool isExtern, TypeDecl* receiverTypeDecl,
                                                           std::vector<GenericParamDecl>* genericParams);
    std::unique_ptr<FunctionTemplate> parseFunctionTemplateProto(TypeDecl* receiverTypeDecl);
    std::unique_ptr<FunctionDecl> parseFunctionDecl(TypeDecl* receiverTypeDecl, bool requireBody = true);
    std::unique_ptr<FunctionTemplate> parseFunctionTemplate(TypeDecl* receiverTypeDecl);
    std::unique_ptr<FunctionDecl> parseExternFunctionDecl();
    std::unique_ptr<InitDecl> parseInitDecl(TypeDecl& receiverTypeDecl);
    std::unique_ptr<DeinitDecl> parseDeinitDecl(TypeDecl& receiverTypeDecl);
    FieldDecl parseFieldDecl(TypeDecl& typeDecl);
    std::unique_ptr<TypeTemplate> parseTypeTemplate();
    Token parseTypeHeader(std::vector<Type>& interfaces, std::vector<GenericParamDecl>* genericParams);
    std::unique_ptr<TypeDecl> parseTypeDecl(std::vector<GenericParamDecl>* genericParams);
    std::unique_ptr<EnumDecl> parseEnumDecl(std::vector<GenericParamDecl>* genericParams);
    std::unique_ptr<ImportDecl> parseImportDecl();
    void parseIfdefBody(std::vector<std::unique_ptr<Decl>>* activeDecls);
    void parseIfdef(std::vector<std::unique_ptr<Decl>>* activeDecls);
    std::unique_ptr<Decl> parseTopLevelDecl(bool addToSymbolTable);

private:
    Lexer lexer;
    Module* currentModule;
    std::vector<Token> tokenBuffer;
    size_t currentTokenIndex;
    llvm::ArrayRef<std::string> importSearchPaths;
    llvm::ArrayRef<std::string> frameworkSearchPaths;
};

}
