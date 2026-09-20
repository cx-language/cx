#include "parse.h"
#include <forward_list>
#include <sstream>
#include <vector>
#pragma warning(push, 0)
#include <llvm/ADT/APSInt.h>
#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/ErrorHandling.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/SaveAndRestore.h>
#pragma warning(pop)
#include "../ast/arena.h"
#include "../ast/decl.h"
#include "../ast/module.h"
#include "../ast/token.h"
#include "../driver/driver.h"
#include "../support/utility.h"
#include "lex.h"

using namespace cx;

Parser::Parser(llvm::MemoryBufferRef input, Module& module, const CompileOptions& options)
: lexer(input), currentModule(&module), currentTokenIndex(0), options(options) {
    tokenBuffer.emplace_back(lexer.nextToken());
}

Token Parser::currentToken() {
    ASSERT(currentTokenIndex < tokenBuffer.size());
    return tokenBuffer[currentTokenIndex];
}

Location Parser::getCurrentLocation() {
    return currentToken().location;
}

Token Parser::lookAhead(int offset) {
    if (int(currentTokenIndex) + offset < 0) return Token(Token::None, Location());
    int count = int(currentTokenIndex) + offset - int(tokenBuffer.size()) + 1;
    while (count-- > 0) {
        tokenBuffer.emplace_back(lexer.nextToken());
    }
    return tokenBuffer[currentTokenIndex + offset];
}

Token Parser::consumeToken() {
    Token token = currentToken();
    if (currentTokenIndex + 1 == tokenBuffer.size()) {
        tokenBuffer.emplace_back(lexer.nextToken());
    }
    currentTokenIndex++;
    return token;
}

/// Adds quotes around the string representation of the given token unless
/// it's an identifier, numeric literal, string literal, or end-of-file.
static std::string quote(Token::Kind tokenKind) {
    std::ostringstream stream;
    if (tokenKind < Token::Break) {
        stream << tokenKind;
    } else {
        stream << '\'' << tokenKind << '\'';
    }
    return stream.str();
}

static std::string formatList(llvm::ArrayRef<Token::Kind> tokens) {
    std::string result;

    for (auto& token : tokens) {
        result += quote(token);

        if (tokens.size() > 2 && &token != &tokens.back()) {
            result += ", ";
        }

        if (&token == &tokens.back() - 1) {
            if (tokens.size() == 2) {
                result += " ";
            }
            result += "or ";
        }
    }

    return result;
}

[[noreturn]] static void unexpectedToken(Token token, llvm::ArrayRef<Token::Kind> expected = {}, const char* contextInfo = nullptr) {
    if (expected.size() == 0) {
        ERROR(token.location, "unexpected " << quote(token) << (contextInfo ? " " : "") << (contextInfo ? contextInfo : ""));
    } else {
        ERROR(token.location, "expected " << formatList(expected) << (contextInfo ? " " : "") << (contextInfo ? contextInfo : "") << ", got " << quote(token));
    }
}

Token Parser::parse(llvm::ArrayRef<Token::Kind> expected, const char* contextInfo) {
    if (!llvm::is_contained(expected, currentToken())) {
        unexpectedToken(currentToken(), expected, contextInfo);
    }
    return consumeToken();
}

void Parser::parseStmtTerminator(const char* contextInfo) {
    if (getCurrentLocation().line != lookAhead(-1).location.line) return;

    switch (currentToken()) {
    case Token::RightBrace:
    case Token::RightParen:
    case Token::In:
        return;
    case Token::Semicolon:
        consumeToken();
        return;
    default:
        unexpectedToken(currentToken(), {Token::Newline, Token::Semicolon}, contextInfo);
    }
}

/// argument-list ::= '(' ')' | '(' nonempty-argument-list ','? ')'
/// nonempty-argument-list ::= argument | nonempty-argument-list ',' argument
/// argument ::= (id '=')? expr
std::vector<NamedValue> Parser::parseArgumentList(bool allowEmpty) {
    parse(Token::LeftParen);
    llvm::SaveAndRestore allowBlockLambdaInArgs(allowBlockLambda, true);
    std::vector<NamedValue> args;

    if (currentToken() == Token::RightParen && allowEmpty) {
        consumeToken();
        return {};
    }

    while (true) {
        std::string name;
        Location location = Location();
        if (lookAhead(1) == Token::Assignment) {
            auto result = parse(Token::Identifier);
            name = result.getString().str();
            location = result.location;
            consumeToken();
        }
        auto value = parseExpr();
        if (!location.isValid()) location = value->location;
        args.push_back({std::move(name), value, location});

        if (parse({Token::Comma, Token::RightParen}) == Token::RightParen) return args;
        // Allow trailing comma (e.g. `foo(a, b,)`).
        if (currentToken() == Token::RightParen) {
            consumeToken();
            return args;
        }
    }
}

/// var-expr ::= id
VarExpr* Parser::parseVarExpr() {
    ASSERT(currentToken() == Token::Identifier);
    auto id = consumeToken();
    return makeAST<VarExpr>(id.getString().str(), id.location);
}

VarExpr* Parser::parseThis() {
    ASSERT(currentToken() == Token::This);
    auto expr = makeAST<VarExpr>("this", getCurrentLocation());
    consumeToken();
    return expr;
}

static std::string replaceEscapeChars(llvm::StringRef literalContent, Location literalStartLocation) {
    std::string result;
    result.reserve(literalContent.size());

    for (auto it = literalContent.begin(), end = literalContent.end(); it != end; ++it) {
        if (*it == '\\') {
            ++it;
            ASSERT(it != end);
            switch (*it) {
            case '0':
                result += '\0';
                break;
            case 'n':
                result += '\n';
                break;
            case 'r':
                result += '\r';
                break;
            case 't':
                result += '\t';
                break;
            case '"':
                result += '"';
                break;
            case '\'':
                result += '\'';
                break;
            case '\\':
                result += '\\';
                break;
            default:
                auto itColumn = literalStartLocation.column + 1 + (it - literalContent.begin());
                Location itLocation(literalStartLocation.file, literalStartLocation.line, itColumn);
                ERROR(itLocation, "unknown escape character '\\" << *it << "'");
            }
            continue;
        }
        result += *it;
    }
    return result;
}

StringLiteralExpr* Parser::parseStringLiteral() {
    ASSERT(currentToken() == Token::StringLiteral);
    auto content = replaceEscapeChars(currentToken().getString().drop_back().drop_front(), getCurrentLocation());
    auto expr = makeAST<StringLiteralExpr>(std::move(content), getCurrentLocation());
    consumeToken();
    return expr;
}

CharacterLiteralExpr* Parser::parseCharacterLiteral() {
    ASSERT(currentToken() == Token::CharacterLiteral);
    auto content = replaceEscapeChars(currentToken().getString().drop_back().drop_front(), getCurrentLocation());
    if (content.size() != 1) ERROR(getCurrentLocation(), "character literal must consist of a single UTF-8 byte");
    auto expr = makeAST<CharacterLiteralExpr>(content[0], getCurrentLocation());
    consumeToken();
    return expr;
}

IntLiteralExpr* Parser::parseIntLiteral() {
    ASSERT(currentToken() == Token::IntegerLiteral);
    auto expr = makeAST<IntLiteralExpr>(currentToken().getIntegerValue(), getCurrentLocation());
    consumeToken();
    return expr;
}

FloatLiteralExpr* Parser::parseFloatLiteral() {
    ASSERT(currentToken() == Token::FloatLiteral);
    auto expr = makeAST<FloatLiteralExpr>(currentToken().getFloatingPointValue(), getCurrentLocation());
    consumeToken();
    return expr;
}

BoolLiteralExpr* Parser::parseBoolLiteral() {
    BoolLiteralExpr* expr;
    switch (currentToken()) {
    case Token::True:
        expr = makeAST<BoolLiteralExpr>(true, getCurrentLocation());
        break;
    case Token::False:
        expr = makeAST<BoolLiteralExpr>(false, getCurrentLocation());
        break;
    default:
        llvm_unreachable("all cases handled");
    }
    consumeToken();
    return expr;
}

NullLiteralExpr* Parser::parseNullLiteral() {
    ASSERT(currentToken() == Token::Null);
    auto expr = makeAST<NullLiteralExpr>(getCurrentLocation());
    consumeToken();
    return expr;
}

UndefinedLiteralExpr* Parser::parseUndefinedLiteral() {
    ASSERT(currentToken() == Token::Undefined);
    auto expr = makeAST<UndefinedLiteralExpr>(getCurrentLocation());
    consumeToken();
    return expr;
}

