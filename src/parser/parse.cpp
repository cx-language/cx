#include <vector>
#include <sstream>
#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/Support/ErrorHandling.h>
#include "parse.h"
#include "lex.h"
#include "../ast/token.h"
#include "../ast/decl.h"
#include "../ast/module.h"
#include "../sema/typecheck.h"
#include "../driver/utility.h"

using namespace delta;

namespace {

std::vector<Token> tokenBuffer;
size_t currentTokenIndex;

Token currentToken() {
    assert(currentTokenIndex < tokenBuffer.size());
    return tokenBuffer[currentTokenIndex];
}

SrcLoc currentLoc() {
    return currentToken().getLoc();
}

Token lookAhead(int offset) {
    if (int(currentTokenIndex) + offset < 0) return NO_TOKEN;
    int count = int(currentTokenIndex) + offset - int(tokenBuffer.size()) + 1;
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

std::string toString(llvm::ArrayRef<TokenKind> expected) {
    std::string string;
    for (const TokenKind& tokenKind : expected) {
        string += quote(tokenKind);
        if (expected.size() > 2) string += ',';
        if (&tokenKind == expected.end() - 2) string += " or ";
    }
    return string;
}

[[noreturn]] void unexpectedToken(Token token, llvm::ArrayRef<TokenKind> expected = {},
                                  const char* contextInfo = nullptr) {
    if (expected.size() == 0) {
        error(token.getLoc(), "unexpected ", quote(token),
              contextInfo ? " " : "", contextInfo ? contextInfo : "");
    } else {
        error(token.getLoc(), "expected ", toString(expected),
              contextInfo ? " " : "", contextInfo ? contextInfo : "",
              ", got ", quote(token));
    }
}

void expect(llvm::ArrayRef<TokenKind> expected, const char* contextInfo) {
    if (!llvm::is_contained(expected, currentToken())) {
        unexpectedToken(currentToken(), expected, contextInfo);
    }
}

Token parse(llvm::ArrayRef<TokenKind> expected, const char* contextInfo = nullptr) {
    expect(expected, contextInfo);
    return consumeToken();
}

void parseStmtTerminator(const char* contextInfo = nullptr) {
    if (currentLoc().line != lookAhead(-1).getLoc().line)
        return; // Ends with a newline.

    switch (currentToken()) {
        case RBRACE: return; // Allow 'if (x) { stmt }'.
        case SEMICOLON: consumeToken(); return;
        default: unexpectedToken(currentToken(), { NEWLINE, SEMICOLON }, contextInfo);
    }
}

std::unique_ptr<Expr> parseExpr();
std::unique_ptr<Expr> parsePreOrPostfixExpr();
std::vector<std::unique_ptr<Expr>> parseExprList();
std::vector<std::unique_ptr<Stmt>> parseStmtsUntil(Token end);
std::vector<std::unique_ptr<Stmt>> parseStmtsUntilOneOf(Token end1, Token end2, Token end3);

/// arg-list ::= '(' ')' | '(' nonempty-arg-list ')'
/// nonempty-arg-list ::= arg | nonempty-arg-list ',' arg
/// arg ::= (id ':')? expr
std::vector<Arg> parseArgList() {
    parse(LPAREN);
    std::vector<Arg> args;
    while (currentToken() != RPAREN) {
        std::string name;
        SrcLoc location = SrcLoc::invalid();
        if (lookAhead(1) == COLON) {
            auto result = parse(IDENTIFIER);
            name = std::move(result.string);
            location = result.getLoc();
            consumeToken();
        }
        auto value = parseExpr();
        if (!location.isValid()) location = value->getSrcLoc();
        args.push_back({ std::move(name), std::move(value), location });
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
    llvm::StringRef content = currentToken().string.drop_back().drop_front();
    auto expr = llvm::make_unique<StrLiteralExpr>(content, currentLoc());
    consumeToken();
    return expr;
}

std::unique_ptr<IntLiteralExpr> parseIntLiteral() {
    assert(currentToken() == INT_LITERAL);
    auto expr = llvm::make_unique<IntLiteralExpr>(currentToken().getIntegerValue(),
                                                  currentLoc());
    consumeToken();
    return expr;
}

std::unique_ptr<FloatLiteralExpr> parseFloatLiteral() {
    assert(currentToken() == FLOAT_LITERAL);
    auto expr = llvm::make_unique<FloatLiteralExpr>(currentToken().getFloatingPointValue(),
                                                    currentLoc());
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
    auto elements = parseExprList();
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
    else if (currentToken() == INT_LITERAL)
        arraySize = consumeToken().getIntegerValue();
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
    return lookAhead(0).getLoc().column + int(lookAhead(0).string.size()) == lookAhead(1).getLoc().column
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
        case INT_LITERAL: expr = parseIntLiteral(); break;
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

/// assign-stmt ::= expr '=' expr ('\n' | ';')
std::unique_ptr<AssignStmt> parseAssignStmt(std::unique_ptr<Expr> lhs) {
    auto loc = currentLoc();
    parse(ASSIGN);
    auto rhs = parseExpr();
    parseStmtTerminator();
    return llvm::make_unique<AssignStmt>(std::move(lhs), std::move(rhs), loc);
}

/// compound-assign-stmt ::= expr compound-assignment-op expr ('\n' | ';')
std::unique_ptr<AugAssignStmt> parseCompoundAssignStmt(std::unique_ptr<Expr> lhs = nullptr) {
    if (!lhs) lhs = parseExpr();
    auto op = BinaryOperator(consumeToken().withoutCompoundEqSuffix());
    SrcLoc loc = currentLoc();
    auto rhs = parseExpr();
    parseStmtTerminator();
    return llvm::make_unique<AugAssignStmt>(std::move(lhs), std::move(rhs), op, loc);
}

/// expr-list ::= '' | nonempty-expr-list
/// nonempty-expr-list ::= expr | expr ',' nonempty-expr-list
std::vector<std::unique_ptr<Expr>> parseExprList() {
    std::vector<std::unique_ptr<Expr>> exprs;

    // TODO: Handle empty expression list.
    if (currentToken() == SEMICOLON || currentToken() == RBRACE) {
        return exprs;
    }

    while (true) {
        exprs.emplace_back(parseExpr());
        if (currentToken() != COMMA) return exprs;
        consumeToken();
    }
}

/// return-stmt ::= 'return' expr-list ('\n' | ';')
std::unique_ptr<ReturnStmt> parseReturnStmt() {
    assert(currentToken() == RETURN);
    auto location = currentLoc();
    consumeToken();
    auto returnValues = parseExprList();
    parseStmtTerminator();
    return llvm::make_unique<ReturnStmt>(std::move(returnValues), location);
}

/// var-stmt ::= type-specifier id '=' initializer ('\n' | ';')
/// type-specifier ::= type | 'var' | 'const'
/// initializer ::= expr | 'uninitialized'
std::unique_ptr<VarDecl> parseVarDeclFromId(Type type) {
    auto name = parse(IDENTIFIER);
    parse(ASSIGN);
    auto initializer = currentToken() != UNINITIALIZED ? parseExpr() : nullptr;
    if (!initializer) consumeToken();
    parseStmtTerminator();
    return llvm::make_unique<VarDecl>(type, std::move(name.string),
                                      std::move(initializer), name.getLoc());
}

std::unique_ptr<VariableStmt> parseVarStmtFromId(Type type) {
    return llvm::make_unique<VariableStmt>(parseVarDeclFromId(type));
}

/// call-stmt ::= call-expr ('\n' | ';')
std::unique_ptr<ExprStmt> parseCallStmt(std::unique_ptr<Expr> callExpr) {
    assert(callExpr->isCallExpr());
    auto stmt = llvm::make_unique<ExprStmt>(std::move(callExpr));
    parseStmtTerminator();
    return stmt;
}

/// inc-stmt ::= expr '++' ('\n' | ';')
std::unique_ptr<IncrementStmt> parseIncrementStmt(std::unique_ptr<Expr> operand) {
    auto location = currentLoc();
    parse(INCREMENT);
    parseStmtTerminator();
    return llvm::make_unique<IncrementStmt>(std::move(operand), location);
}

/// dec-stmt ::= expr '--' ('\n' | ';')
std::unique_ptr<DecrementStmt> parseDecrementStmt(std::unique_ptr<Expr> operand) {
    auto location = currentLoc();
    parse(DECREMENT);
    parseStmtTerminator();
    return llvm::make_unique<DecrementStmt>(std::move(operand), location);
}

/// defer-stmt ::= 'defer' call-expr ('\n' | ';')
std::unique_ptr<DeferStmt> parseDeferStmt() {
    assert(currentToken() == DEFER);
    consumeToken();
    // FIXME: Doesn't have to be a variable expression.
    auto stmt = llvm::make_unique<DeferStmt>(parseCallExpr(parseVariableExpr()));
    parseStmtTerminator();
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

/// for-stmt ::= 'for' '(' id 'in' expr ')' '{' stmt* '}'
std::unique_ptr<ForStmt> parseForStmt() {
    assert(currentToken() == FOR);
    consumeToken();
    parse(LPAREN);
    auto id = parse(IDENTIFIER);
    parse(IN);
    auto range = parseExpr();
    parse(RPAREN);
    parse(LBRACE);
    auto body = parseStmtsUntil(RBRACE);
    parse(RBRACE);
    return llvm::make_unique<ForStmt>(std::string(id.string), std::move(range),
                                      std::move(body), id.getLoc());
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

/// break-stmt ::= 'break' ('\n' | ';')
std::unique_ptr<BreakStmt> parseBreakStmt() {
    auto location = currentLoc();
    consumeToken();
    parseStmtTerminator();
    return llvm::make_unique<BreakStmt>(location);
}

/// stmt ::= var-stmt | assign-stmt | compound-assign-stmt | return-stmt |
///          inc-stmt | dec-stmt | call-stmt | defer-stmt |
///          if-stmt | switch-stmt | while-stmt | for-stmt | break-stmt
std::unique_ptr<Stmt> parseStmt() {
    switch (currentToken()) {
        case IDENTIFIER:
            if (lookAhead(1).is(IDENTIFIER, AND, STAR))
                return parseVarStmtFromId(parseType());

            if (lookAhead(1) == LBRACKET && lookAhead(2) == INT_LITERAL
            &&  lookAhead(3) == RBRACKET && lookAhead(4).is(IDENTIFIER, AND, STAR))
                return parseVarStmtFromId(parseType());

            if (lookAhead(1) == LBRACKET
            &&  lookAhead(2) == RBRACKET && lookAhead(3).is(IDENTIFIER, AND, STAR))
                return parseVarStmtFromId(parseType());

            break;
        case RETURN: return parseReturnStmt();
        case VAR: case CONST: return parseVarStmtFromId(Type(nullptr, consumeToken() == VAR));
        case MUTABLE: return parseVarStmtFromId(parseType());
        case DEFER: return parseDeferStmt();
        case IF: return parseIfStmt();
        case WHILE: return parseWhileStmt();
        case FOR: return parseForStmt();
        case SWITCH: return parseSwitchStmt();
        case BREAK: return parseBreakStmt();
        case UNDERSCORE: {
            consumeToken();
            parse(ASSIGN);
            auto stmt = llvm::make_unique<ExprStmt>(parseExpr());
            parseStmtTerminator();
            return std::move(stmt);
        }
        default: break;
    }

    // If we're here, the statement starts with an expression.
    std::unique_ptr<Expr> expr = parseExpr();

    switch (currentToken()) {
        case INCREMENT: return parseIncrementStmt(std::move(expr));
        case DECREMENT: return parseDecrementStmt(std::move(expr));
        case ASSIGN: return parseAssignStmt(std::move(expr));
        case PLUS_EQ: case MINUS_EQ: case STAR_EQ: case SLASH_EQ: case MOD_EQ:
        case AND_EQ: case AND_AND_EQ: case OR_EQ: case OR_OR_EQ:
        case XOR_EQ: case LSHIFT_EQ: case RSHIFT_EQ:
            return parseCompoundAssignStmt(std::move(expr));
        default:
            if (!expr->isCallExpr()) unexpectedToken(currentToken());
            return parseCallStmt(std::move(expr));
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

/// param-decl ::= type id
ParamDecl parseParam() {
    auto type = parseType();
    auto name = parse(IDENTIFIER);
    return ParamDecl(std::move(type), std::move(name.string), name.getLoc());
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

    if (currentToken() != IDENTIFIER && !currentToken().isOverloadable())
        unexpectedToken(currentToken(), {}, "as function name");

    SrcLoc nameLoc = currentLoc();
    llvm::StringRef name;
    if (currentToken() == IDENTIFIER) {
        name = consumeToken().string;
    } else if (!receiverType.empty()) {
        error(nameLoc, "operator functions must be non-member functions");
    } else {
        name = toString(consumeToken().kind);
    }

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

    return llvm::make_unique<FuncDecl>(std::move(name), std::move(params),
                                       std::move(returnType), std::move(receiverType),
                                       nameLoc);
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

/// extern-func-decl ::= 'extern' func-proto ('\n' | ';')
std::unique_ptr<FuncDecl> parseExternFuncDecl() {
    assert(currentToken() == EXTERN);
    consumeToken();
    auto decl = parseFuncProto();
    parseStmtTerminator();
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

/// field-decl ::= type id ('\n' | ';')
FieldDecl parseFieldDecl() {
    auto type = parseType();
    auto name = parse(IDENTIFIER);
    parseStmtTerminator();
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

/// import-decl ::= 'import' string-literal ('\n' | ';')
std::unique_ptr<ImportDecl> parseImportDecl() {
    assert(currentToken() == IMPORT);
    consumeToken();
    expect(STRING_LITERAL, "after 'import'");
    auto target = parseStrLiteral();
    parseStmtTerminator("after 'import' declaration");
    return llvm::make_unique<ImportDecl>(std::move(target->value), target->getSrcLoc());
}

/// decl ::= func-decl | generic-func-decl | extern-func-decl | init-decl | deinit-decl |
///          type-decl | import-decl | var-decl
std::unique_ptr<Decl> parseDecl() {
    switch (currentToken()) {
        case MUTATING:
            consumeToken();
            expect(FUNC, "after 'mutating'");
            // fallthrough
        case FUNC: {
            bool isMutating = lookAhead(-1) == MUTATING;
            if (lookAhead(2) != LT) {
                auto decl = parseFuncDecl();
                decl->setMutating(isMutating);
                addToSymbolTable(*decl);
                return std::move(decl);
            } else {
                auto decl = parseGenericFuncDecl();
                decl->func->setMutating(isMutating);
                addToSymbolTable(*decl);
                return std::move(decl);
            }
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
        case MUTABLE: {
            auto decl = parseVarDeclFromId(parseType());
            addToSymbolTable(*decl);
            return std::move(decl);
        }
        case CLASS: case STRUCT: {
            auto decl = parseTypeDecl();
            addToSymbolTable(*decl);
            return std::move(decl);
        }
        case VAR: case CONST: {
            auto decl = parseVarDeclFromId(Type(nullptr, consumeToken() == VAR));
            addToSymbolTable(*decl);
            return std::move(decl);
        }
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
    return FileUnit(filePath, std::move(topLevelDecls));
}
