#include <vector>
#include <forward_list>
#include <sstream>
#pragma warning(push, 0)
#include <llvm/ADT/APSInt.h>
#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/Support/ErrorHandling.h>
#include <llvm/Support/MemoryBuffer.h>
#pragma warning(pop)
#include "parse.h"
#include "lex.h"
#include "../ast/token.h"
#include "../ast/decl.h"
#include "../ast/module.h"
#include "../sema/typecheck.h"
#include "../support/utility.h"

using namespace delta;

namespace delta {
extern const char* currentFilePath;
}

namespace {

std::vector<Token> tokenBuffer;
size_t currentTokenIndex;
Module* currentModule;

Token currentToken() {
    ASSERT(currentTokenIndex < tokenBuffer.size());
    return tokenBuffer[currentTokenIndex];
}

SourceLocation getCurrentLocation() {
    return currentToken().getLocation();
}

Token lookAhead(int offset) {
    if (int(currentTokenIndex) + offset < 0) return Token::None;
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
std::string quote(Token::Kind tokenKind) {
    std::ostringstream stream;
    if (tokenKind < Token::Break) {
        stream << tokenKind;
    } else {
        stream << '\'' << tokenKind << '\'';
    }
    return stream.str();
}

[[noreturn]] void unexpectedToken(Token token, llvm::ArrayRef<Token::Kind> expected = {},
                                  const char* contextInfo = nullptr) {
    if (expected.size() == 0) {
        error(token.getLocation(), "unexpected ", quote(token),
              contextInfo ? " " : "", contextInfo ? contextInfo : "");
    } else {
        error(token.getLocation(), "expected ", toDisjunctiveList(expected, quote),
              contextInfo ? " " : "", contextInfo ? contextInfo : "",
              ", got ", quote(token));
    }
}

void expect(llvm::ArrayRef<Token::Kind> expected, const char* contextInfo) {
    if (!llvm::is_contained(expected, currentToken())) {
        unexpectedToken(currentToken(), expected, contextInfo);
    }
}

Token parse(llvm::ArrayRef<Token::Kind> expected, const char* contextInfo = nullptr) {
    expect(expected, contextInfo);
    return consumeToken();
}

static void checkStmtTerminatorConsistency(Token::Kind currentTerminator,
                                           llvm::function_ref<SourceLocation()> getLocation) {
    static Token::Kind previousTerminator = Token::None;
    static const char* filePath = nullptr;

    if (filePath != delta::currentFilePath) {
        filePath = delta::currentFilePath;
        previousTerminator = Token::None;
    }

    if (previousTerminator == Token::None) {
        previousTerminator = currentTerminator;
    } else if (previousTerminator != currentTerminator) {
        warning(getLocation(), "inconsistent statement terminator, expected ", quote(previousTerminator));
    }
}

void parseStmtTerminator(const char* contextInfo = nullptr) {
    if (getCurrentLocation().line != lookAhead(-1).getLocation().line) {
        checkStmtTerminatorConsistency(Token::Newline, [] {
            // TODO: Use pre-existing buffer instead of reading from file here.
            readLineFromFile(lookAhead(-1).getLocation());
            std::ifstream file(getCurrentLocation().file);
            for (auto line = lookAhead(-1).getLocation().line; --line;) {
                file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            }
            std::string line;
            std::getline(file, line);
            return SourceLocation(getCurrentLocation().file, lookAhead(-1).getLocation().line,
                                  static_cast<SourceLocation::IntegerType>(line.size() + 1));
        });
        return;
    }

    switch (currentToken()) {
        case Token::RightBrace:
            checkStmtTerminatorConsistency(Token::Newline, getCurrentLocation);
            return;
        case Token::Semicolon:
            checkStmtTerminatorConsistency(Token::Semicolon, getCurrentLocation);
            consumeToken();
            return;
        default:
            unexpectedToken(currentToken(), { Token::Newline, Token::Semicolon }, contextInfo);
    }
}

std::unique_ptr<Expr> parseExpr();
std::unique_ptr<Expr> parsePreOrPostfixExpr();
std::vector<std::unique_ptr<Expr>> parseExprList();
std::vector<std::unique_ptr<Stmt>> parseStmtsUntil(Token end, Decl* parent);
std::vector<std::unique_ptr<Stmt>> parseStmtsUntilOneOf(Token end1, Token end2, Token end3, Decl* parent);
Type parseType();
std::unique_ptr<TypeDecl> parseTypeDecl(std::vector<GenericParamDecl>* genericParams);
std::vector<ParamDecl> parseParamList(bool* isVariadic, bool withTypes);
ParamDecl parseParam(bool withType);

/// argument-list ::= '(' ')' | '(' nonempty-argument-list ')'
/// nonempty-argument-list ::= argument | nonempty-argument-list ',' argument
/// argument ::= (id ':')? expr
std::vector<Argument> parseArgumentList() {
    parse(Token::LeftParen);
    std::vector<Argument> args;
    while (currentToken() != Token::RightParen) {
        std::string name;
        SourceLocation location = SourceLocation::invalid();
        if (lookAhead(1) == Token::Colon) {
            auto result = parse(Token::Identifier);
            name = std::move(result.getString());
            location = result.getLocation();
            consumeToken();
        }
        auto value = parseExpr();
        if (!location.isValid()) location = value->getLocation();
        args.push_back({ std::move(name), std::move(value), location });
        if (currentToken() != Token::RightParen) parse(Token::Comma);
    }
    consumeToken();
    return args;
}

/// var-expr ::= id
std::unique_ptr<VarExpr> parseVarExpr() {
    ASSERT(currentToken().is(Token::Identifier, Token::Init));
    auto id = consumeToken();
    return llvm::make_unique<VarExpr>(id.getString(), id.getLocation());
}

std::unique_ptr<VarExpr> parseThis() {
    ASSERT(currentToken() == Token::This);
    auto expr = llvm::make_unique<VarExpr>("this", getCurrentLocation());
    consumeToken();
    return expr;
}

std::string replaceEscapeChars(llvm::StringRef literalContent, SourceLocation literalStartLocation) {
    std::string result;
    result.reserve(literalContent.size());

    for (auto it = literalContent.begin(), end = literalContent.end(); it != end; ++it) {
        if (*it == '\\') {
            ++it;
            ASSERT(it != end);
            switch (*it) {
                case '0': result += '\0'; break;
                case 'a': result += '\a'; break;
                case 'b': result += '\b'; break;
                case 'n': result += '\n'; break;
                case 'r': result += '\r'; break;
                case 't': result += '\t'; break;
                case 'v': result += '\v'; break;
                case '"': result += '"'; break;
                case '\'': result += '\''; break;
                case '\\': result += '\\'; break;
                default:
                    auto itColumn = literalStartLocation.column + 1 + (it - literalContent.begin());
                    SourceLocation itLocation(literalStartLocation.file, literalStartLocation.line, itColumn);
                    error(itLocation, "unknown escape character '\\", *it, "'");
            }
            continue;
        }
        result += *it;
    }
    return result;
}

std::unique_ptr<StringLiteralExpr> parseStringLiteral() {
    ASSERT(currentToken() == Token::StringLiteral);
    auto content = replaceEscapeChars(currentToken().getString().drop_back().drop_front(), getCurrentLocation());
    auto expr = llvm::make_unique<StringLiteralExpr>(std::move(content), getCurrentLocation());
    consumeToken();
    return expr;
}

std::unique_ptr<CharacterLiteralExpr> parseCharacterLiteral() {
    ASSERT(currentToken() == Token::CharacterLiteral);
    auto content = replaceEscapeChars(currentToken().getString().drop_back().drop_front(), getCurrentLocation());
    if (content.size() != 1) error(getCurrentLocation(), "character literal must consist of a single UTF-8 byte");
    auto expr = llvm::make_unique<CharacterLiteralExpr>(content[0], getCurrentLocation());
    consumeToken();
    return expr;
}

std::unique_ptr<IntLiteralExpr> parseIntLiteral() {
    ASSERT(currentToken() == Token::IntegerLiteral);
    auto expr = llvm::make_unique<IntLiteralExpr>(currentToken().getIntegerValue(),
                                                  getCurrentLocation());
    consumeToken();
    return expr;
}

std::unique_ptr<FloatLiteralExpr> parseFloatLiteral() {
    ASSERT(currentToken() == Token::FloatLiteral);
    auto expr = llvm::make_unique<FloatLiteralExpr>(currentToken().getFloatingPointValue(),
                                                    getCurrentLocation());
    consumeToken();
    return expr;
}

std::unique_ptr<BoolLiteralExpr> parseBoolLiteral() {
    std::unique_ptr<BoolLiteralExpr> expr;
    switch (currentToken()) {
        case Token::True: expr = llvm::make_unique<BoolLiteralExpr>(true, getCurrentLocation()); break;
        case Token::False: expr = llvm::make_unique<BoolLiteralExpr>(false, getCurrentLocation()); break;
        default: llvm_unreachable("all cases handled");
    }
    consumeToken();
    return expr;
}

std::unique_ptr<NullLiteralExpr> parseNullLiteral() {
    ASSERT(currentToken() == Token::Null);
    auto expr = llvm::make_unique<NullLiteralExpr>(getCurrentLocation());
    consumeToken();
    return expr;
}

/// array-literal ::= '[' expr-list ']'
std::unique_ptr<ArrayLiteralExpr> parseArrayLiteral() {
    ASSERT(currentToken() == Token::LeftBracket);
    auto location = getCurrentLocation();
    consumeToken();
    auto elements = parseExprList();
    parse(Token::RightBracket);
    return llvm::make_unique<ArrayLiteralExpr>(std::move(elements), location);
}

/// non-empty-type-list ::= type | type ',' non-empty-type-list
std::vector<Type> parseNonEmptyTypeList() {
    std::vector<Type> types;

    while (true) {
        types.push_back(parseType());

        if (currentToken() == Token::Comma) {
            consumeToken();
            continue;
        }

        if (currentToken() == Token::RightShift) {
            tokenBuffer[currentTokenIndex] = Token::Greater;
            tokenBuffer.insert(tokenBuffer.begin() + currentTokenIndex + 1, Token::Greater);
        }

        break;
    }

    return types;
}

/// generic-argument-list ::= '<' non-empty-type-list '>'
std::vector<Type> parseGenericArgumentList() {
    ASSERT(currentToken() == Token::Less);
    consumeToken();
    std::vector<Type> genericArgs = parseNonEmptyTypeList();
    parse(Token::Greater);
    return genericArgs;
}

int64_t parseArraySizeInBrackets() {
    ASSERT(currentToken() == Token::LeftBracket);
    consumeToken();
    int64_t arraySize;

    switch (currentToken()) {
        case Token::IntegerLiteral:
            arraySize = consumeToken().getIntegerValue();
            break;
        case Token::RightBracket:
            arraySize = ArrayType::runtimeSize;
            break;
        case Token::QuestionMark:
            consumeToken();
            arraySize = ArrayType::unknownSize;
            break;
        default:
            error(getCurrentLocation(), "non-literal array bounds not implemented yet");
    }

    parse(Token::RightBracket);
    return arraySize;
}

/// simple-type ::= id | id generic-argument-list | id '[' int-literal? ']'
Type parseSimpleType(bool isMutable) {
    llvm::StringRef id = parse(Token::Identifier).getString();

    Type type;
    std::vector<Type> genericArgs;

    switch (currentToken()) {
        case Token::Less:
            genericArgs = parseGenericArgumentList();
            LLVM_FALLTHROUGH;
        default:
            return BasicType::get(id, std::move(genericArgs), isMutable);
        case Token::LeftBracket:
            type = BasicType::get(id, {}, isMutable);
            break;
    }

    return ArrayType::get(type, parseArraySizeInBrackets());
}

/// function-type ::= param-type-list '->' type
/// param-type-list ::= '(' param-types ')'
/// param-types ::= '' | non-empty-param-types
/// non-empty-param-types ::= type | type ',' non-empty-param-types
Type parseFunctionType() {
    ASSERT(currentToken() == Token::LeftParen);
    consumeToken();
    std::vector<Type> paramTypes;

    while (currentToken() != Token::RightParen) {
        paramTypes.emplace_back(parseType());
        if (currentToken() != Token::RightParen) parse(Token::Comma);
    }

    consumeToken();
    parse(Token::RightArrow);
    Type returnType = parseType();
    return FunctionType::get(returnType, std::move(paramTypes));
}

/// type ::= simple-type | 'mutable' simple-type | type 'mutable'? '*' | type 'mutable'? '?' | function-type
Type parseType() {
    Type type;

    switch (currentToken()) {
        case Token::Identifier:
            type = parseSimpleType(false);
            break;
        case Token::Mutable:
            consumeToken();
            type = parseSimpleType(true);
            type.setMutable(true);
            break;
        case Token::LeftParen:
            type = parseFunctionType();
            break;
        default:
            unexpectedToken(currentToken(), { Token::Identifier, Token::Mutable, Token::LeftParen });
    }

    while (true) {
        switch (currentToken()) {
            case Token::Star:
                type = PointerType::get(type);
                consumeToken();
                break;
            case Token::QuestionMark:
                type = OptionalType::get(type);
                consumeToken();
                break;
            case Token::Mutable:
                consumeToken();
                switch (currentToken()) {
                    case Token::Star:
                        type = PointerType::get(type, true);
                        consumeToken();
                        break;
                    case Token::QuestionMark:
                        type = OptionalType::get(type, true);
                        consumeToken();
                    default:
                        unexpectedToken(currentToken(), { Token::Star, Token::QuestionMark }, "after 'mutable'");
                }
                break;
            case Token::LeftBracket:
                type = ArrayType::get(type, parseArraySizeInBrackets());
                break;
            case Token::And:
                error(getCurrentLocation(), "Delta doesn't have C++-style references; ",
                      "use pointers ('*') instead, they are non-null by default");
            default:
                return type;
        }
    }
}

/// cast-expr ::= 'cast' '<' type '>' '(' expr ')'
std::unique_ptr<CastExpr> parseCastExpr() {
    ASSERT(currentToken() == Token::Cast);
    auto location = getCurrentLocation();
    consumeToken();
    parse(Token::Less);
    auto type = parseType();
    parse(Token::Greater);
    parse(Token::LeftParen);
    auto expr = parseExpr();
    parse(Token::RightParen);
    return llvm::make_unique<CastExpr>(type, std::move(expr), location);
}

/// sizeof-expr ::= 'sizeof' '(' type ')'
std::unique_ptr<SizeofExpr> parseSizeofExpr() {
    assert(currentToken() == Token::Sizeof);
    auto location = getCurrentLocation();
    consumeToken();
    parse(Token::LeftParen);
    auto type = parseType();
    parse(Token::RightParen);
    return llvm::make_unique<SizeofExpr>(type, location);
}

/// addressof-expr ::= 'addressof' '(' expr ')'
std::unique_ptr<AddressofExpr> parseAddressofExpr() {
    assert(currentToken() == Token::Addressof);
    auto location = getCurrentLocation();
    consumeToken();
    parse(Token::LeftParen);
    auto operand = parseExpr();
    parse(Token::RightParen);
    return llvm::make_unique<AddressofExpr>(std::move(operand), location);
}

/// member-expr ::= expr '.' id
std::unique_ptr<MemberExpr> parseMemberExpr(std::unique_ptr<Expr> lhs) {
    auto location = getCurrentLocation();
    llvm::StringRef member;

    if (currentToken().is(Token::Identifier, Token::Init, Token::Deinit)) {
        member = consumeToken().getString();
    } else {
        unexpectedToken(currentToken(), Token::Identifier);
    }

    return llvm::make_unique<MemberExpr>(std::move(lhs), member, location);
}

/// subscript-expr ::= expr '[' expr ']'
std::unique_ptr<SubscriptExpr> parseSubscript(std::unique_ptr<Expr> operand) {
    ASSERT(currentToken() == Token::LeftBracket);
    auto location = getCurrentLocation();
    consumeToken();
    auto index = parseExpr();
    parse(Token::RightBracket);
    return llvm::make_unique<SubscriptExpr>(std::move(operand), std::move(index), location);
}

/// unwrap-expr ::= expr '!'
std::unique_ptr<UnwrapExpr> parseUnwrapExpr(std::unique_ptr<Expr> operand) {
    ASSERT(currentToken() == Token::Not);
    auto location = getCurrentLocation();
    consumeToken();
    return llvm::make_unique<UnwrapExpr>(std::move(operand), location);
}

/// call-expr ::= expr generic-argument-list? '(' arguments ')'
std::unique_ptr<CallExpr> parseCallExpr(std::unique_ptr<Expr> callee) {
    std::vector<Type> genericArgs;
    if (currentToken() == Token::Less) {
        genericArgs = parseGenericArgumentList();
    }
    auto location = getCurrentLocation();
    auto args = parseArgumentList();
    return llvm::make_unique<CallExpr>(std::move(callee), std::move(args),
                                       std::move(genericArgs), location);
}

std::unique_ptr<LambdaExpr> parseLambdaExpr() {
    ASSERT(currentToken().is(Token::LeftParen, Token::Identifier));
    auto location = getCurrentLocation();
    std::vector<ParamDecl> params;

    if (currentToken() == Token::Identifier) {
        params.push_back(parseParam(true));
    } else {
        params = parseParamList(nullptr, true);
    }

    parse(Token::RightArrow);
    auto body = parseExpr();
    return llvm::make_unique<LambdaExpr>(std::move(params), std::move(body), location);
}

/// paren-expr ::= '(' expr ')'
std::unique_ptr<Expr> parseParenExpr() {
    ASSERT(currentToken() == Token::LeftParen);
    consumeToken();
    auto expr = parseExpr();
    parse(Token::RightParen);
    return expr;
}

/// if-expr ::= expr '?' expr ':' expr
std::unique_ptr<IfExpr> parseIfExpr(std::unique_ptr<Expr> condition) {
    ASSERT(currentToken() == Token::QuestionMark);
    auto location = getCurrentLocation();
    consumeToken();
    auto thenExpr = parseExpr();
    parse(Token::Colon);
    auto elseExpr = parseExpr();
    return llvm::make_unique<IfExpr>(std::move(condition), std::move(thenExpr), std::move(elseExpr), location);
}

bool shouldParseGenericArgumentList() {
    // Temporary hack: use spacing to determine whether to parse a generic argument list
    // of a less-than binary expression. Zero spaces on either side of '<' will cause it
    // to be interpreted as a generic argument list, for now.
    return lookAhead(0).getLocation().column + int(lookAhead(0).getString().size()) == lookAhead(1).getLocation().column
           || lookAhead(1).getLocation().column + 1 == lookAhead(2).getLocation().column;
}

/// postfix-expr ::= postfix-expr postfix-op | call-expr | variable-expr | string-literal |
///                  int-literal | float-literal | bool-literal | null-literal |
///                  paren-expr | array-literal | cast-expr | subscript-expr | member-expr
///                  unwrap-expr | lambda-expr | sizeof-expr | addressof-expr
std::unique_ptr<Expr> parsePostfixExpr() {
    std::unique_ptr<Expr> expr;
    switch (currentToken()) {
        case Token::Identifier:
        case Token::Init:
            switch (lookAhead(1)) {
                case Token::LeftParen: expr = parseCallExpr(parseVarExpr()); break;
                case Token::Less:
                    if (shouldParseGenericArgumentList()) {
                        expr = parseCallExpr(parseVarExpr());
                        break;
                    }
                    LLVM_FALLTHROUGH;
                default: expr = parseVarExpr(); break;
            }
            break;
        case Token::StringLiteral: expr = parseStringLiteral(); break;
        case Token::CharacterLiteral: expr = parseCharacterLiteral(); break;
        case Token::IntegerLiteral: expr = parseIntLiteral(); break;
        case Token::FloatLiteral: expr = parseFloatLiteral(); break;
        case Token::True: case Token::False: expr = parseBoolLiteral(); break;
        case Token::Null: expr = parseNullLiteral(); break;
        case Token::This: expr = parseThis(); break;
        case Token::LeftParen:
            if (lookAhead(1) == Token::RightParen && lookAhead(2) == Token::RightArrow) {
                expr = parseLambdaExpr();
            } else if (lookAhead(1) == Token::Identifier && lookAhead(2) == Token::Colon) {
                expr = parseLambdaExpr();
            } else {
                expr = parseParenExpr();
            }
            break;
        case Token::LeftBracket: expr = parseArrayLiteral(); break;
        case Token::Cast: expr = parseCastExpr(); break;
        case Token::Sizeof: expr = parseSizeofExpr(); break;
        case Token::Addressof: expr = parseAddressofExpr(); break;
        default: unexpectedToken(currentToken()); break;
    }
    while (true) {
        switch (currentToken()) {
            case Token::LeftBracket:
                expr = parseSubscript(std::move(expr));
                break;
            case Token::LeftParen:
                expr = parseCallExpr(std::move(expr));
                break;
            case Token::Dot:
                consumeToken();
                expr = parseMemberExpr(std::move(expr));
                break;
            case Token::Not:
                expr = parseUnwrapExpr(std::move(expr));
                break;
            case Token::QuestionMark:
                expr = parseIfExpr(std::move(expr));
                break;
            default:
                return expr;
        }
    }
}

/// prefix-expr ::= prefix-operator (prefix-expr | postfix-expr)
std::unique_ptr<PrefixExpr> parsePrefixExpr() {
    ASSERT(currentToken().isPrefixOperator());
    auto op = currentToken();
    auto location = getCurrentLocation();
    consumeToken();
    return llvm::make_unique<PrefixExpr>(op, parsePreOrPostfixExpr(), location);
}

std::unique_ptr<Expr> parsePreOrPostfixExpr() {
    return currentToken().isPrefixOperator() ? parsePrefixExpr() : parsePostfixExpr();
}

/// binary-expr ::= expr op expr
std::unique_ptr<Expr> parseBinaryExpr(std::unique_ptr<Expr> lhs, int minPrecedence) {
    while (currentToken().isBinaryOperator() && currentToken().getPrecedence() >= minPrecedence) {
        auto backtrackLocation = currentTokenIndex;
        auto op = consumeToken();
        auto expr = parsePreOrPostfixExpr();
        if (currentToken().isAssignmentOperator()) {
            currentTokenIndex = backtrackLocation;
            break;
        }

        while (currentToken().isBinaryOperator() && currentToken().getPrecedence() > op.getPrecedence()) {
            auto token = consumeToken();
            expr = llvm::make_unique<BinaryExpr>(token, std::move(expr), parsePreOrPostfixExpr(),
                                                 token.getLocation());
        }
        lhs = llvm::make_unique<BinaryExpr>(op, std::move(lhs), std::move(expr), op.getLocation());
    }
    return lhs;
}

/// expr ::= prefix-expr | postfix-expr | binary-expr
std::unique_ptr<Expr> parseExpr() {
    return parseBinaryExpr(parsePreOrPostfixExpr(), 0);
}

/// assign-stmt ::= expr '=' expr ('\n' | ';')
std::unique_ptr<AssignStmt> parseAssignStmt(std::unique_ptr<Expr> lhs) {
    auto location = getCurrentLocation();
    parse(Token::Assignment);
    auto rhs = currentToken() != Token::Undefined ? parseExpr() : nullptr;
    if (!rhs) consumeToken();
    parseStmtTerminator();
    return llvm::make_unique<AssignStmt>(std::move(lhs), std::move(rhs), false, location);
}

/// compound-assign-stmt ::= expr compound-assignment-op expr ('\n' | ';')
std::unique_ptr<AssignStmt> parseCompoundAssignStmt(std::unique_ptr<Expr> lhs = nullptr) {
    if (!lhs) lhs = parseExpr();
    SourceLocation location = getCurrentLocation();
    auto op = BinaryOperator(consumeToken().withoutCompoundEqSuffix());
    auto rhs = parseExpr();
    parseStmtTerminator();

    std::shared_ptr<Expr> sharedLHS = std::move(lhs);
    auto binaryExpr = llvm::make_unique<BinaryExpr>(op, std::shared_ptr<Expr>(sharedLHS),
                                                    std::move(rhs), location);
    return llvm::make_unique<AssignStmt>(std::move(sharedLHS), std::move(binaryExpr), true, location);
}

/// expr-list ::= '' | nonempty-expr-list
/// nonempty-expr-list ::= expr | expr ',' nonempty-expr-list
std::vector<std::unique_ptr<Expr>> parseExprList() {
    std::vector<std::unique_ptr<Expr>> exprs;

    // TODO: Handle empty expression list.
    if (currentToken() == Token::Semicolon || currentToken() == Token::RightBrace) {
        return exprs;
    }

    while (true) {
        exprs.emplace_back(parseExpr());
        if (currentToken() != Token::Comma) return exprs;
        consumeToken();
    }
}

/// return-stmt ::= 'return' expr-list ('\n' | ';')
std::unique_ptr<ReturnStmt> parseReturnStmt() {
    ASSERT(currentToken() == Token::Return);
    auto location = getCurrentLocation();
    consumeToken();

    auto returnValues = parseExprList();
    std::unique_ptr<Expr> returnValue;

    switch (returnValues.size()) {
        case 0:
            break;
        case 1:
            returnValue = std::move(returnValues[0]);
            break;
        default:
            auto location = returnValues[0]->getLocation();
            returnValue = llvm::make_unique<TupleExpr>(std::move(returnValues), location);
            break;
    }

    parseStmtTerminator();
    return llvm::make_unique<ReturnStmt>(std::move(returnValue), location);
}

/// var-decl ::= mutability-specifier id type-specifier? '=' initializer ('\n' | ';')
/// mutability-specifier ::= 'let' | 'var'
/// type-specifier ::= ':' type
/// initializer ::= expr | 'undefined'
std::unique_ptr<VarDecl> parseVarDecl(bool requireInitialValue, Decl* parent) {
    bool isMutable = parse({ Token::Let, Token::Var }) == Token::Var;
    auto name = parse(Token::Identifier);

    Type type;
    if (currentToken() == Token::Colon) {
        consumeToken();
        auto typeLocation = getCurrentLocation();
        type = parseType();
        if (type.isMutable()) error(typeLocation, "type specifier cannot specify mutability");
    }
    type.setMutable(isMutable);

    std::unique_ptr<Expr> initializer;

    if (requireInitialValue) {
        parse(Token::Assignment);
        initializer = currentToken() != Token::Undefined ? parseExpr() : nullptr;
        if (!initializer) consumeToken();
        parseStmtTerminator();
    }

    return llvm::make_unique<VarDecl>(type, name.getString(), std::move(initializer), parent,
                                      *currentModule, name.getLocation());
}

/// var-stmt ::= var-decl
std::unique_ptr<VarStmt> parseVarStmt(Decl* parent) {
    return llvm::make_unique<VarStmt>(parseVarDecl(true, parent));
}

/// call-stmt ::= call-expr ('\n' | ';')
std::unique_ptr<ExprStmt> parseCallStmt(std::unique_ptr<Expr> callExpr) {
    ASSERT(callExpr->isCallExpr());
    auto stmt = llvm::make_unique<ExprStmt>(std::move(callExpr));
    parseStmtTerminator();
    return stmt;
}

/// inc-stmt ::= expr '++' ('\n' | ';')
std::unique_ptr<IncrementStmt> parseIncrementStmt(std::unique_ptr<Expr> operand) {
    auto location = getCurrentLocation();
    parse(Token::Increment);
    parseStmtTerminator();
    return llvm::make_unique<IncrementStmt>(std::move(operand), location);
}

/// dec-stmt ::= expr '--' ('\n' | ';')
std::unique_ptr<DecrementStmt> parseDecrementStmt(std::unique_ptr<Expr> operand) {
    auto location = getCurrentLocation();
    parse(Token::Decrement);
    parseStmtTerminator();
    return llvm::make_unique<DecrementStmt>(std::move(operand), location);
}

/// defer-stmt ::= 'defer' call-expr ('\n' | ';')
std::unique_ptr<DeferStmt> parseDeferStmt() {
    ASSERT(currentToken() == Token::Defer);
    consumeToken();
    // FIXME: Doesn't have to be a variable expression.
    auto stmt = llvm::make_unique<DeferStmt>(parseCallExpr(parseVarExpr()));
    parseStmtTerminator();
    return stmt;
}

/// if-stmt ::= 'if' '(' expr ')' '{' stmt* '}' ('else' else-branch)?
/// else-branch ::= if-stmt | '{' stmt* '}'
std::unique_ptr<IfStmt> parseIfStmt(Decl* parent) {
    ASSERT(currentToken() == Token::If);
    consumeToken();
    parse(Token::LeftParen);
    auto condition = parseExpr();
    parse(Token::RightParen);
    parse(Token::LeftBrace);
    auto thenStmts = parseStmtsUntil(Token::RightBrace, parent);
    parse(Token::RightBrace);
    std::vector<std::unique_ptr<Stmt>> elseStmts;
    if (currentToken() != Token::Else)
        return llvm::make_unique<IfStmt>(std::move(condition), std::move(thenStmts),
                                         std::move(elseStmts));
    consumeToken();
    if (currentToken() == Token::LeftBrace) {
        consumeToken();
        elseStmts = parseStmtsUntil(Token::RightBrace, parent);
        parse(Token::RightBrace);
        return llvm::make_unique<IfStmt>(std::move(condition), std::move(thenStmts),
                                         std::move(elseStmts));
    }
    if (currentToken() == Token::If) {
        elseStmts.emplace_back(parseIfStmt(parent));
        return llvm::make_unique<IfStmt>(std::move(condition), std::move(thenStmts),
                                         std::move(elseStmts));
    }
    unexpectedToken(currentToken(), { Token::LeftBrace, Token::If });
}

/// while-stmt ::= 'while' '(' expr ')' '{' stmt* '}'
std::unique_ptr<WhileStmt> parseWhileStmt(Decl* parent) {
    ASSERT(currentToken() == Token::While);
    consumeToken();
    parse(Token::LeftParen);
    auto condition = parseExpr();
    parse(Token::RightParen);
    parse(Token::LeftBrace);
    auto body = parseStmtsUntil(Token::RightBrace, parent);
    parse(Token::RightBrace);
    return llvm::make_unique<WhileStmt>(std::move(condition), std::move(body));
}

/// for-stmt ::= 'for' '(' id 'in' expr ')' '{' stmt* '}'
std::unique_ptr<Stmt> parseForStmt(Decl* parent) {
    ASSERT(currentToken() == Token::For);
    auto location = getCurrentLocation();
    consumeToken();
    parse(Token::LeftParen);
    auto variable = parseVarDecl(false, parent);
    parse(Token::In);
    auto range = parseExpr();
    parse(Token::RightParen);
    parse(Token::LeftBrace);
    auto body = parseStmtsUntil(Token::RightBrace, parent);
    parse(Token::RightBrace);
    return ForStmt(std::move(variable), std::move(range), std::move(body), location).lower();
}

/// switch-stmt ::= 'switch' '(' expr ')' '{' cases default-case? '}'
/// cases ::= case | case cases
/// case ::= 'case' expr ':' stmt+
/// default-case ::= 'default' ':' stmt+
std::unique_ptr<SwitchStmt> parseSwitchStmt(Decl* parent) {
    ASSERT(currentToken() == Token::Switch);
    consumeToken();
    parse(Token::LeftParen);
    auto condition = parseExpr();
    parse(Token::RightParen);
    parse(Token::LeftBrace);
    std::vector<SwitchCase> cases;
    std::vector<std::unique_ptr<Stmt>> defaultStmts;
    bool defaultSeen = false;
    while (true) {
        if (currentToken() == Token::Case) {
            consumeToken();
            auto value = parseExpr();
            parse(Token::Colon);
            auto stmts = parseStmtsUntilOneOf(Token::Case, Token::Default, Token::RightBrace, parent);
            cases.push_back({ std::move(value), std::move(stmts) });
        } else if (currentToken() == Token::Default) {
            if (defaultSeen)
                error(getCurrentLocation(), "switch-statement may only contain one 'default' case");
            consumeToken();
            parse(Token::Colon);
            defaultStmts = parseStmtsUntilOneOf(Token::Case, Token::Default, Token::RightBrace, parent);
            defaultSeen = true;
        } else {
            error(getCurrentLocation(), "expected 'case' or 'default'");
        }
        if (currentToken() == Token::RightBrace) break;
    }
    consumeToken();
    return llvm::make_unique<SwitchStmt>(std::move(condition), std::move(cases),
                                         std::move(defaultStmts));
}

/// break-stmt ::= 'break' ('\n' | ';')
std::unique_ptr<BreakStmt> parseBreakStmt() {
    auto location = getCurrentLocation();
    consumeToken();
    parseStmtTerminator();
    return llvm::make_unique<BreakStmt>(location);
}

/// stmt ::= var-stmt | assign-stmt | compound-assign-stmt | return-stmt |
///          inc-stmt | dec-stmt | call-stmt | defer-stmt |
///          if-stmt | switch-stmt | while-stmt | for-stmt | break-stmt
std::unique_ptr<Stmt> parseStmt(Decl* parent) {
    switch (currentToken()) {
        case Token::Return: return parseReturnStmt();
        case Token::Let: case Token::Var: return parseVarStmt(parent);
        case Token::Defer: return parseDeferStmt();
        case Token::If: return parseIfStmt(parent);
        case Token::While: return parseWhileStmt(parent);
        case Token::For: return parseForStmt(parent);
        case Token::Switch: return parseSwitchStmt(parent);
        case Token::Break: return parseBreakStmt();
        case Token::Underscore: {
            consumeToken();
            parse(Token::Assignment);
            auto stmt = llvm::make_unique<ExprStmt>(parseExpr());
            parseStmtTerminator();
            return std::move(stmt);
        }
        default: break;
    }

    // If we're here, the statement starts with an expression.
    std::unique_ptr<Expr> expr = parseExpr();

    switch (currentToken()) {
        case Token::Increment: return parseIncrementStmt(std::move(expr));
        case Token::Decrement: return parseDecrementStmt(std::move(expr));
        case Token::Assignment: return parseAssignStmt(std::move(expr));
        default:
            if (currentToken().isCompoundAssignmentOperator()) {
                return parseCompoundAssignStmt(std::move(expr));
            }

            if (!expr->isCallExpr()) unexpectedToken(currentToken());
            return parseCallStmt(std::move(expr));
    }
}

std::vector<std::unique_ptr<Stmt>> parseStmtsUntil(Token end, Decl* parent) {
    std::vector<std::unique_ptr<Stmt>> stmts;
    while (currentToken() != end)
        stmts.emplace_back(parseStmt(parent));
    return stmts;
}

std::vector<std::unique_ptr<Stmt>> parseStmtsUntilOneOf(Token end1, Token end2, Token end3, Decl* parent) {
    std::vector<std::unique_ptr<Stmt>> stmts;
    while (currentToken() != end1 && currentToken() != end2  && currentToken() != end3)
        stmts.emplace_back(parseStmt(parent));
    return stmts;
}

/// param-decl ::= id ':' type
ParamDecl parseParam(bool withType) {
    auto name = parse(Token::Identifier);
    Type type;

    if (withType) {
        parse(Token::Colon);
        type = parseType();
    }

    return ParamDecl(std::move(type), std::move(name.getString()), name.getLocation());
}

/// param-list ::= '(' params ')'
/// params ::= '' | non-empty-params
/// non-empty-params ::= param-decl | param-decl ',' non-empty-params
std::vector<ParamDecl> parseParamList(bool* isVariadic, bool withTypes) {
    parse(Token::LeftParen);
    std::vector<ParamDecl> params;
    while (currentToken() != Token::RightParen) {
        if (isVariadic && currentToken() == Token::DotDotDot) {
            consumeToken();
            *isVariadic = true;
            break;
        }
        params.emplace_back(parseParam(withTypes));
        if (currentToken() != Token::RightParen) parse(Token::Comma);
    }
    parse(Token::RightParen);
    return params;
}

void parseGenericParamList(std::vector<GenericParamDecl>& genericParams) {
    parse(Token::Less);
    while (true) {
        auto genericParamName = parse(Token::Identifier);
        genericParams.emplace_back(genericParamName.getString(), genericParamName.getLocation());

        if (currentToken() == Token::Colon) { // Generic type constraint.
            consumeToken();
            genericParams.back().addConstraint(BasicType::get(parse(Token::Identifier).getString(), {}));
            // TODO: Add support for multiple generic type constraints.
        }

        if (currentToken() == Token::Greater) break;
        parse(Token::Comma);
    }
    parse(Token::Greater);
}

/// function-proto ::= 'func' id param-list ('->' type)?
std::unique_ptr<FunctionDecl> parseFunctionProto(bool isExtern, TypeDecl* receiverTypeDecl,
                                                 std::vector<GenericParamDecl>* genericParams) {
    ASSERT(currentToken() == Token::Func);
    consumeToken();

    bool isValidFunctionName =
        currentToken() == Token::Identifier ||
        currentToken().isOverloadable() ||
        (currentToken() == Token::LeftBracket && lookAhead(1) == Token::RightBracket);

    if (!isValidFunctionName)
        unexpectedToken(currentToken(), {}, "as function name");

    SourceLocation nameLocation = getCurrentLocation();
    llvm::StringRef name;
    if (currentToken() == Token::Identifier) {
        name = consumeToken().getString();
    } else if (currentToken() == Token::LeftBracket) {
        consumeToken();
        parse(Token::RightBracket);
        name = "[]";
    } else if (receiverTypeDecl) {
        error(nameLocation, "operator functions other than subscript must be non-member functions");
    } else {
        name = toString(consumeToken().getKind());
    }

    if (currentToken() == Token::Less) {
        parseGenericParamList(*genericParams);
    }

    bool isVariadic = false;
    auto params = parseParamList(isExtern ? &isVariadic : nullptr, true);

    Type returnType = Type::getVoid();
    if (currentToken() == Token::RightArrow) {
        consumeToken();
        returnType = parseType();
        while (currentToken() == Token::Comma) {
            consumeToken();
            returnType.appendType(parseType());
        }
    }

    FunctionProto proto(std::move(name), std::move(params), std::move(returnType), isVariadic, isExtern);

    if (receiverTypeDecl) {
        return llvm::make_unique<MethodDecl>(std::move(proto), *receiverTypeDecl,
                                             std::vector<Type>(), nameLocation);
    } else {
        return llvm::make_unique<FunctionDecl>(std::move(proto), std::vector<Type>(),
                                               *currentModule, nameLocation);
    }
}

/// function-template-proto ::= 'func' id template-param-list param-list ('->' type)?
/// template-param-list ::= '<' template-param-decls '>'
/// template-param-decls ::= id | id ',' template-param-decls
std::unique_ptr<FunctionTemplate> parseFunctionTemplateProto(TypeDecl* receiverTypeDecl) {
    std::vector<GenericParamDecl> genericParams;
    auto decl = parseFunctionProto(false, receiverTypeDecl, &genericParams);
    return llvm::make_unique<FunctionTemplate>(std::move(genericParams), std::move(decl));
}

/// function-decl ::= function-proto '{' stmt* '}'
std::unique_ptr<FunctionDecl> parseFunctionDecl(TypeDecl* receiverTypeDecl, bool requireBody = true) {
    auto decl = parseFunctionProto(false, receiverTypeDecl, nullptr);

    if (requireBody || currentToken() == Token::LeftBrace) {
        parse(Token::LeftBrace);
        decl->setBody(parseStmtsUntil(Token::RightBrace, decl.get()));
        parse(Token::RightBrace);
    }

    if (lookAhead(-1) != Token::RightBrace) {
        parseStmtTerminator();
    }

    return decl;
}

/// function-template-decl ::= function-template-proto '{' stmt* '}'
std::unique_ptr<FunctionTemplate> parseFunctionTemplate(TypeDecl* receiverTypeDecl) {
    auto decl = parseFunctionTemplateProto(receiverTypeDecl);
    parse(Token::LeftBrace);
    decl->getFunctionDecl()->setBody(parseStmtsUntil(Token::RightBrace, decl.get()));
    parse(Token::RightBrace);
    return decl;
}

/// extern-function-decl ::= 'extern' function-proto ('\n' | ';')
std::unique_ptr<FunctionDecl> parseExternFunctionDecl() {
    ASSERT(currentToken() == Token::Extern);
    consumeToken();
    auto decl = parseFunctionProto(true, nullptr, nullptr);
    parseStmtTerminator();
    return decl;
}

/// init-decl ::= 'init' param-list '{' stmt* '}'
std::unique_ptr<InitDecl> parseInitDecl(TypeDecl& receiverTypeDecl) {
    auto initLocation = parse(Token::Init).getLocation();
    auto params = parseParamList(nullptr, true);
    auto decl = llvm::make_unique<InitDecl>(receiverTypeDecl, std::move(params), initLocation);
    parse(Token::LeftBrace);
    decl->setBody(parseStmtsUntil(Token::RightBrace, decl.get()));
    parse(Token::RightBrace);
    return decl;
}

/// deinit-decl ::= 'deinit' '(' ')' '{' stmt* '}'
std::unique_ptr<DeinitDecl> parseDeinitDecl(TypeDecl& receiverTypeDecl) {
    auto deinitLocation = parse(Token::Deinit).getLocation();
    parse(Token::LeftParen);
    auto expectedRParenLocation = getCurrentLocation();
    if (consumeToken() != Token::RightParen) error(expectedRParenLocation, "deinitializers cannot have parameters");
    auto decl = llvm::make_unique<DeinitDecl>(receiverTypeDecl, deinitLocation);
    parse(Token::LeftBrace);
    decl->setBody(parseStmtsUntil(Token::RightBrace, decl.get()));
    parse(Token::RightBrace);
    return decl;
}

/// field-decl ::= ('let' | 'var') id ':' type ('\n' | ';')
FieldDecl parseFieldDecl(TypeDecl& typeDecl) {
    expect({ Token::Let, Token::Var }, "in field declaration");
    bool isMutable = consumeToken() == Token::Var;
    auto name = parse(Token::Identifier);

    parse(Token::Colon);
    auto typeLocation = getCurrentLocation();
    Type type = parseType();
    if (type.isMutable()) error(typeLocation, "type specifier cannot specify mutability");
    type.setMutable(isMutable);

    parseStmtTerminator();
    return FieldDecl(type, std::move(name.getString()), typeDecl, name.getLocation());
}

/// type-template-decl ::= ('class' | 'struct' | 'interface') id generic-param-list? '{' member-decl* '}'
std::unique_ptr<TypeTemplate> parseTypeTemplate() {
    std::vector<GenericParamDecl> genericParams;
    auto typeDecl = parseTypeDecl(&genericParams);
    return llvm::make_unique<TypeTemplate>(std::move(genericParams), std::move(typeDecl));
}

static Token parseTypeHeader(std::vector<Type>& interfaces,
                             std::vector<GenericParamDecl>* genericParams) {
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

/// type-decl ::= ('class' | 'struct' | 'interface') id generic-param-list? interface-list? '{' member-decl* '}'
/// interface-list ::= ':' non-empty-type-list
/// member-decl ::= field-decl | function-decl
std::unique_ptr<TypeDecl> parseTypeDecl(std::vector<GenericParamDecl>* genericParams) {
    TypeTag tag;
    switch (consumeToken()) {
        case Token::Class: tag = TypeTag::Class; break;
        case Token::Struct: tag = TypeTag::Struct; break;
        case Token::Interface: tag = TypeTag::Interface; break;
        default: llvm_unreachable("invalid token");
    }

    std::vector<Type> interfaces;
    auto name = parseTypeHeader(interfaces, genericParams);

    auto typeDecl = llvm::make_unique<TypeDecl>(tag, name.getString(), std::vector<Type>(),
                                                std::move(interfaces), *currentModule,
                                                name.getLocation());
    parse(Token::LeftBrace);

    while (currentToken() != Token::RightBrace) {
        switch (currentToken()) {
            case Token::Mutating:
                consumeToken();
                expect(Token::Func, "after 'mutating'");
                LLVM_FALLTHROUGH;
            case Token::Func: {
                bool isMutating = lookAhead(-1) == Token::Mutating;
                auto requireBody = tag != TypeTag::Interface;

                if (lookAhead(2) == Token::Less) {
                    auto decl = parseFunctionTemplate(typeDecl.get());
                    llvm::cast<MethodDecl>(decl->getFunctionDecl())->setMutating(isMutating);
                    typeDecl->addMethod(std::move(decl));
                } else {
                    auto decl = llvm::cast<MethodDecl>(parseFunctionDecl(typeDecl.get(), requireBody));
                    decl->setMutating(isMutating);
                    typeDecl->addMethod(std::move(decl));
                }
                break;
            }
            case Token::Init:
                typeDecl->addMethod(parseInitDecl(*typeDecl));
                break;
            case Token::Deinit:
                typeDecl->addMethod(parseDeinitDecl(*typeDecl));
                break;
            case Token::Let: case Token::Var:
                typeDecl->addField(parseFieldDecl(*typeDecl));
                break;
            default:
                unexpectedToken(currentToken());
        }
    }

    consumeToken();
    return typeDecl;
}

/// enum-decl ::= 'enum' id generic-param-list? interface-list? '{' enum-case-decl* '}'
/// enum-case-decl ::= 'case' id ('\n' | ';')
std::unique_ptr<EnumDecl> parseEnumDecl(std::vector<GenericParamDecl>* genericParams) {
    ASSERT(currentToken() == Token::Enum);
    consumeToken();

    std::vector<Type> interfaces;
    auto name = parseTypeHeader(interfaces, genericParams);

    parse(Token::LeftBrace);
    std::vector<EnumCase> cases;
    auto valueCounter = llvm::APSInt::get(0);

    while (currentToken() != Token::RightBrace) {
        parse(Token::Case);
        auto caseName = parse(Token::Identifier);
        auto value = llvm::make_unique<IntLiteralExpr>(valueCounter.getExtValue(), caseName.getLocation());
        cases.emplace_back(caseName.getString(), std::move(value), caseName.getLocation());
        parseStmtTerminator("after enum case");
        ++valueCounter;
    }

    consumeToken();
    return llvm::make_unique<EnumDecl>(name.getString(), std::move(cases), *currentModule,
                                       name.getLocation());
}

/// import-decl ::= 'import' (id | string-literal) ('\n' | ';')
std::unique_ptr<ImportDecl> parseImportDecl() {
    ASSERT(currentToken() == Token::Import);
    consumeToken();

    std::string importTarget;
    auto location = getCurrentLocation();

    if (currentToken() == Token::StringLiteral) {
        importTarget = parseStringLiteral()->getValue();
    } else {
        importTarget = parse({ Token::Identifier, Token::StringLiteral }, "after 'import'").getString();
    }

    parseStmtTerminator("after 'import' declaration");
    return llvm::make_unique<ImportDecl>(std::move(importTarget), *currentModule, location);
}

/// top-level-decl ::= function-decl | extern-function-decl | type-decl | enum-decl | import-decl | var-decl
std::unique_ptr<Decl> parseTopLevelDecl(Module& module) {
    switch (currentToken()) {
        case Token::Func:
            if (lookAhead(2) == Token::Less) {
                auto decl = parseFunctionTemplate(nullptr);
                module.addToSymbolTable(*decl);
                return std::move(decl);
            } else {
                auto decl = parseFunctionDecl(nullptr);
                module.addToSymbolTable(*decl);
                return std::move(decl);
            }
        case Token::Extern: {
            auto decl = parseExternFunctionDecl();
            module.addToSymbolTable(*decl);
            return std::move(decl);
        }
        case Token::Class: case Token::Struct: case Token::Interface: {
            if (lookAhead(2) == Token::Less) {
                auto decl = parseTypeTemplate();
                module.addToSymbolTable(*decl);
                return std::move(decl);
            } else {
                auto decl = parseTypeDecl(nullptr);
                module.addToSymbolTable(*decl);
                return std::move(decl);
            }
        }
        case Token::Enum: {
            if (lookAhead(2) == Token::Less) {
                error(getCurrentLocation(), "generic enums not implemented yet");
            } else {
                auto decl = parseEnumDecl(nullptr);
                module.addToSymbolTable(llvm::cast<EnumDecl>(*decl));
                return std::move(decl);
            }
        }
        case Token::Let: case Token::Var: {
            auto decl = parseVarDecl(true, nullptr);
            module.addToSymbolTable(*decl, true);
            return std::move(decl);
        }
        case Token::Import:
            return parseImportDecl();
        default:
            unexpectedToken(currentToken());
    }
}

void initParser(std::unique_ptr<llvm::MemoryBuffer> input) {
    initLexer(std::move(input));
    tokenBuffer.clear();
    currentTokenIndex = 0;
    tokenBuffer.emplace_back(lex());
}

SourceFile parse(std::unique_ptr<llvm::MemoryBuffer> input, Module& module) {
    std::string identifier = input->getBufferIdentifier();
    initParser(std::move(input));
    std::vector<std::unique_ptr<Decl>> topLevelDecls;
    SourceFile sourceFile(identifier);

    while (currentToken() != Token::None) {
        topLevelDecls.emplace_back(parseTopLevelDecl(module));
    }

    sourceFile.setDecls(std::move(topLevelDecls));
    return sourceFile;
}

}

void delta::parse(llvm::StringRef filePath, Module& module) {
    auto buffer = llvm::MemoryBuffer::getFile(filePath);
    if (!buffer) printErrorAndExit("no such file: '", filePath, "'");

    currentModule = &module;
    module.addSourceFile(::parse(std::move(*buffer), module));
}

std::unique_ptr<Expr> delta::parseExpr(std::unique_ptr<llvm::MemoryBuffer> input,
                                       Module& module) {
    initParser(std::move(input));
    currentModule = &module;
    return ::parseExpr();
}