/// array-literal ::= '[' expr-list ']'
ArrayLiteralExpr* Parser::parseArrayLiteral() {
    ASSERT(currentToken() == Token::LeftBracket);
    auto location = getCurrentLocation();
    consumeToken();
    auto elements = parseExprList();
    parse(Token::RightBracket);
    return makeAST<ArrayLiteralExpr>(std::move(elements), location);
}

/// tuple-literal ::= '(' tuple-literal-elements ')'
/// tuple-literal-elements ::= tuple-literal-element | tuple-literal-elements ',' tuple-literal-element
/// tuple-literal-element ::= (id '=')? expr
/// paren-expr ::= '(' expr ')'
Expr* Parser::parseTupleLiteralOrParenExpr() {
    ASSERT(currentToken() == Token::LeftParen);
    auto location = getCurrentLocation();
    auto elements = parseArgumentList(false);

    if (elements.size() == 1 && elements[0].name.empty()) {
        return elements[0].value;
    }

    for (auto& element : elements) {
        if (element.name.empty()) {
            if (auto* varExpr = llvm::dyn_cast<VarExpr>(element.value)) {
                element.name = varExpr->identifier;
            }
        }
    }

    return makeAST<TupleExpr>(std::move(elements), location);
}

/// non-empty-type-list ::= type | type ',' non-empty-type-list
std::vector<Type> Parser::parseNonEmptyTypeList() {
    std::vector<Type> types;

    while (true) {
        types.push_back(parseType());

        if (currentToken() == Token::Comma) {
            consumeToken();
        } else {
            if (currentToken() == Token::RightShift) {
                tokenBuffer[currentTokenIndex] = Token(Token::Greater, currentToken().location);
                tokenBuffer.insert(tokenBuffer.begin() + currentTokenIndex + 1, Token(Token::Greater, currentToken().location.nextColumn()));
            }
            return types;
        }
    }
}

/// generic-argument-list ::= '<' non-empty-type-list '>'
std::vector<Type> Parser::parseGenericArgumentList() {
    ASSERT(currentToken() == Token::Less);
    consumeToken();
    std::vector<Type> genericArgs = parseNonEmptyTypeList();
    parse(Token::Greater);
    return genericArgs;
}

Type Parser::parseArrayType(Type elementType) {
    ASSERT(currentToken() == Token::LeftBracket);
    consumeToken();

    switch (currentToken()) {
    case Token::IntegerLiteral: {
        auto arraySize = consumeToken().getIntegerValue().getExtValue();
        parse(Token::RightBracket);
        return ArrayType::get(elementType, arraySize, elementType.location);
    }
    case Token::RightBracket:
        consumeToken();
        return BasicType::get("ArrayRef", elementType, Mutability::Mutable, elementType.location);

    case Token::Star:
        consumeToken();
        parse(Token::RightBracket);
        return ArrayType::get(elementType, ArrayType::UnknownSize, elementType.location);

    default:
        ERROR(getCurrentLocation(), "non-literal array bounds not implemented yet");
    }
}

/// simple-type ::= id | id generic-argument-list | id '[' (int-literal | '*')? ']'
Type Parser::parseSimpleType(Mutability mutability) {
    auto identifier = parse(Token::Identifier);
    std::vector<Type> genericArgs;

    switch (currentToken()) {
    case Token::Less:
        genericArgs = parseGenericArgumentList();
        LLVM_FALLTHROUGH;
    default:
        return BasicType::get(identifier.getString(), std::move(genericArgs), mutability, identifier.location);
    case Token::LeftBracket:
        return parseArrayType(BasicType::get(identifier.getString(), {}, mutability, identifier.location));
    }
}

/// tuple-type ::= '(' tuple-type-elements ')'
/// tuple-type-elements ::= tuple-type-element | tuple-type-elements ',' tuple-type-element
/// tuple-type-element ::= type id?
Type Parser::parseTupleType() {
    ASSERT(currentToken() == Token::LeftParen);
    auto location = getCurrentLocation();
    consumeToken();
    std::vector<TupleElement> elements;

    while (currentToken() != Token::RightParen) {
        auto type = parseType();
        std::string name = currentToken() == Token::Identifier ? consumeToken().getString().str() : "";
        elements.push_back({std::move(name), type});
        if (currentToken() != Token::RightParen) parse(Token::Comma);
    }

    consumeToken();
    return TupleType::get(std::move(elements), Mutability::Mutable, location);
}

/// function-type ::= type '(' param-types ')'
/// param-types ::= '' | non-empty-param-types
/// non-empty-param-types ::= type | type ',' non-empty-param-types
Type Parser::parseFunctionType(Type returnType) {
    parse(Token::LeftParen);
    std::vector<Type> paramTypes;

    while (currentToken() != Token::RightParen) {
        paramTypes.emplace_back(parseType());
        if (currentToken() != Token::RightParen) parse(Token::Comma);
    }

    consumeToken();
    return FunctionType::get(returnType, std::move(paramTypes), false, Mutability::Mutable, returnType.location);
}

/// type ::= simple-type | 'const' simple-type | type '*' | type '?' | function-type | tuple-type
Type Parser::parseType() {
    Type type;
    auto location = getCurrentLocation();

    switch (currentToken()) {
    case Token::Identifier:
        type = parseSimpleType(Mutability::Mutable);
        break;
    case Token::Const:
        consumeToken();
        type = parseSimpleType(Mutability::Const);
        break;
    case Token::LeftParen:
        type = parseTupleType();
        break;
    default:
        ERROR(getCurrentLocation(), "expected type, got " << quote(currentToken()));
    }

    while (true) {
        switch (currentToken()) {
        case Token::Star:
            type = PointerType::get(type, Mutability::Mutable, location);
            consumeToken();
            break;
        case Token::QuestionMark:
            type = OptionalType::get(type, Mutability::Mutable, location);
            consumeToken();
            break;
        case Token::QuestionQuestion:
            // A lexed `??` in type position is two nested optionals (e.g. `int??`).
            type = OptionalType::get(OptionalType::get(type, Mutability::Mutable, location), Mutability::Mutable, location);
            consumeToken();
            break;
        case Token::LeftParen:
            type = parseFunctionType(type);
            break;
        case Token::LeftBracket:
            type = parseArrayType(type);
            break;
        case Token::And:
            ERROR(getCurrentLocation(), "C* doesn't have C++-style references; use pointers ('*') instead, they are non-null by default");
        default:
            return type.withLocation(location);
        }
    }
}

/// sizeof-expr ::= 'sizeof' '(' (type | variable) ')'
SizeofExpr* Parser::parseSizeofExpr() {
    ASSERT(currentToken() == Token::Sizeof);
    auto location = getCurrentLocation();
    consumeToken();
    parse(Token::LeftParen);
    auto type = parseType();
    parse(Token::RightParen);
    return makeAST<SizeofExpr>(type, location);
}

/// member-expr ::= expr '.' id
MemberExpr* Parser::parseMemberExpr(Expr* lhs) {
    auto location = getCurrentLocation();
    auto member = parse(Token::Identifier);
    return makeAST<MemberExpr>(lhs, member.getString().str(), location);
}

/// index-expr ::= expr '[' expr ']'
/// index-assignment-expr ::= index-expr '=' expr
Expr* Parser::parseIndexExprOrIndexAssignmentExpr(Expr* base) {
    ASSERT(currentToken() == Token::LeftBracket);
    auto location = getCurrentLocation();
    consumeToken();
    llvm::SaveAndRestore allowBlockLambdaInIndex(allowBlockLambda, true);
    auto index = parseExpr();
    parse(Token::RightBracket);

    if (currentToken() == Token::Assignment) {
        consumeToken();
        return makeAST<IndexAssignmentExpr>(base, index, parseExpr(), location);
    }

    return makeAST<IndexExpr>(base, index, location);
}

/// unwrap-expr ::= expr '!'
UnwrapExpr* Parser::parseUnwrapExpr(Expr* operand) {
    ASSERT(currentToken() == Token::Not);
    auto location = getCurrentLocation();
    consumeToken();
    return makeAST<UnwrapExpr>(operand, location);
}

/// call-expr ::= expr generic-argument-list? argument-list
CallExpr* Parser::parseCallExpr(Expr* callee) {
    std::vector<Type> genericArgs;
    if (currentToken() == Token::Less) {
        genericArgs = parseGenericArgumentList();
    }
    auto location = getCurrentLocation();
    auto args = parseArgumentList(true);
    return makeAST<CallExpr>(callee, std::move(args), std::move(genericArgs), location);
}

