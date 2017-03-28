#include <vector>
#include <sstream>
#include <initializer_list>
#include <llvm/ADT/STLExtras.h>
#include <llvm/Support/ErrorHandling.h>
#include "parse.h"
#include "lex.h"
#include "token.h"
#include "../sema/typecheck.h"
#include "../ast/decl.h"
#include "../ast/module.h"
#include "../driver/utility.h"

using namespace delta;

namespace {

std::vector<Token> tokenBuffer;
int currentTokenIndex;

Token currentToken() {
    assert(currentTokenIndex < tokenBuffer.size());
    return tokenBuffer[currentTokenIndex];
}

SrcLoc currentLoc() {
    return currentToken().getLoc();
}

Token lookAhead(int offset) {
    auto count = currentTokenIndex + offset - int(tokenBuffer.size()) + 1;
    while (count-- > 0) tokenBuffer.emplace_back(lex());
    return tokenBuffer[currentTokenIndex + offset];
}

Token consumeToken() {
    Token token = currentToken();
    if (++currentTokenIndex == tokenBuffer.size())
        tokenBuffer.emplace_back(lex());
    return token;
}

/// Adds quotes around the string representation of the given token unless
/// it's an identifier, numeric literal, string literal, or end-of-file.
std::string quote(TokenKind tokenKind) {
    std::ostringstream stream;
    if (tokenKind < BREAK) {
        stream << tokenKind;
    } else {
        stream << '\'' << tokenKind << '\'';
    }
    return stream.str();
}

[[noreturn]] void unexpectedToken(Token token, std::initializer_list<TokenKind> expected = { }) {
    if (expected.size() == 0) {
        error(token.getLoc(), "unexpected ", quote(token));
    }
    std::ostringstream stringOfExpected;
    for (const TokenKind& tokenKind : expected) {
        stringOfExpected << quote(tokenKind);
        if (&tokenKind != expected.end() - 2)
            stringOfExpected << ", ";
        else
            stringOfExpected << (expected.size() != 2 ? ", or " : " or ");
    }
    error(token.getLoc(), "expected ", stringOfExpected.str(), "got ", quote(token));
}

void expect(TokenKind tokenKind, const char* contextInfo) {
    if (currentToken().kind != tokenKind) {
        error(currentLoc(), "expected ", quote(tokenKind), ' ', contextInfo,
              ", got ", quote(currentToken()));
    }
}

Token parse(TokenKind tokenKind, const char* contextInfo = nullptr) {
    if (currentToken().kind != tokenKind) {
        if (contextInfo) {
            error(currentLoc(), "expected ", quote(tokenKind), " ", contextInfo,
                  ", got ", quote(currentToken()));
        } else {
            error(currentLoc(), "expected ", quote(tokenKind),
                  ", got ", quote(currentToken()));
        }
    }
    return consumeToken();
}

std::unique_ptr<Expr> parseExpr();
std::unique_ptr<Expr> parsePreOrPostfixExpr();
std::vector<std::unique_ptr<Expr>> parseExprListUntil(Token end);
std::vector<std::unique_ptr<Stmt>> parseStmtsUntil(Token end);
std::vector<std::unique_ptr<Stmt>> parseStmtsUntilOneOf(Token end1, Token end2, Token end3);

/// arg-list ::= '(' ')' | '(' nonempty-arg-list ')'
/// nonempty-arg-list ::= arg | nonempty-arg-list ',' arg
/// arg ::= expr | id ':' expr
std::vector<Arg> parseArgList() {
    parse(LPAREN);
    std::vector<Arg> args;
    while (currentToken() != RPAREN) {
        std::string label;
        SrcLoc location = SrcLoc::invalid();
        if (lookAhead(1) == COLON) {
            auto result = parse(IDENTIFIER);
            label = std::move(result.string);
            location = result.getLoc();
            consumeToken();
        }
        auto value = parseExpr();
        if (!location.isValid()) location = value->getSrcLoc();
        args.push_back({ std::move(label), std::move(value), location });
        if (currentToken() != RPAREN) parse(COMMA);
    }
    consumeToken();
    return args;
}

/// var-expr ::= id
std::unique_ptr<VariableExpr> parseVariableExpr() {
    assert(currentToken() == IDENTIFIER);
    auto id = parse(IDENTIFIER);
    return llvm::make_unique<VariableExpr>(std::move(id.string), id.getLoc());
}

std::unique_ptr<VariableExpr> parseThis() {
    assert(currentToken() == THIS);
    auto expr = llvm::make_unique<VariableExpr>("this", currentLoc());
    consumeToken();
    return expr;
}

std::unique_ptr<StrLiteralExpr> parseStrLiteral() {
    assert(currentToken() == STRING_LITERAL);
    auto expr = llvm::make_unique<StrLiteralExpr>(currentToken().string, currentLoc());
    consumeToken();
    return expr;
}

std::unique_ptr<IntLiteralExpr> parseIntLiteral() {
    assert(currentToken() == NUMBER);
    auto expr = llvm::make_unique<IntLiteralExpr>(currentToken().number, currentLoc());
    consumeToken();
    return expr;
}

std::unique_ptr<FloatLiteralExpr> parseFloatLiteral() {
    assert(currentToken() == FLOAT_LITERAL);
    auto expr = llvm::make_unique<FloatLiteralExpr>(currentToken().floatValue, currentLoc());
    consumeToken();
    return expr;
}

std::unique_ptr<BoolLiteralExpr> parseBoolLiteral() {
    std::unique_ptr<BoolLiteralExpr> expr;
    switch (currentToken()) {
        case TRUE: expr = llvm::make_unique<BoolLiteralExpr>(true, currentLoc()); break;
        case FALSE: expr = llvm::make_unique<BoolLiteralExpr>(false, currentLoc()); break;
        default: llvm_unreachable("all cases handled");
    }
    consumeToken();
    return expr;
}

std::unique_ptr<NullLiteralExpr> parseNullLiteral() {
    assert(currentToken() == NULL_LITERAL);
    auto expr = llvm::make_unique<NullLiteralExpr>(currentLoc());
    consumeToken();
    return expr;
}

/// array-literal ::= '[' expr-list ']'
std::unique_ptr<ArrayLiteralExpr> parseArrayLiteral() {
    assert(currentToken() == LBRACKET);
    auto location = currentLoc();
    consumeToken();
    auto elements = parseExprListUntil(RBRACKET);
    parse(RBRACKET);
    return llvm::make_unique<ArrayLiteralExpr>(std::move(elements), location);
}

/// simple-type ::= id | id '[' int-literal? ']'
Type parseSimpleType(bool isMutable) {
    assert(currentToken() == IDENTIFIER);
    auto type = BasicType::get(currentToken().string, isMutable);
    consumeToken();
    if (currentToken() != LBRACKET) return type;
    consumeToken();

    int64_t arraySize;
    if (currentToken() == RBRACKET)
        arraySize = ArrayType::unsized;
    else if (currentToken() == NUMBER)
        arraySize = consumeToken().number;
    else
        error(currentLoc(), "non-literal array bounds not implemented yet");

    parse(RBRACKET);
    return ArrayType::get(type, arraySize);
}

/// type ::= simple-type | 'mutable' simple-type | 'mutable' '(' type ')' | type '&' | type '*'
Type parseType() {
    Type type;
    switch (currentToken()) {
        case IDENTIFIER:
            type = parseSimpleType(false);
            break;
        case MUTABLE:
            consumeToken();
            if (currentToken() != LPAREN) {
                type = parseSimpleType(true);
            } else {
                consumeToken();
                type = parseType();
                parse(RPAREN);
            }
            type.setMutable(true);
            break;
        default:
            unexpectedToken(currentToken(), { IDENTIFIER, MUTABLE });
    }
    while (true) {
        switch (currentToken()) {
            case AND: case STAR:
                type = PtrType::get(type, currentToken() == AND);
                consumeToken();
                break;
            default:
                return type;
        }
    }
}

/// cast-expr ::= 'cast' '<' type '>' '(' expr ')'
std::unique_ptr<CastExpr> parseCastExpr() {
    assert(currentToken() == CAST);
    auto location = currentLoc();
    consumeToken();
    parse(LT);
    auto type = parseType();
    parse(GT);
    parse(LPAREN);
    auto expr = parseExpr();
    parse(RPAREN);
    return llvm::make_unique<CastExpr>(type, std::move(expr), location);
}

/// member-expr ::= expr '.' id
std::unique_ptr<MemberExpr> parseMemberExpr(std::unique_ptr<Expr> lhs) {
    auto member = parse(IDENTIFIER);
    return llvm::make_unique<MemberExpr>(std::move(lhs), std::move(member.string), member.getLoc());
}

/// subscript-expr ::= expr '[' expr ']'
std::unique_ptr<SubscriptExpr> parseSubscript(std::unique_ptr<Expr> operand) {
    assert(currentToken() == LBRACKET);
    auto location = currentLoc();
    consumeToken();
    auto index = parseExpr();
    parse(RBRACKET);
    return llvm::make_unique<SubscriptExpr>(std::move(operand), std::move(index), location);
}

/// unwrap-expr ::= expr '!'
std::unique_ptr<UnwrapExpr> parseUnwrapExpr(std::unique_ptr<Expr> operand) {
    assert(currentToken() == NOT);
    auto location = currentLoc();
    consumeToken();
    return llvm::make_unique<UnwrapExpr>(std::move(operand), location);
}

/// call-expr ::= expr generic-arg-list? '(' args ')'
/// generic-arg-list ::= '<' generic-args '>'
/// generic-args ::= type | type ',' generic-args
std::unique_ptr<CallExpr> parseCallExpr(std::unique_ptr<Expr> func) {
    std::vector<Type> genericArgs;
    if (currentToken() == LT) {
        consumeToken();
        while (true) {
            genericArgs.emplace_back(parseType());
            if (currentToken() == GT) break;
            parse(COMMA);
        }
        consumeToken();
    }
    auto location = currentLoc();
    auto args = parseArgList();
    return llvm::make_unique<CallExpr>(std::move(func), std::move(args), false,
                                       std::move(genericArgs), location);
}

/// paren-expr ::= '(' expr ')'
std::unique_ptr<Expr> parseParenExpr() {
    assert(currentToken() == LPAREN);
    consumeToken();
    auto expr = parseExpr();
    parse(RPAREN);
    return expr;
}

bool shouldParseGenericArgList() {
    // Temporary hack: use spacing to determine whether to parse a generic argument list
    // of a less-than binary expression. Zero spaces on either side of '<' will cause it
    // to be interpreted as a generic argument list, for now.
    return lookAhead(0).getLoc().column + std::strlen(lookAhead(0).string) == lookAhead(1).getLoc().column
        || lookAhead(1).getLoc().column + 1 == lookAhead(2).getLoc().column;
}

/// postfix-expr ::= postfix-expr postfix-op | call-expr | variable-expr | string-literal |
///                  int-literal | float-literal | bool-literal | null-literal |
///                  paren-expr | array-literal | cast-expr | subscript-expr | member-expr
///                  unwrap-expr
std::unique_ptr<Expr> parsePostfixExpr() {
    std::unique_ptr<Expr> expr;
    switch (currentToken()) {
        case IDENTIFIER:
            switch (lookAhead(1)) {
                case LPAREN: expr = parseCallExpr(parseVariableExpr()); break;
                case LT:
                    if (shouldParseGenericArgList()) {
                        expr = parseCallExpr(parseVariableExpr());
                        break;
                    }
                    // fallthrough
                default: expr = parseVariableExpr(); break;
            }
            break;
        case STRING_LITERAL: expr = parseStrLiteral(); break;
        case NUMBER: expr = parseIntLiteral(); break;
        case FLOAT_LITERAL: expr = parseFloatLiteral(); break;
        case TRUE: case FALSE: expr = parseBoolLiteral(); break;
        case NULL_LITERAL: expr = parseNullLiteral(); break;
        case THIS: expr = parseThis(); break;
        case LPAREN: expr = parseParenExpr(); break;
        case LBRACKET: expr = parseArrayLiteral(); break;
        case CAST: expr = parseCastExpr(); break;
        default: unexpectedToken(currentToken()); break;
    }
    while (true) {
        switch (currentToken()) {
            case LBRACKET:
                expr = parseSubscript(std::move(expr));
                break;
            case LPAREN:
                expr = parseCallExpr(std::move(expr));
                break;
            case DOT:
                consumeToken();
                expr = parseMemberExpr(std::move(expr));
                break;
            case NOT:
                expr = parseUnwrapExpr(std::move(expr));
                break;
            default:
                return expr;
        }
    }
}

/// prefix-expr ::= prefix-operator (prefix-expr | postfix-expr)
std::unique_ptr<PrefixExpr> parsePrefixExpr() {
    assert(currentToken().isPrefixOperator());
    auto op = currentToken();
    auto location = currentLoc();
    consumeToken();
    return llvm::make_unique<PrefixExpr>(op, parsePreOrPostfixExpr(), location);
}

std::unique_ptr<Expr> parsePreOrPostfixExpr() {
    return currentToken().isPrefixOperator() ? parsePrefixExpr() : parsePostfixExpr();
}

/// binary-expr ::= expr op expr
std::unique_ptr<Expr> parseBinaryExpr(std::unique_ptr<Expr> lhs, int minPrecedence) {
    while (currentToken().isBinaryOperator() && currentToken().getPrecedence() >= minPrecedence) {
        auto op = consumeToken();
        auto rhs = parsePreOrPostfixExpr();
        while (currentToken().isBinaryOperator() && currentToken().getPrecedence() > op.getPrecedence()) {
            rhs = parseBinaryExpr(std::move(rhs), currentToken().getPrecedence());
        }
        lhs = llvm::make_unique<BinaryExpr>(op, std::move(lhs), std::move(rhs), op.getLoc());
    }
    return lhs;
}

/// expr ::= prefix-expr | postfix-expr | binary-expr
std::unique_ptr<Expr> parseExpr() {
    return parseBinaryExpr(parsePreOrPostfixExpr(), 0);
}

/// assign-stmt ::= expr '=' expr ';'
std::unique_ptr<AssignStmt> parseAssignStmt(std::unique_ptr<Expr> lhs) {
    auto loc = currentLoc();
    parse(ASSIGN);
    auto rhs = parseExpr();
    parse(SEMICOLON);
    return llvm::make_unique<AssignStmt>(std::move(lhs), std::move(rhs), loc);
}

/// compound-assign-stmt ::= expr compound-assignment-op expr ';'
std::unique_ptr<AugAssignStmt> parseCompoundAssignStmt(std::unique_ptr<Expr> lhs = nullptr) {
    if (!lhs) lhs = parseExpr();
    auto op = BinaryOperator(consumeToken().withoutCompoundEqSuffix());
    SrcLoc loc = currentLoc();
    auto rhs = parseExpr();
    parse(SEMICOLON);
    return llvm::make_unique<AugAssignStmt>(std::move(lhs), std::move(rhs), op, loc);
}

/// expr-list ::= '' | nonempty-expr-list
/// nonempty-expr-list ::= expr | expr ',' nonempty-expr-list
std::vector<std::unique_ptr<Expr>> parseExprListUntil(Token end) {
    std::vector<std::unique_ptr<Expr>> exprs;
    if (currentToken() == end) return exprs;
    while (true) {
        exprs.emplace_back(parseExpr());
        if (currentToken() == end) return exprs;
        parse(COMMA);
    }
}

/// return-stmt ::= 'return' expr-list ';'
std::unique_ptr<ReturnStmt> parseReturnStmt() {
    assert(currentToken() == RETURN);
    auto location = currentLoc();
    consumeToken();
    auto returnValues = parseExprListUntil(SEMICOLON);
    parse(SEMICOLON);
    return llvm::make_unique<ReturnStmt>(std::move(returnValues), location);
}

/// var-stmt ::= type-specifier id '=' initializer ';'
/// type-specifier ::= type | 'var' | 'const'
/// initializer ::= expr | 'uninitialized'
std::unique_ptr<VarDecl> parseVarDeclFromId(Type type) {
    auto name = parse(IDENTIFIER);
    parse(ASSIGN);
    auto initializer = currentToken() != UNINITIALIZED ? parseExpr() : nullptr;
    if (!initializer) consumeToken();
    parse(SEMICOLON);
    return llvm::make_unique<VarDecl>(type, std::move(name.string),
                                      std::move(initializer), name.getLoc());
}

std::unique_ptr<VariableStmt> parseVarStmtFromId(Type type) {
    return llvm::make_unique<VariableStmt>(parseVarDeclFromId(type).release());
}

/// call-stmt ::= call-expr ';'
std::unique_ptr<ExprStmt> parseCallStmt(std::unique_ptr<Expr> callExpr) {
    assert(currentToken() == SEMICOLON);
    assert(callExpr->isCallExpr());
    auto stmt = llvm::make_unique<ExprStmt>(std::move(callExpr));
    consumeToken();
    return stmt;
}

/// inc-stmt ::= expr '++' ';'
std::unique_ptr<IncrementStmt> parseIncrementStmt(std::unique_ptr<Expr> operand) {
    auto location = currentLoc();
    parse(INCREMENT);
    parse(SEMICOLON);
    return llvm::make_unique<IncrementStmt>(std::move(operand), location);
}

/// dec-stmt ::= expr '--' ';'
std::unique_ptr<DecrementStmt> parseDecrementStmt(std::unique_ptr<Expr> operand) {
    auto location = currentLoc();
    parse(DECREMENT);
    parse(SEMICOLON);
    return llvm::make_unique<DecrementStmt>(std::move(operand), location);
}

/// defer-stmt ::= 'defer' call-expr ';'
std::unique_ptr<DeferStmt> parseDeferStmt() {
    assert(currentToken() == DEFER);
    consumeToken();
    // FIXME: Doesn't have to be a variable expression.
    auto stmt = llvm::make_unique<DeferStmt>(parseCallExpr(parseVariableExpr()));
    parse(SEMICOLON);
    return stmt;
}

/// if-stmt ::= 'if' '(' expr ')' '{' stmt* '}' ('else' else-branch)?
/// else-branch ::= if-stmt | '{' stmt* '}'
std::unique_ptr<IfStmt> parseIfStmt() {
    assert(currentToken() == IF);
    consumeToken();
    parse(LPAREN);
    auto condition = parseExpr();
    parse(RPAREN);
    parse(LBRACE);
    auto thenStmts = parseStmtsUntil(RBRACE);
    parse(RBRACE);
    std::vector<std::unique_ptr<Stmt>> elseStmts;
    if (currentToken() != ELSE)
        return llvm::make_unique<IfStmt>(std::move(condition), std::move(thenStmts),
                                         std::move(elseStmts));
    consumeToken();
    if (currentToken() == LBRACE) {
        consumeToken();
        elseStmts = parseStmtsUntil(RBRACE);
        parse(RBRACE);
        return llvm::make_unique<IfStmt>(std::move(condition), std::move(thenStmts),
                                         std::move(elseStmts));
    }
    if (currentToken() == IF) {
        elseStmts.emplace_back(parseIfStmt());
        return llvm::make_unique<IfStmt>(std::move(condition), std::move(thenStmts),
                                         std::move(elseStmts));
    }
    unexpectedToken(currentToken(), { LBRACE, IF });
}

/// while-stmt ::= 'while' '(' expr ')' '{' stmt* '}'
std::unique_ptr<WhileStmt> parseWhileStmt() {
    assert(currentToken() == WHILE);
    consumeToken();
    parse(LPAREN);
    auto condition = parseExpr();
    parse(RPAREN);
    parse(LBRACE);
    auto body = parseStmtsUntil(RBRACE);
    parse(RBRACE);
    return llvm::make_unique<WhileStmt>(std::move(condition), std::move(body));
}

/// switch-stmt ::= 'switch' '(' expr ')' '{' cases default-case? '}'
/// cases ::= case | case cases
/// case ::= 'case' expr ':' stmt+
/// default-case ::= 'default' ':' stmt+
std::unique_ptr<SwitchStmt> parseSwitchStmt() {
    assert(currentToken() == SWITCH);
    consumeToken();
    parse(LPAREN);
    auto condition = parseExpr();
    parse(RPAREN);
    parse(LBRACE);
    std::vector<SwitchCase> cases;
    std::vector<std::unique_ptr<Stmt>> defaultStmts;
    bool defaultSeen = false;
    while (true) {
        if (currentToken() == CASE) {
            consumeToken();
            auto value = parseExpr();
            parse(COLON);
            auto stmts = parseStmtsUntilOneOf(CASE, DEFAULT, RBRACE);
            cases.push_back({ std::move(value), std::move(stmts) });
        } else if (currentToken() == DEFAULT) {
            if (defaultSeen)
                error(currentLoc(), "switch-statement may only contain one 'default' case");
            consumeToken();
            parse(COLON);
            defaultStmts = parseStmtsUntilOneOf(CASE, DEFAULT, RBRACE);
            defaultSeen = true;
        } else {
            error(currentLoc(), "expected 'case' or 'default'");
        }
        if (currentToken() == RBRACE) break;
    }
    consumeToken();
    return llvm::make_unique<SwitchStmt>(std::move(condition), std::move(cases),
                                         std::move(defaultStmts));
}

/// break-stmt ::= 'break' ';'
std::unique_ptr<BreakStmt> parseBreakStmt() {
    auto location = currentLoc();
    consumeToken();
    parse(SEMICOLON);
    return llvm::make_unique<BreakStmt>(location);
}

/// stmt ::= var-stmt | assign-stmt | compound-assign-stmt | return-stmt |
///          inc-stmt | dec-stmt | call-stmt | defer-stmt |
///          if-stmt | switch-stmt | while-stmt | break-stmt
std::unique_ptr<Stmt> parseStmt() {
    switch (currentToken()) {
        case IDENTIFIER:
            if (lookAhead(1).is(IDENTIFIER, AND, STAR)
            || (lookAhead(1) == LBRACKET && lookAhead(2) == NUMBER
            &&  lookAhead(3) == RBRACKET && lookAhead(4).is(IDENTIFIER, AND, STAR)))
                return parseVarStmtFromId(parseType());
            break;
        case RETURN: return parseReturnStmt();
        case VAR: case CONST: return parseVarStmtFromId(Type(nullptr, consumeToken() == VAR));
        case MUTABLE: return parseVarStmtFromId(parseType());
        case DEFER: return parseDeferStmt();
        case IF: return parseIfStmt();
        case WHILE: return parseWhileStmt();
        case SWITCH: return parseSwitchStmt();
        case BREAK: return parseBreakStmt();
        case UNDERSCORE: {
            consumeToken();
            parse(ASSIGN);
            auto stmt = llvm::make_unique<ExprStmt>(parseExpr());
            parse(SEMICOLON);
            return std::move(stmt);
        }
        default: break;
    }

    // If we're here, the statement starts with an expression.
    std::unique_ptr<Expr> expr = parseExpr();

    switch (currentToken()) {
        case SEMICOLON:
            if (!expr->isCallExpr()) unexpectedToken(currentToken());
            return parseCallStmt(std::move(expr));
        case INCREMENT: return parseIncrementStmt(std::move(expr));
        case DECREMENT: return parseDecrementStmt(std::move(expr));
        case ASSIGN: return parseAssignStmt(std::move(expr));
        case PLUS_EQ: case MINUS_EQ: case STAR_EQ: case SLASH_EQ:
        case AND_EQ: case AND_AND_EQ: case OR_EQ: case OR_OR_EQ:
        case XOR_EQ: case LSHIFT_EQ: case RSHIFT_EQ:
            return parseCompoundAssignStmt(std::move(expr));
        default: unexpectedToken(currentToken());
    }
}

std::vector<std::unique_ptr<Stmt>> parseStmtsUntil(Token end) {
    std::vector<std::unique_ptr<Stmt>> stmts;
    while (currentToken() != end)
        stmts.emplace_back(parseStmt());
    return stmts;
}

std::vector<std::unique_ptr<Stmt>> parseStmtsUntilOneOf(Token end1, Token end2, Token end3) {
    std::vector<std::unique_ptr<Stmt>> stmts;
    while (currentToken() != end1 && currentToken() != end2  && currentToken() != end3)
        stmts.emplace_back(parseStmt());
    return stmts;
}

/// param-decl ::= (id ':')? type id
ParamDecl parseParam() {
    std::string label;
    if (lookAhead(1) == COLON) {
        label = parse(IDENTIFIER).string;
        consumeToken();
    }
    auto type = parseType();
    auto name = parse(IDENTIFIER);
    return ParamDecl(std::move(label), std::move(type), std::move(name.string), name.getLoc());
}

/// param-list ::= '(' params ')'
/// params ::= '' | non-empty-params
/// non-empty-params ::= param-decl | param-decl ',' non-empty-params
std::vector<ParamDecl> parseParamList() {
    parse(LPAREN);
    std::vector<ParamDecl> params;
    while (currentToken() != RPAREN) {
        params.emplace_back(parseParam());
        if (currentToken() != RPAREN) parse(COMMA);
    }
    parse(RPAREN);
    return params;
}

/// func-proto ::= 'func' (id '::')? id param-list ('->' type)?
/// generic-func-proto ::= 'func' (id '::')? id generic-param-list param-list ('->' type)?
/// generic-param-list ::= '<' generic-param-decls '>'
/// generic-param-decls ::= id | id ',' generic-param-decls
std::unique_ptr<FuncDecl> parseFuncProto(std::vector<GenericParamDecl>* genericParams = nullptr) {
    assert(currentToken() == FUNC);
    consumeToken();

    std::string receiverType;
    if (lookAhead(1) == COLON_COLON) {
        receiverType = parse(IDENTIFIER).string;
        parse(COLON_COLON);
    }

    auto name = parse(IDENTIFIER);

    if (genericParams) {
        parse(LT);
        while (true) {
            auto genericParamName = parse(IDENTIFIER);
            genericParams->emplace_back(genericParamName.string, genericParamName.getLoc());
            if (currentToken() == GT) break;
            parse(COMMA);
        }
        parse(GT);
    }

    auto params = parseParamList();

    Type returnType = Type::getVoid();
    if (currentToken() == RARROW) {
        consumeToken();
        returnType = parseType();
        while (currentToken() == COMMA) {
            consumeToken();
            returnType.appendType(parseType());
        }
    }

    return llvm::make_unique<FuncDecl>(std::move(name.string), std::move(params),
                                       std::move(returnType), std::move(receiverType),
                                       name.getLoc());
}

/// func-decl ::= func-proto '{' stmt* '}'
std::unique_ptr<FuncDecl> parseFuncDecl(std::vector<GenericParamDecl>* genericParams = nullptr) {
    auto decl = parseFuncProto(genericParams);
    parse(LBRACE);
    decl->body = std::make_shared<std::vector<std::unique_ptr<Stmt>>>(parseStmtsUntil(RBRACE));
    parse(RBRACE);
    return decl;
}

/// generic-func-decl ::= generic-func-proto '{' stmt* '}'
std::unique_ptr<GenericFuncDecl> parseGenericFuncDecl() {
    std::vector<GenericParamDecl> genericParams;
    auto func = parseFuncDecl(&genericParams);
    return llvm::make_unique<GenericFuncDecl>(std::move(func), std::move(genericParams));
}

/// extern-func-decl ::= 'extern' func-proto ';'
std::unique_ptr<FuncDecl> parseExternFuncDecl() {
    assert(currentToken() == EXTERN);
    consumeToken();
    auto decl = parseFuncProto();
    parse(SEMICOLON);
    return decl;
}

/// init-decl ::= id '::' 'init' param-list '{' stmt* '}'
std::unique_ptr<InitDecl> parseInitDecl() {
    assert(currentToken() == IDENTIFIER);
    auto type = parse(IDENTIFIER);
    parse(COLON_COLON);
    parse(INIT);
    auto params = parseParamList();
    parse(LBRACE);
    auto body = std::make_shared<std::vector<std::unique_ptr<Stmt>>>(parseStmtsUntil(RBRACE));
    parse(RBRACE);
    return llvm::make_unique<InitDecl>(std::move(type.string), std::move(params),
                                       std::move(body), type.getLoc());
}

/// deinit-decl ::= id '::' 'deinit' '(' ')' '{' stmt* '}'
std::unique_ptr<DeinitDecl> parseDeinitDecl() {
    assert(currentToken() == IDENTIFIER);
    auto type = parse(IDENTIFIER);
    parse(COLON_COLON);
    parse(DEINIT);
    parse(LPAREN);
    if (consumeToken() != RPAREN) error(currentLoc(), "deinitializers cannot have parameters");
    parse(LBRACE);
    auto body = std::make_shared<std::vector<std::unique_ptr<Stmt>>>(parseStmtsUntil(RBRACE));
    parse(RBRACE);
    return llvm::make_unique<DeinitDecl>(std::move(type.string), std::move(body), type.getLoc());
}

/// field-decl ::= type id ';'
FieldDecl parseFieldDecl() {
    auto type = parseType();
    auto name = parse(IDENTIFIER);
    parse(SEMICOLON);
    return FieldDecl(type, std::move(name.string), name.getLoc());
}

/// type-decl ::= ('class' | 'struct') id '{' field-decl* '}'
std::unique_ptr<TypeDecl> parseTypeDecl() {
    assert(currentToken() == CLASS || currentToken() == STRUCT);
    auto tag = consumeToken() == CLASS ? TypeTag::Class : TypeTag::Struct;
    auto name = parse(IDENTIFIER);
    parse(LBRACE);
    std::vector<FieldDecl> fields;
    while (currentToken() != RBRACE)
        fields.emplace_back(parseFieldDecl());
    consumeToken();
    return llvm::make_unique<TypeDecl>(tag, std::move(name.string), std::move(fields), name.getLoc());
}

/// import-decl ::= 'import' string-literal ';'
std::unique_ptr<ImportDecl> parseImportDecl() {
    assert(currentToken() == IMPORT);
    consumeToken();
    expect(STRING_LITERAL, "after 'import'");
    auto target = parseStrLiteral();
    parse(SEMICOLON, "after 'import' declaration");
    return llvm::make_unique<ImportDecl>(std::move(target->value), target->getSrcLoc());
}

/// decl ::= func-decl | generic-func-decl | extern-func-decl | init-decl | deinit-decl |
///          type-decl | import-decl | var-decl
std::unique_ptr<Decl> parseDecl() {
    switch (currentToken()) {
        case FUNC:
            if (lookAhead(2) != LT) {
                auto decl = parseFuncDecl();
                addToSymbolTable(*decl);
                return std::move(decl);
            } else {
                auto decl = parseGenericFuncDecl();
                addToSymbolTable(*decl);
                return std::move(decl);
            }
        case EXTERN: {
            auto decl = parseExternFuncDecl();
            addToSymbolTable(*decl);
            return std::move(decl);
        }
        case IDENTIFIER:
            if (lookAhead(1) == COLON_COLON) {
                switch (lookAhead(2)) {
                    case INIT: {
                        auto decl = parseInitDecl();
                        addToSymbolTable(*decl);
                        return std::move(decl);
                    }
                    case DEINIT: {
                        auto decl = parseDeinitDecl();
                        addToSymbolTable(*decl);
                        return std::move(decl);
                    }
                    default: unexpectedToken(lookAhead(2), { INIT, DEINIT });
                }
            }
            // fallthrough
        case MUTABLE:
            return parseVarDeclFromId(parseType());
        case CLASS: case STRUCT: {
            auto decl = parseTypeDecl();
            addToSymbolTable(*decl);
            return std::move(decl);
        }
        case VAR: case CONST:
            return parseVarDeclFromId(Type(nullptr, consumeToken() == VAR));
        case IMPORT:
            return parseImportDecl();
        default:
            unexpectedToken(currentToken());
    }
}

}

FileUnit delta::parse(const char* filePath) {
    initLexer(filePath);
    tokenBuffer.clear();
    currentTokenIndex = 0;
    tokenBuffer.emplace_back(lex());

    std::vector<std::unique_ptr<Decl>> topLevelDecls;
    while (currentToken() != NO_TOKEN)
        topLevelDecls.emplace_back(parseDecl());
    return FileUnit(std::move(topLevelDecls));
}