/// lambda-expr ::= param-list '=>' expr | param-list ('=>')? block | id '=>' expr | id '=>' block
LambdaExpr* Parser::parseLambdaExpr() {
    ASSERT(currentToken().is({Token::LeftParen, Token::Identifier}));
    auto location = getCurrentLocation();
    std::vector<ParamDecl> params;

    if (currentToken() == Token::Identifier) {
        auto paramName = consumeToken();
        params.push_back(ParamDecl(Type(), paramName.getString().str(), false, paramName.location));
    } else {
        params = parseParamList(nullptr, false);
        for (auto& param : params) {
            if (param.defaultValue) {
                ERROR(param.getLocation(), "lambda parameters cannot have default values");
            }
        }
    }

    auto lambda = makeAST<LambdaExpr>(std::move(params), currentModule, location);

    if (currentToken() == Token::LeftBrace) {
        lambda->functionDecl->body = parseBlock(lambda->functionDecl);
    } else {
        parse(Token::FatArrow);
        if (currentToken() == Token::LeftBrace) {
            lambda->functionDecl->body = parseBlock(lambda->functionDecl);
        } else {
            auto expr = parseExpr();
            lambda->functionDecl->body = {makeAST<ReturnStmt>(expr, expr->location)};
        }
    }

    return lambda;
}

// TODO: change to: 'if' expr 'then' expr 'else' expr
/// if-expr ::= expr '?' expr ':' expr
IfExpr* Parser::parseIfExpr(Expr* condition) {
    ASSERT(currentToken() == Token::QuestionMark);
    auto location = getCurrentLocation();
    consumeToken();
    auto thenExpr = parseExpr();
    parse(Token::Colon);
    auto elseExpr = parseExpr();
    return makeAST<IfExpr>(condition, thenExpr, elseExpr, location);
}

/// if-then-else-expr ::= 'if' expr 'then' expr 'else' expr
IfExpr* Parser::parseIfThenElseExpr() {
    ASSERT(currentToken() == Token::If);
    auto location = consumeToken().location;
    Expr* condition;
    {
        llvm::SaveAndRestore disallowBlockLambda(allowBlockLambda, false);
        condition = parseExpr();
    }
    parse(Token::Then);
    auto thenExpr = parseExpr();
    parse(Token::Else);
    auto elseExpr = parseExpr();
    return makeAST<IfExpr>(condition, thenExpr, elseExpr, location);
}

bool Parser::shouldParseVarStmt() {
    if (currentToken().is({Token::Var, Token::Const})) return true;
    if (!currentToken().is({Token::Identifier, Token::LeftParen})) return false;
    if (lookAhead(1).is(Token::Dot) || isCompoundAssignmentOperator(lookAhead(1))) return false;
    int offset = 2;

    while (true) {
        if (lookAhead(offset).is(Token::Assignment)) {
            if (lookAhead(offset - 1).is(Token::Identifier)) {
                // Walk back over any ', name' pairs of a multi-variable declaration.
                int back = offset - 2;
                while (lookAhead(back).is(Token::Comma) && lookAhead(back - 1).is(Token::Identifier)) {
                    back -= 2;
                }
                if (lookAhead(back).is({Token::Identifier, Token::RightBracket, Token::QuestionMark, Token::QuestionQuestion, Token::Greater})) {
                    return true;
                }
                if (lookAhead(back).is(Token::Star)) {
                    if (lookAhead(back - 1).is(Token::Semicolon) || lookAhead(back).location.line != lookAhead(back - 1).location.line) {
                        return false;
                    }
                    return true;
                }
            }
            return false;
        } else if (lookAhead(offset).is(Token::Semicolon) || lookAhead(offset).location.line != lookAhead(offset - 1).location.line) {
            if (lookAhead(offset - 1).is(Token::Identifier)) {
                // Walk back over any ', name' pairs of a multi-variable declaration.
                int back = offset - 2;
                while (lookAhead(back).is(Token::Comma) && lookAhead(back - 1).is(Token::Identifier)) {
                    back -= 2;
                }
                if (lookAhead(back).is({Token::Identifier, Token::RightBracket, Token::QuestionMark, Token::QuestionQuestion, Token::Greater, Token::Star})) {
                    return true;
                }
            }
            return false;
        } else {
            offset++;
        }
    }
}

bool Parser::shouldParseGenericArgumentList() {
    // Temporary hack: use spacing to determine whether to parse a generic argument list
    // of a less-than binary expression. Zero spaces on either side of '<' will cause it
    // to be interpreted as a generic argument list, for now.
    return lookAhead(0).location.column + int(lookAhead(0).getString().size()) == lookAhead(1).location.column
        || lookAhead(1).location.column + 1 == lookAhead(2).location.column;
}

bool Parser::shouldParseGenericArgumentListAfterMember() {
    ASSERT(currentToken() == Token::Less);
    if (!(lookAhead(-1).location.column + int(lookAhead(-1).getString().size()) == lookAhead(0).location.column
          || lookAhead(0).location.column + 1 == lookAhead(1).location.column)) {
        return false;
    }
    // A generic argument list is always followed by a call, so only treat '<' as one if the
    // matching '>' is followed by '('; otherwise it's a less-than comparison (e.g. 'box.value<2').
    int depth = 0;
    for (int offset = 0;; ++offset) {
        switch (lookAhead(offset)) {
        case Token::Less:
            ++depth;
            break;
        case Token::Greater:
            if (--depth == 0) return lookAhead(offset + 1) == Token::LeftParen;
            break;
        case Token::RightShift: // Closes two levels, mirroring the '>>' split in parseNonEmptyTypeList.
            depth -= 2;
            if (depth <= 0) return lookAhead(offset + 1) == Token::LeftParen;
            break;
        case Token::Identifier:
        case Token::Const:
        case Token::Comma:
        case Token::Star:
        case Token::QuestionMark:
        case Token::QuestionQuestion:
        case Token::LeftBracket:
        case Token::RightBracket:
        case Token::IntegerLiteral:
        case Token::LeftParen:
        case Token::RightParen:
            break;
        default:
            return false;
        }
    }
}

/// Returns true if a fat-arrow token immediately follows the current set of parentheses,
/// or a left-brace does and block-bodied lambdas are allowed in this position.
/// Block-bodied lambdas are disallowed while parsing loop/if/switch conditions,
/// where '(expr) {' is a parenthesized condition followed by the body, not a lambda.
bool Parser::lambdaAfterParentheses() {
    ASSERT(currentToken() == Token::LeftParen);
    int offset = 1;

    for (int parenDepth = 1; parenDepth > 0; ++offset) {
        switch (lookAhead(offset)) {
        case Token::LeftParen:
            ++parenDepth;
            break;
        case Token::RightParen:
            --parenDepth;
            break;
        default:
            break;
        }
    }

    if (lookAhead(offset) == Token::FatArrow) return true;
    return allowBlockLambda && lookAhead(offset) == Token::LeftBrace;
}

/// postfix-expr ::= postfix-expr postfix-op | call-expr | variable-expr | string-literal |
///                  int-literal | float-literal | bool-literal | null-literal |
///                  paren-expr | array-literal | tuple-literal | index-expr | index-assignment-expr
///                  member-expr | unwrap-expr | lambda-expr | sizeof-expr
Expr* Parser::parsePostfixExpr() {
    Expr* expr;

    switch (currentToken()) {
    case Token::Identifier:
        switch (lookAhead(1)) {
        case Token::LeftParen:
            expr = parseCallExpr(parseVarExpr());
            break;
        case Token::FatArrow:
            expr = parseLambdaExpr();
            break;
        case Token::Less:
            if (shouldParseGenericArgumentList()) {
                expr = parseCallExpr(parseVarExpr());
                break;
            }
            LLVM_FALLTHROUGH;
        default:
            expr = parseVarExpr();
            break;
        }
        break;
    case Token::StringLiteral:
        expr = parseStringLiteral();
        break;
    case Token::CharacterLiteral:
        expr = parseCharacterLiteral();
        break;
    case Token::IntegerLiteral:
        expr = parseIntLiteral();
        break;
    case Token::FloatLiteral:
        expr = parseFloatLiteral();
        break;
    case Token::True:
    case Token::False:
        expr = parseBoolLiteral();
        break;
    case Token::Null:
        expr = parseNullLiteral();
        break;
    case Token::This:
        expr = parseThis();
        break;
    case Token::LeftParen:
        if (lambdaAfterParentheses()) {
            expr = parseLambdaExpr();
        } else {
            expr = parseTupleLiteralOrParenExpr();
        }
        break;
    case Token::LeftBracket:
        expr = parseArrayLiteral();
        break;
    case Token::Sizeof:
        expr = parseSizeofExpr();
        break;
    case Token::Undefined:
        expr = parseUndefinedLiteral();
        break;
    case Token::Switch:
        expr = parseSwitchExpr();
        break;
    case Token::If:
        expr = parseIfThenElseExpr();
        break;
    default:
        unexpectedToken(currentToken());
        break;
    }

    while (true) {
        switch (currentToken()) {
        case Token::LeftBracket:
            expr = parseIndexExprOrIndexAssignmentExpr(expr);
            break;
        case Token::LeftParen:
            expr = parseCallExpr(expr);
            break;
        case Token::Dot:
            consumeToken();
            expr = parseMemberExpr(expr);
            if (currentToken() == Token::Less && shouldParseGenericArgumentListAfterMember()) {
                expr = parseCallExpr(expr);
            }
            break;
        case Token::Increment:
        case Token::Decrement:
            expr = parseIncrementOrDecrementExpr(expr);
            break;
        case Token::Not:
            expr = parseUnwrapExpr(expr);
            break;
        default:
            return expr;
        }
    }
}

/// prefix-expr ::= prefix-operator (prefix-expr | postfix-expr)
UnaryExpr* Parser::parsePrefixExpr() {
    ASSERT(isUnaryOperator(currentToken()));
    auto op = consumeToken();
    return makeAST<UnaryExpr>(op.kind, parsePreOrPostfixExpr(), op.location);
}

Expr* Parser::parsePreOrPostfixExpr() {
    return isUnaryOperator(currentToken()) ? parsePrefixExpr() : parsePostfixExpr();
}

/// inc-expr ::= expr '++'
/// dec-expr ::= expr '--'
UnaryExpr* Parser::parseIncrementOrDecrementExpr(Expr* operand) {
    auto op = parse({Token::Increment, Token::Decrement});
    return makeAST<UnaryExpr>(op.kind, operand, op.location);
}

/// binary-expr ::= expr op expr
Expr* Parser::parseBinaryExpr(int minPrecedence) {
    auto lhs = parsePreOrPostfixExpr();

    while (((isBinaryOperator(currentToken()) || currentToken() == Token::QuestionMark) && getPrecedence(currentToken()) >= minPrecedence)) {
        if (currentToken() == Token::QuestionMark) {
            lhs = parseIfExpr(lhs);
            continue;
        }

        auto lhsEndLine = lookAhead(-1).location.line;
        auto backtrackLocation = currentTokenIndex;
        auto op = consumeToken();
        // Assignments associate to the right so `a = b = 1` parses as `a = (b = 1)`.
        // `??` does too so `a ?? b ?? c` parses as `a ?? (b ?? c)`.
        auto rhs = parseBinaryExpr(isAssignmentOperator(op) || op == Token::QuestionQuestion ? getPrecedence(op) : getPrecedence(op) + 1);

        if (isAssignmentOperator(currentToken()) && op.location.line != lhsEndLine) {
            // The operator continues on a later line only to hit an assignment (e.g. a
            // dereference statement after another statement); backtrack so it starts a new statement.
            currentTokenIndex = backtrackLocation;
            break;
        }

        lhs = makeAST<BinaryExpr>(op.kind, lhs, rhs, op.location);
    }

    return lhs;
}

/// expr ::= prefix-expr | postfix-expr | binary-expr | if-expr
Expr* Parser::parseExpr() {
    return parseBinaryExpr(0);
}

Expr* Parser::parseExprOrVarDecl(Decl* parent) {
    if (!shouldParseVarStmt()) {
        return parseExpr();
    } else {
        return makeAST<VarDeclExpr>(parseVarDecl(parent, AccessLevel::None, false));
    }
}

/// expr-list ::= '' | nonempty-expr-list ','?
/// nonempty-expr-list ::= expr | expr ',' nonempty-expr-list
std::vector<Expr*> Parser::parseExprList() {
    llvm::SaveAndRestore allowBlockLambdaInList(allowBlockLambda, true);
    std::vector<Expr*> exprs;

    switch (currentToken()) {
    case Token::Semicolon:
    case Token::RightBrace:
    case Token::RightBracket:
        return exprs;
    default:
        break;
    }

    while (true) {
        exprs.emplace_back(parseExpr());
        if (currentToken() != Token::Comma) return exprs;
        consumeToken();
        // Allow trailing comma (e.g. `[1, 2,]`).
        if (currentToken() == Token::RightBracket) return exprs;
    }
}

/// return-stmt ::= 'return' expr ('\n' | ';')
ReturnStmt* Parser::parseReturnStmt() {
    ASSERT(currentToken() == Token::Return);
    auto location = getCurrentLocation();
    consumeToken();
    auto returnValue = currentToken().is({Token::Semicolon, Token::RightBrace}) ? nullptr : parseExpr();
    parseStmtTerminator();
    return makeAST<ReturnStmt>(returnValue, location);
}

/// var-decl ::= type-specifier id '=' initializer ('\n' | ';')
/// type-specifier ::= 'const' | 'const' type | type | 'var'
/// initializer ::= expr | 'undefined'
VarDecl* Parser::parseVarDecl(Decl* parent, AccessLevel accessLevel, bool requireTerminator) {
    Type type;
    auto mutability = Mutability::Mutable;

    if (currentToken() == Token::Const) {
        consumeToken();
        mutability = Mutability::Const;
    }

    if (currentToken() == Token::Var) {
        consumeToken();
    } else if (lookAhead(1) != Token::Assignment) {
        type = parseType();
    }

    auto name = parse(Token::Identifier);
    return parseVarDeclAfterName(parent, accessLevel, type.withMutability(mutability), name.getString(), name.location, requireTerminator);
}

VarDecl* Parser::parseVarDeclAfterName(Decl* parent, AccessLevel accessLevel, Type type, llvm::StringRef name, Location nameLocation, bool requireTerminator) {
    Expr* initializer = nullptr;

    if (currentToken() == Token::Assignment) {
        consumeToken();
        initializer = parseExpr();
    } else if (currentToken() == Token::Semicolon || currentToken() == Token::Comma || currentToken().location.line != lookAhead(-1).location.line) {
        WARN(nameLocation, "missing initializer");
    }

    if (requireTerminator) parseStmtTerminator();
    return makeAST<VarDecl>(type, name.str(), initializer, parent, accessLevel, *currentModule, nameLocation);
}

/// var-stmt ::= var-decl (',' id ('=' initializer)?)*
VarStmt* Parser::parseVarStmt(Decl* parent) {
    std::vector<VarDecl*> decls;
    decls.push_back(parseVarDecl(parent, AccessLevel::None, false));
    while (currentToken() == Token::Comma) {
        consumeToken();
        auto name = parse(Token::Identifier);
        decls.push_back(parseVarDeclAfterName(parent, AccessLevel::None, decls.front()->type, name.getString(), name.location, false));
    }
    parseStmtTerminator();
    return makeAST<VarStmt>(std::move(decls));
}

/// expr-stmt ::= expr ('\n' | ';')
ExprStmt* Parser::parseExprStmt() {
    auto stmt = makeAST<ExprStmt>(parseExpr());
    parseStmtTerminator();
    return stmt;
}

/// block ::= '{' stmt* '}'
std::vector<Stmt*> Parser::parseBlock(Decl* parent) {
    parse(Token::LeftBrace);
    std::vector<Stmt*> stmts;
    while (currentToken() != Token::RightBrace) {
        stmts.push_back(parseStmt(parent));
    }
    consumeToken();
    return stmts;
}

/// block-or-stmt ::= block | stmt
std::vector<Stmt*> Parser::parseBlockOrStmt(Decl* parent) {
    if (currentToken() == Token::LeftBrace) {
        return parseBlock(parent);
    } else {
        return {parseStmt(parent)};
    }
}

/// defer-stmt ::= 'defer' expr ('\n' | ';')
DeferStmt* Parser::parseDeferStmt() {
    ASSERT(currentToken() == Token::Defer);
    consumeToken();
    auto stmt = makeAST<DeferStmt>(parseExpr());
    parseStmtTerminator();
    return stmt;
}

/// if-stmt ::= 'if' (expr | var-decl) block-or-stmt ('else' block-or-stmt)?
Stmt* Parser::parseIfStmt(Decl* parent) {
    ASSERT(currentToken() == Token::If);
    auto location = consumeToken().location;
    bool parens = currentToken() == Token::LeftParen;
    if (parens) consumeToken();
    Expr* condition;
    {
        llvm::SaveAndRestore disallowBlockLambda(allowBlockLambda, false);
        condition = parseExprOrVarDecl(parent);
        if (parens) parse(Token::RightParen);
    }
    if (currentToken() == Token::Then) {
        if (condition->isVarDeclExpr()) {
            ERROR(condition->location, "variable declaration conditions are not supported in if expressions");
        }
        consumeToken();
        auto thenExpr = parseExpr();
        parse(Token::Else);
        auto elseExpr = parseExpr();
        auto stmt = makeAST<ExprStmt>(makeAST<IfExpr>(condition, thenExpr, elseExpr, location));
        parseStmtTerminator();
        return stmt;
    }
    bool thenIsBlock = currentToken() == Token::LeftBrace;
    auto thenStmts = parseBlockOrStmt(parent);
    std::vector<Stmt*> elseStmts;
    Location elseLocation;
    if (currentToken() == Token::Else) {
        elseLocation = consumeToken().location;
        elseStmts = parseBlockOrStmt(parent);
    } else if (!thenIsBlock && thenStmts.size() == 1) {
        if (auto* innerIf = llvm::dyn_cast<IfStmt>(thenStmts.front()); innerIf && !innerIf->elseBody.empty()) {
            WARN(innerIf->elseLocation, "add explicit braces to avoid dangling else");
        }
    }
    return makeAST<IfStmt>(condition, std::move(thenStmts), std::move(elseStmts), elseLocation);
}

/// while-stmt ::= 'while' (expr | var-decl) block-or-stmt
WhileStmt* Parser::parseWhileStmt(Decl* parent) {
    ASSERT(currentToken() == Token::While);
    auto location = consumeToken().location;
    bool parens = currentToken() == Token::LeftParen;
    if (parens) consumeToken();
    Expr* condition;
    {
        llvm::SaveAndRestore disallowBlockLambda(allowBlockLambda, false);
        condition = parseExprOrVarDecl(parent);
        if (parens) parse(Token::RightParen);
    }
    auto body = parseBlockOrStmt(parent);
    return makeAST<WhileStmt>(condition, std::move(body), location);
}

/// do-while-stmt ::= 'do' block-or-stmt 'while' expr ('\n' | ';')
DoWhileStmt* Parser::parseDoWhileStmt(Decl* parent) {
    ASSERT(currentToken() == Token::Do);
    auto location = consumeToken().location;
    auto body = parseBlockOrStmt(parent);
    parse(Token::While);
    bool parens = currentToken() == Token::LeftParen;
    if (parens) consumeToken();
    Expr* condition;
    {
        llvm::SaveAndRestore disallowBlockLambda(allowBlockLambda, false);
        condition = parseExpr();
        if (parens) parse(Token::RightParen);
    }
    parseStmtTerminator();
    return makeAST<DoWhileStmt>(condition, std::move(body), location);
}

/// for-stmt ::= 'for' for-header block-or-stmt
/// for-header ::= var-decl ';' expr? ';' expr? |
///            '(' var-decl ';' expr? ';' expr? ')'
/// foreach-stmt ::= 'for' foreach-header block-or-stmt
/// foreach-header ::= id 'in' expr
Stmt* Parser::parseForOrForEachStmt(Decl* parent) {
    ASSERT(currentToken() == Token::For);
    auto location = consumeToken().location;
    bool parens = currentToken() == Token::LeftParen;
    if (parens) consumeToken();

    if (currentToken() == Token::Identifier && lookAhead(1) == Token::In) {
        auto name = parse(Token::Identifier);
        if (parens) {
            ERROR(location, "for-each loop header must not be parenthesized, write 'for " << name.getString() << " in ...'");
        }
        auto* varDecl = makeAST<VarDecl>(Type(), name.getString().str(), nullptr, parent, AccessLevel::None, *currentModule, name.location);
        parse(Token::In);
        Expr* range;
        {
            llvm::SaveAndRestore disallowBlockLambda(allowBlockLambda, false);
            range = parseExpr();
        }
        if (parens) parse(Token::RightParen);
        auto body = parseBlockOrStmt(parent);
        return makeAST<ForEachStmt>(varDecl, range, std::move(body), location);
    }

    auto varStmt = currentToken() == Token::Semicolon ? (consumeToken(), nullptr) : parseVarStmt(parent);

    if (!varStmt || varStmt->decls.front()->initializer) {
        // C-style for loop. The condition and increment expressions may be omitted.
        Expr* condition = nullptr;
        if (currentToken() == Token::Semicolon) {
            consumeToken();
        } else {
            llvm::SaveAndRestore disallowBlockLambda(allowBlockLambda, false);
            condition = parseExpr();
            parse(Token::Semicolon);
        }
        Expr* increment = nullptr;
        if (currentToken() != Token::RightParen && currentToken() != Token::LeftBrace) {
            llvm::SaveAndRestore disallowBlockLambda(allowBlockLambda, false);
            increment = parseExpr();
        }
        if (parens) parse(Token::RightParen);
        auto body = parseBlockOrStmt(parent);
        return makeAST<ForStmt>(varStmt, condition, increment, std::move(body), location);
    } else if (currentToken() == Token::In) {
        ERROR(varStmt->decls.front()->getLocation(),
              "for-each loop variable must be a bare identifier, write 'for " << varStmt->decls.front()->getName() << " in ...'");
    } else {
        parse(Token::In);
        llvm_unreachable("parse() throws on mismatch");
    }
}

/// case-header ::= 'case' expr identifier? ':'
std::pair<Expr*, VarDecl*> Parser::parseSwitchCaseHeader(Decl* parent) {
    ASSERT(currentToken() == Token::Case);
    consumeToken();
    auto value = parseExpr();

    VarDecl* associatedValue = nullptr;
    if (currentToken() == Token::Identifier) {
        auto name = parse(Token::Identifier);
        associatedValue = makeAST<VarDecl>(Type(), name.getString().str(), nullptr, parent, AccessLevel::None, *currentModule, name.location);
    }

    parse(Token::Colon);
    return {value, associatedValue};
}

/// switch-stmt ::= 'switch' expr '{' cases default-case? '}'
/// cases ::= case | case cases
/// case ::= case-header stmt*
/// default-case ::= 'default' ':' stmt*
SwitchStmt* Parser::parseSwitchStmt(Decl* parent) {
    ASSERT(currentToken() == Token::Switch);
    consumeToken();
    Expr* condition;
    {
        llvm::SaveAndRestore disallowBlockLambda(allowBlockLambda, false);
        condition = parseExpr();
    }
    parse(Token::LeftBrace);
    std::vector<SwitchCase> cases;
    std::vector<Stmt*> defaultStmts;
    bool defaultSeen = false;

    while (true) {
        if (currentToken() == Token::Case) {
            auto [value, associatedValue] = parseSwitchCaseHeader(parent);
            auto stmts = parseStmtsUntilOneOf(Token::Case, Token::Default, Token::RightBrace, parent);
            cases.push_back(SwitchCase(value, associatedValue, std::move(stmts)));
        } else if (currentToken() == Token::Default) {
            if (defaultSeen) {
                ERROR(getCurrentLocation(), "switch-statement may only contain one 'default' case");
            }
            consumeToken();
            parse(Token::Colon);
            defaultStmts = parseStmtsUntilOneOf(Token::Case, Token::Default, Token::RightBrace, parent);
            defaultSeen = true;
        } else {
            ERROR(getCurrentLocation(), "expected 'case' or 'default'");
        }

        if (currentToken() == Token::RightBrace) break;
    }

    consumeToken();
    return makeAST<SwitchStmt>(condition, std::move(cases), std::move(defaultStmts));
}

/// switch-expr ::= 'switch' expr '{' arms default-arm? '}'
/// arms ::= arm | arm arms
/// arm ::= case-header expr ','?
/// default-arm ::= 'default' ':' expr ','?
SwitchExpr* Parser::parseSwitchExpr() {
    ASSERT(currentToken() == Token::Switch);
    auto location = getCurrentLocation();
    consumeToken();
    auto condition = parseExpr();
    parse(Token::LeftBrace);
    std::vector<SwitchExprArm> arms;
    Expr* defaultExpr = nullptr;
    bool defaultSeen = false;

    while (true) {
        if (currentToken() == Token::Case) {
            // Expression parsing has no enclosing declaration; the binding's parent is set during typechecking.
            auto [value, associatedValue] = parseSwitchCaseHeader(nullptr);
            if (currentToken().is({Token::Case, Token::Default, Token::RightBrace})) {
                ERROR(getCurrentLocation(), "switch expression case must have a value");
            }
            auto armExpr = parseExpr();
            if (currentToken() == Token::Comma) consumeToken();
            parseStmtTerminator("in switch expression case");
            arms.push_back(SwitchExprArm(value, associatedValue, armExpr));
        } else if (currentToken() == Token::Default) {
            if (defaultSeen) {
                ERROR(getCurrentLocation(), "switch-expression may only contain one 'default' case");
            }
            consumeToken();
            parse(Token::Colon);
            if (currentToken().is({Token::Case, Token::Default, Token::RightBrace})) {
                ERROR(getCurrentLocation(), "switch expression default case must have a value");
            }
            defaultExpr = parseExpr();
            if (currentToken() == Token::Comma) consumeToken();
            parseStmtTerminator("in switch expression default case");
            defaultSeen = true;
        } else {
            ERROR(getCurrentLocation(), "expected 'case' or 'default'");
        }

        if (currentToken() == Token::RightBrace) break;
    }

    consumeToken();
    return makeAST<SwitchExpr>(condition, std::move(arms), defaultExpr, location);
}

/// break-stmt ::= 'break' ('\n' | ';')
BreakStmt* Parser::parseBreakStmt() {
    auto location = getCurrentLocation();
    consumeToken();
    parseStmtTerminator();
    return makeAST<BreakStmt>(location);
}

/// continue-stmt ::= 'continue' ('\n' | ';')
ContinueStmt* Parser::parseContinueStmt() {
    auto location = getCurrentLocation();
    consumeToken();
    parseStmtTerminator();
    return makeAST<ContinueStmt>(location);
}

/// stmt ::= var-stmt | return-stmt | expr-stmt | defer-stmt | if-stmt | switch-stmt |
///          while-stmt | do-while-stmt | for-stmt | foreach-stmt | break-stmt | continue-stmt | block
Stmt* Parser::parseStmt(Decl* parent) {
    switch (currentToken()) {
    case Token::LeftBrace:
        return makeAST<CompoundStmt>(parseBlock(parent));
    case Token::Return:
        return parseReturnStmt();
    case Token::Defer:
        return parseDeferStmt();
    case Token::If:
        return parseIfStmt(parent);
    case Token::While:
        return parseWhileStmt(parent);
    case Token::Do:
        return parseDoWhileStmt(parent);
    case Token::For:
        return parseForOrForEachStmt(parent);
    case Token::Switch:
        return parseSwitchStmt(parent);
    case Token::Break:
        return parseBreakStmt();
    case Token::Continue:
        return parseContinueStmt();
    case Token::Identifier:
        if (currentToken().getString() == "_") {
            consumeToken();
            parse(Token::Assignment);
            auto* stmt = parseExprStmt();
            stmt->discardsResult = true;
            return stmt;
        } else if (lookAhead(1).is(Token::Assignment)) {
            return parseExprStmt();
        }
        LLVM_FALLTHROUGH;
    default:
        if (shouldParseVarStmt()) {
            return parseVarStmt(parent);
        }
        return parseExprStmt();
    }
}

std::vector<Stmt*> Parser::parseStmtsUntilOneOf(Token::Kind end1, Token::Kind end2, Token::Kind end3, Decl* parent) {
    std::vector<Stmt*> stmts;
    while (currentToken() != end1 && currentToken() != end2 && currentToken() != end3) {
        stmts.emplace_back(parseStmt(parent));
    }
    return stmts;
}

/// param-decl ::= 'public'? type? id | 'public'? type '...' id | 'public'? type
ParamDecl Parser::parseParam(bool requireType) {
    bool isPublic = currentToken() == Token::Public;
    if (isPublic) consumeToken();

    Type type;
    if (requireType || !lookAhead(1).is({Token::Comma, Token::RightParen})) {
        type = parseType();
    }

    bool isPack = false;
    if (type && currentToken() == Token::DotDotDot) {
        consumeToken();
        isPack = true;
    }

    // A typed parameter may omit its name, e.g. `void f(int)`.
    if (type && currentToken().is({Token::Comma, Token::RightParen})) {
        ParamDecl param(type, "", isPublic, getCurrentLocation());
        param.isPack = isPack;
        return param;
    }

    auto name = parse(Token::Identifier);
    ParamDecl param(type, name.getString().str(), isPublic, name.location);
    param.isPack = isPack;
    if (currentToken() == Token::Assignment) {
        if (isPack) {
            ERROR(name.location, "variadic parameter cannot have a default value");
        }
        consumeToken();
        param.defaultValue = parseExpr();
    }
    return param;
}

/// param-list ::= '(' params ')'
/// params ::= '' | non-empty-params
/// non-empty-params ::= param-decl | param-decl ',' non-empty-params
std::vector<ParamDecl> Parser::parseParamList(bool* isVariadic, bool requireTypes) {
    parse(Token::LeftParen);
    std::vector<ParamDecl> params;
    while (currentToken() != Token::RightParen) {
        if (isVariadic && currentToken() == Token::DotDotDot) {
            consumeToken();
            *isVariadic = true;
            break;
        }
        params.emplace_back(parseParam(requireTypes));
        if (params.back().isPack && currentToken() != Token::RightParen) {
            ERROR(params.back().getLocation(), "variadic parameter must be the last parameter");
        }
        if (currentToken() != Token::RightParen) parse(Token::Comma);
    }
    parse(Token::RightParen);
    for (size_t i = 1; i < params.size(); ++i) {
        if (params[i - 1].defaultValue && !params[i].defaultValue && !params[i].isPack) {
            if (params[i].getName().empty()) {
                ERROR(params[i].getLocation(), "unnamed parameter follows a parameter with a default value");
            }
            ERROR(params[i].getLocation(), "parameter '" << params[i].getName() << "' without a default value follows a parameter with a default value");
        }
    }
    return params;
}

void Parser::parseGenericParamList(std::vector<GenericParamDecl>& genericParams) {
    parse(Token::Less);
    while (true) {
        auto genericParamName = parse(Token::Identifier);
        genericParams.emplace_back(genericParamName.getString().str(), genericParamName.location);

        if (currentToken() == Token::Colon) {
            consumeToken();
            genericParams.back().constraints = {parseType()};
        }

        if (currentToken() == Token::Greater) break;
        parse(Token::Comma);
    }
    parse(Token::Greater);
}

llvm::StringRef Parser::parseFunctionName(TypeDecl* receiverTypeDecl) {
    auto name = parse(Token::Identifier);

    if (name.getString() == "operator") {
        auto op = consumeToken();
        if (op == Token::LeftBracket) {
            parse(Token::RightBracket);
            if (currentToken() == Token::Assignment) {
                consumeToken();
                return "[]=";
            } else {
                return "[]";
            }
        } else {
            if (!isOverloadable(op)) {
                unexpectedToken(op, {}, "as function name");
            }
            if (receiverTypeDecl) {
                ERROR(name.location, "operator functions other than 'operator[]' must be non-member functions");
            }
            return toString(op);
        }
    } else {
        return name.getString();
    }
}

/// function-proto ::= type id param-list
FunctionDecl* Parser::parseFunctionProto(bool isExtern, TypeDecl* receiverTypeDecl, AccessLevel accessLevel, std::vector<GenericParamDecl>* genericParams,
                                         Type returnType, llvm::StringRef name, Location location) {
    if (currentToken() == Token::Less) {
        parseGenericParamList(*genericParams);
    }

    bool isVariadic = false;
    auto params = parseParamList(isExtern ? &isVariadic : nullptr);
    if (isExtern) {
        for (const ParamDecl& param : params) {
            if (param.isPack)
                ERROR(param.getLocation(), "variadic parameter '" << param.type << "... " << param.getName()
                                                                  << "' is not allowed in extern functions, use a bare '...' (C-style varargs) instead");
        }
    }
    FunctionProto proto(name.str(), std::move(params), returnType, isVariadic, isExtern);

    if (receiverTypeDecl) {
        return makeAST<MethodDecl>(std::move(proto), *receiverTypeDecl, std::vector<Type>(), accessLevel, location);
    } else {
        return makeAST<FunctionDecl>(std::move(proto), std::vector<Type>(), accessLevel, *currentModule, location);
    }
}

/// function-template-proto ::= type id template-param-list param-list
/// template-param-list ::= '<' template-param-decls '>'
/// template-param-decls ::= id | id ',' template-param-decls
FunctionTemplate* Parser::parseFunctionTemplateProto(TypeDecl* receiverTypeDecl, AccessLevel accessLevel, Type type, llvm::StringRef name, Location location) {
    std::vector<GenericParamDecl> genericParams;
    auto decl = parseFunctionProto(false, receiverTypeDecl, accessLevel, &genericParams, type, name, location);
    return makeAST<FunctionTemplate>(std::move(genericParams), decl, accessLevel);
}

/// function-decl ::= function-proto '{' stmt* '}'
FunctionDecl* Parser::parseFunctionDecl(TypeDecl* receiverTypeDecl, AccessLevel accessLevel, bool requireBody, Type type, llvm::StringRef name,
                                        Location location) {
    auto decl = parseFunctionProto(false, receiverTypeDecl, accessLevel, nullptr, type, name, location);

    if (requireBody || currentToken() == Token::LeftBrace) {
        decl->body = parseBlock(decl);
    }

    if (lookAhead(-1) != Token::RightBrace) {
        parseStmtTerminator();
    }

    return decl;
}

/// function-template-decl ::= function-template-proto '{' stmt* '}'
FunctionTemplate* Parser::parseFunctionTemplate(TypeDecl* receiverTypeDecl, AccessLevel accessLevel, Type type, llvm::StringRef name, Location location) {
    auto decl = parseFunctionTemplateProto(receiverTypeDecl, accessLevel, type, name, location);
    decl->functionDecl->body = parseBlock(decl);
    return decl;
}

/// extern-function-decl ::= 'private'? 'extern' function-proto ('\n' | ';')
FunctionDecl* Parser::parseExternFunctionDecl(AccessLevel accessLevel, Type type, llvm::StringRef name, Location location) {
    auto decl = parseFunctionProto(true, nullptr, accessLevel, nullptr, type, name, location);
    parseStmtTerminator();
    return decl;
}

/// constructor-decl ::= id param-list '{' stmt* '}'
ConstructorDecl* Parser::parseConstructorDecl(TypeDecl& receiverTypeDecl, AccessLevel accessLevel) {
    ASSERT(currentToken() == Token::Identifier);
    auto location = consumeToken().location;
    auto params = parseParamList(nullptr);
    auto decl = makeAST<ConstructorDecl>(receiverTypeDecl, std::move(params), accessLevel, location);
    decl->body = parseBlock(decl);
    return decl;
}

/// destructor-decl ::= '~' id param-list '{' stmt* '}'
DestructorDecl* Parser::parseDestructorDecl(TypeDecl& receiverTypeDecl) {
    ASSERT(currentToken() == Token::Tilde);
    auto location = consumeToken().location;
    if (parse(Token::Identifier).getString() != receiverTypeDecl.getName()) {
        REPORT_ERROR(location, "expected '" << receiverTypeDecl.getName() << "' after '~'");
    }
    auto params = parseParamList(nullptr);
    if (!params.empty()) REPORT_ERROR(location, "destructors cannot have parameters");
    auto decl = makeAST<DestructorDecl>(receiverTypeDecl, location);
    decl->body = parseBlock(decl);
    return decl;
}

/// field-decl ::= type id ('=' expr)? ('\n' | ';')
FieldDecl Parser::parseFieldDecl(TypeDecl& typeDecl, AccessLevel accessLevel, Type type, llvm::StringRef name, Location location) {
    Expr* defaultValue = nullptr;

    if (currentToken() == Token::Assignment) {
        consumeToken();
        defaultValue = parseExpr();
    }

    parseStmtTerminator();
    return FieldDecl(type, name.str(), defaultValue, typeDecl, accessLevel, location);
}

/// type-template-decl ::= ('struct' | 'interface') id generic-param-list? '{' member-decl* '}' ';'?
TypeTemplate* Parser::parseTypeTemplate(AccessLevel accessLevel) {
    std::vector<GenericParamDecl> genericParams;
    auto typeDecl = parseTypeDecl(&genericParams, accessLevel);
    return makeAST<TypeTemplate>(std::move(genericParams), typeDecl, accessLevel);
}

Token Parser::parseTypeHeader(std::vector<Type>& interfaces, std::vector<GenericParamDecl>* genericParams) {
    auto name = parse(Token::Identifier);

    if (currentToken() == Token::Less) {
        parseGenericParamList(*genericParams);
    }

    if (currentToken() == Token::Colon) {
        consumeToken();
        interfaces = parseNonEmptyTypeList();
    }

    return name;
}

/// type-decl ::= ('struct' | 'interface') id generic-param-list? interface-list? '{' member-decl* '}' ';'?
/// interface-list ::= ':' non-empty-type-list
/// member-decl ::= field-decl | function-decl | constructor-decl | destructor-decl
TypeDecl* Parser::parseTypeDecl(std::vector<GenericParamDecl>* genericParams, AccessLevel typeAccessLevel) {
    TypeTag tag;
    switch (consumeToken()) {
    case Token::Struct:
        tag = TypeTag::Struct;
        break;
    case Token::Interface:
        tag = TypeTag::Interface;
        break;
    default:
        llvm_unreachable("invalid token");
    }

    std::vector<Type> interfaces;
    auto typeName = parseTypeHeader(interfaces, genericParams);
    auto typeDecl = makeAST<TypeDecl>(tag, typeName.getString().str(), std::vector<Type>(), std::move(interfaces), typeAccessLevel, *currentModule, nullptr,
                                      typeName.location);
    bool hasConstructor = false;
    parse(Token::LeftBrace);

    while (currentToken() != Token::RightBrace) {
        AccessLevel accessLevel = AccessLevel::Default;

    start:
        switch (currentToken()) {
        case Token::Private:
            if (tag == TypeTag::Interface) {
                WARN(getCurrentLocation(), "interface members cannot be private");
            }
            if (accessLevel != AccessLevel::Default) {
                WARN(getCurrentLocation(), "duplicate access specifier");
            }
            accessLevel = AccessLevel::Private;
            consumeToken();
            goto start;
        case Token::Tilde:
            if (accessLevel != AccessLevel::Default) {
                WARN(lookAhead(-1).location, "destructors cannot be " << accessLevel);
            }
            typeDecl->addMethod(parseDestructorDecl(*typeDecl));
            break;
        case Token::Identifier:
            if (lookAhead(1) == Token::LeftParen && currentToken().getString() == typeName.getString()) {
                typeDecl->addMethod(parseConstructorDecl(*typeDecl, accessLevel));
                hasConstructor = true;
                break;
            }
            LLVM_FALLTHROUGH;
        default: {
            auto type = parseType();
            auto location = getCurrentLocation();
            auto name = parseFunctionName(&*typeDecl);
            auto requireBody = tag != TypeTag::Interface;

            switch (currentToken()) {
            case Token::LeftParen:
                typeDecl->addMethod(parseFunctionDecl(typeDecl, accessLevel, requireBody, type, name, location));
                break;
            case Token::Less:
                typeDecl->addMethod(parseFunctionTemplate(typeDecl, accessLevel, type, name, location));
                break;
            default:
                typeDecl->addField(parseFieldDecl(*typeDecl, accessLevel, type, name, location));
                break;
            }
            break;
        }
        }
    }

    if (tag == TypeTag::Struct && !hasConstructor) {
        typeDecl->addAutogeneratedConstructor();
    }

    consumeToken();
    // Allow an optional trailing ';' (e.g. `struct S {...};`).
    if (currentToken() == Token::Semicolon) consumeToken();
    return typeDecl;
}

/// enum-template-decl ::= 'enum' id generic-param-list? '{' enum-case-decl* '}' ';'?
TypeTemplate* Parser::parseEnumTemplate(AccessLevel accessLevel) {
    std::vector<GenericParamDecl> genericParams;
    auto enumDecl = parseEnumDecl(&genericParams, accessLevel);
    return makeAST<TypeTemplate>(std::move(genericParams), enumDecl, accessLevel);
}

/// enum-decl ::= 'enum' id generic-param-list? interface-list? '{' enum-case-decl* '}' ';'?
/// enum-case-decl ::= id tuple-type? (',' | '\n' | ';')
EnumDecl* Parser::parseEnumDecl(std::vector<GenericParamDecl>* genericParams, AccessLevel typeAccessLevel) {
    ASSERT(currentToken() == Token::Enum);
    consumeToken();

    std::vector<Type> interfaces;
    auto name = parseTypeHeader(interfaces, genericParams);

    parse(Token::LeftBrace);
    std::vector<EnumCase> cases;
    auto valueCounter = llvm::APSInt::get(0);

    while (currentToken() != Token::RightBrace) {
        auto caseName = parse(Token::Identifier);
        Type associatedType;

        if (currentToken() == Token::LeftParen) {
            associatedType = parseTupleType();
        }

        auto value = makeAST<IntLiteralExpr>(valueCounter, caseName.location);
        cases.push_back(EnumCase(caseName.getString().str(), value, associatedType, typeAccessLevel, caseName.location));
        ++valueCounter;

        if (currentToken() == Token::Comma) {
            consumeToken();
        } else {
            parseStmtTerminator();
        }
    }

    consumeToken();
    // Allow an optional trailing ';' (e.g. `enum E {...};`).
    if (currentToken() == Token::Semicolon) consumeToken();
    return makeAST<EnumDecl>(name.getString().str(), std::move(cases), typeAccessLevel, *currentModule, nullptr, name.location);
}

/// import-decl ::= 'import' (id | string-literal) ('\n' | ';')
ImportDecl* Parser::parseImportDecl() {
    ASSERT(currentToken() == Token::Import);
    consumeToken();

    auto location = getCurrentLocation();
    std::string importTarget;

    if (currentToken() == Token::StringLiteral) {
        importTarget = parseStringLiteral()->value;
    } else {
        importTarget = parse({Token::Identifier, Token::StringLiteral}, "after 'import'").getString().str();
    }

    parseStmtTerminator("after 'import' declaration");
    return makeAST<ImportDecl>(std::move(importTarget), *currentModule, location);
}

void Parser::parseIfdefBody(std::vector<Decl*>* activeDecls) {
    if (currentToken() == Token::HashIf) {
        parseIfdef(activeDecls);
    } else {
        if (activeDecls) {
            activeDecls->emplace_back(parseTopLevelDecl(true));
        } else {
            parseTopLevelDecl(false);
        }
    }
}

void Parser::parseIfdef(std::vector<Decl*>* activeDecls) {
    ASSERT(currentToken() == Token::HashIf);
    consumeToken();
    bool negate = currentToken() == Token::Not;
    if (negate) consumeToken();
    auto identifier = parse(Token::Identifier);

    bool condition = false;
    if (identifier.getString() == "hasInclude") {
        parse(Token::LeftParen);
        auto header = parse(Token::StringLiteral);
        parse(Token::RightParen);

        for (llvm::StringRef path : llvm::concat<const std::string>(options.importSearchPaths, options.frameworkSearchPaths)) {
            auto headerPath = (path + "/" + header.getString().drop_back().drop_front()).str();
            if (llvm::sys::fs::exists(headerPath) && !llvm::sys::fs::is_directory(headerPath)) {
                condition = true;
                break;
            }
        }
    } else {
        condition = llvm::is_contained(options.defines, identifier.getString());
        if (options.warnUndefinedMacros && !condition && currentModule->name != "std") {
            WARN(identifier.location, "undefined macro '" << identifier.getString() << "', assuming false");
        }
    }

    if (negate) condition = !condition;

    while (!currentToken().is({Token::HashElse, Token::HashEndif})) {
        parseIfdefBody(condition ? activeDecls : nullptr);
    }

    if (currentToken() == Token::HashElse) {
        consumeToken();
        while (currentToken() != Token::HashEndif) {
            parseIfdefBody(condition ? nullptr : activeDecls);
        }
    }

    consumeToken();
}

/// top-level-decl ::= function-decl | extern-function-decl | type-decl | enum-decl | import-decl | var-decl
/// @throws CompileError
Decl* Parser::parseTopLevelDecl(bool addToSymbolTable) {
    AccessLevel accessLevel = AccessLevel::Default;
    Decl* decl = nullptr;

start:
    switch (currentToken()) {
    case Token::Private:
        if (accessLevel != AccessLevel::Default) WARN(getCurrentLocation(), "duplicate access specifier");
        accessLevel = AccessLevel::Private;
        consumeToken();
        goto start;
    case Token::Extern:
        consumeToken();
        return parseTopLevelFunctionOrVariable(true, addToSymbolTable, accessLevel);
    case Token::Struct:
    case Token::Interface:
        if (lookAhead(2) == Token::Less) {
            decl = parseTypeTemplate(accessLevel);
            if (addToSymbolTable) currentModule->addToSymbolTable(llvm::cast<TypeTemplate>(*decl));
        } else {
            decl = parseTypeDecl(nullptr, accessLevel);
            if (addToSymbolTable) currentModule->addToSymbolTable(llvm::cast<TypeDecl>(*decl));
        }
        break;
    case Token::Enum:
        if (lookAhead(2) == Token::Less) {
            decl = parseEnumTemplate(accessLevel);
            if (addToSymbolTable) currentModule->addToSymbolTable(llvm::cast<TypeTemplate>(*decl));
        } else {
            decl = parseEnumDecl(nullptr, accessLevel);
            if (addToSymbolTable) currentModule->addToSymbolTable(llvm::cast<EnumDecl>(*decl));
        }
        break;
    case Token::Var:
    case Token::Const:
        // Determine if this is a constant declaration or if the const is part of a type.
        if (currentToken() == Token::Const && lookAhead(2) != Token::Assignment) {
            return parseTopLevelFunctionOrVariable(false, addToSymbolTable, accessLevel);
        }
        decl = parseVarDecl(nullptr, accessLevel);
        if (addToSymbolTable) currentModule->addToSymbolTable(llvm::cast<VarDecl>(*decl));
        break;
    case Token::Import:
        if (accessLevel != AccessLevel::Default) {
            WARN(lookAhead(-1).location, "imports cannot have access specifiers");
        }
        return parseImportDecl();
    default:
        return parseTopLevelFunctionOrVariable(false, addToSymbolTable, accessLevel);
    }

    return decl;
}

Decl* Parser::parseTopLevelFunctionOrVariable(bool isExtern, bool addToSymbolTable, AccessLevel accessLevel) {
    Decl* decl;
    // A call-shaped `name(...)` here is a misplaced statement, not a function
    // type; report that instead of a confusing type error from inside the
    // parentheses. Tokens that can start or end a function-type parameter
    // list keep the normal path.
    if (currentToken() == Token::Identifier && lookAhead(1) == Token::LeftParen
        && !lookAhead(2).is({Token::Identifier, Token::Const, Token::LeftParen, Token::RightParen, Token::DotDotDot})) {
        ERROR(getCurrentLocation(), "statements are not allowed in global scope");
    }
    auto type = parseType();
    auto location = getCurrentLocation();
    auto name = parseFunctionName(nullptr);

    switch (currentToken()) {
    case Token::LeftParen:
        if (isExtern) {
            decl = parseExternFunctionDecl(accessLevel, type, name, location);
        } else {
            decl = parseFunctionDecl(nullptr, accessLevel, false, type, name, location);
        }
        if (addToSymbolTable) currentModule->addToSymbolTable(llvm::cast<FunctionDecl>(*decl));
        break;
    case Token::Less:
        decl = parseFunctionTemplate(nullptr, accessLevel, type, name, location);
        if (addToSymbolTable) currentModule->addToSymbolTable(llvm::cast<FunctionTemplate>(*decl));
        break;
    default:
        decl = parseVarDeclAfterName(nullptr, accessLevel, type, name, location);
        if (addToSymbolTable) currentModule->addToSymbolTable(llvm::cast<VarDecl>(*decl));
        break;
    }

    return decl;
}

void Parser::parse() {
    std::vector<Decl*> topLevelDecls;
    SourceFile sourceFile(lexer.getFilePath(), currentModule);

    try {
        while (currentToken() != Token::None) {
            if (currentToken() == Token::HashIf) {
                parseIfdef(&topLevelDecls);
            } else {
                auto previousTokenIndex = currentTokenIndex;
                topLevelDecls.push_back(parseTopLevelDecl(true));
                if (currentTokenIndex == previousTokenIndex) break;
            }
        }
    } catch (const CompileError& error) {
        error.report();
    }

    sourceFile.topLevelDecls = std::move(topLevelDecls);
    currentModule->addSourceFile(std::move(sourceFile));
}
