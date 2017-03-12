%{
    #include <iostream>
    #include <vector>
    #include "../ast/expr.h"
    #include "../ast/decl.h"
    #include "../sema/typecheck.h"

    struct YYLTYPE;

    int yyerror(const char*);

    namespace delta {

    #define yylex lex
    int lex();

    /// The root of the abstract syntax tree, for passing from Bison to compiler.
    extern std::vector<std::unique_ptr<Decl>> globalAST;

    /// Shorthand to avoid overly long lines.
    template<typename T>
    std::unique_ptr<T> u(T* ptr) { return std::unique_ptr<T>(ptr); }

    SrcLoc loc(YYLTYPE);

    }

    #pragma GCC diagnostic ignored "-Wunused-function"
%}

%glr-parser
%error-verbose

// Keywords
%token BREAK    "break"
%token CASE     "case"
%token CAST     "cast"
%token CLASS    "class"
%token CONST    "const"
%token DEFAULT  "default"
%token DEFER    "defer"
%token DEINIT   "deinit"
%token ELSE     "else"
%token EXTERN   "extern"
%token FALSE    "false"
%token FUNC     "func"
%token IF       "if"
%token IMPORT   "import"
%token INIT     "init"
%token MUTABLE  "mutable"
%token NULL_LITERAL "null"
%token RETURN   "return"
%token STRUCT   "struct"
%token SWITCH   "switch"
%token THIS     "this"
%token TRUE     "true"
%token UNINITIALIZED "uninitialized"
%token VAR      "var"
%token WHILE    "while"
%token UNDERSCORE "_"

// Operators
%token EQ       "=="
%token NE       "!="
%token LT       "<"
%token LE       "<="
%token GT       ">"
%token GE       ">="
%token PLUS     "+"
%token MINUS    "-"
%token STAR     "*"
%token SLASH    "/"
%token INCREMENT"++"
%token DECREMENT"--"
%token NOT      "!"
%token AND      "&"
%token AND_AND  "&&"
%token OR       "|"
%token OR_OR    "||"
%token XOR      "^"
%token COMPL    "~"
%token LSHIFT   "<<"
%token RSHIFT   ">>"
%token PLUS_EQ  "+="
%token MINUS_EQ "-="
%token STAR_EQ  "*="
%token SLASH_EQ "/="
%token AND_EQ   "&="
%token AND_AND_EQ"&&="
%token OR_EQ    "|="
%token OR_OR_EQ "||="
%token XOR_EQ   "^="
%token LSHIFT_EQ"<<="
%token RSHIFT_EQ">>="

// Miscellaneous
%token ASSIGN   "="
%token LPAREN   "("
%token RPAREN   ")"
%token LBRACKET "["
%token RBRACKET "]"
%token LBRACE   "{"
%token RBRACE   "}"
%token DOT      "."
%token COMMA    ","
%token COLON    ":"
%token COLON_COLON "::"
%token SEMICOLON";"
%token RARROW   "->"

// Precedence and associativity
%nonassoc "=="
%left "+" "-"
%left "*" "/"
%left "&"
%left "[" "]" "."

// Types
%token <string> IDENTIFIER STRING_LITERAL
%token <number> NUMBER
%type <expr> expression prefix_expression binary_expression parenthesized_expression
             call_expression cast_expression member_access_expression subscript_expression
             assignment_lhs_expression array_literal
%type <declList> declaration_list
%type <decl> declaration function_definition initializer_definition
             deinitializer_definition function_prototype member_function_prototype
             generic_function_prototype extern_function_declaration variable_definition
             immutable_variable_definition mutable_variable_definition
             typed_variable_definition composite_type_declaration import_declaration
%type <stmtList> else_body statement_list nonempty_statement_list
%type <stmt> statement return_statement increment_statement decrement_statement
             call_statement defer_statement if_statement switch_statement while_statement
             break_statement assignment_statement compound_assignment_statement
%type <exprList> expression_list nonempty_expression_list
%type <argList> argument_list nonempty_argument_list
%type <arg> argument
%type <paramDeclList> parameter_list nonempty_parameter_list
%type <paramDecl> parameter
%type <genericParamDeclList> generic_parameter_list
%type <genericParamDecl> generic_parameter
%type <fieldDeclList> member_list
%type <fieldDecl> field_declaration
%type <typeList> generic_argument_list
%type <type> type return_type_specifier return_type_list
%type <caseList> case_list
%type <switchCase> case

%union {
    char* string;
    long long number;
    std::vector<std::unique_ptr<delta::Decl>>* declList;
    std::vector<delta::ParamDecl>* paramDeclList;
    std::vector<delta::GenericParamDecl>* genericParamDeclList;
    std::vector<delta::FieldDecl>* fieldDeclList;
    std::vector<std::unique_ptr<delta::Stmt>>* stmtList;
    std::vector<std::unique_ptr<delta::Expr>>* exprList;
    std::vector<delta::Type>* typeList;
    std::vector<delta::Arg>* argList;
    std::vector<delta::SwitchCase>* caseList;
    delta::Arg* arg;
    delta::Decl* decl;
    delta::ParamDecl* paramDecl;
    delta::GenericParamDecl* genericParamDecl;
    delta::FieldDecl* fieldDecl;
    delta::Stmt* stmt;
    delta::Expr* expr;
    delta::Type type;
    delta::SwitchCase* switchCase;
};

%initial-action {
    yylloc.first_line = yylloc.last_line = 1;
    yylloc.first_column = yylloc.last_column = 0;
}

%{
    #ifdef YYBISON // To prevent this from being put in the generated header.
    using namespace delta;
    #endif
%}

%%

source_file: declaration_list { std::move($1->begin(), $1->end(), std::back_inserter(globalAST)); };

declaration_list:
    /* empty */ { $$ = new std::vector<std::unique_ptr<Decl>>(); }
|   declaration_list declaration { $$ = $1; $1->emplace_back($2); };

declaration:
    function_definition { $$ = $1; }
|   initializer_definition { $$ = $1; }
|   deinitializer_definition { $$ = $1; }
|   extern_function_declaration { $$ = $1; }
|   composite_type_declaration { $$ = $1; }
|   import_declaration { $$ = $1; }
|   variable_definition { $$ = $1; };

// Declarations ////////////////////////////////////////////////////////////////

function_definition:
    function_prototype "{" statement_list "}"
        { $$ = $1; $$->getFuncDecl().body.reset($3); }
|   member_function_prototype "{" statement_list "}"
        { $$ = $1; $$->getFuncDecl().body.reset($3); }
|   generic_function_prototype "{" statement_list "}"
        { $$ = $1; $$->getGenericFuncDecl().func->body.reset($3); };

function_prototype:
    "func" IDENTIFIER "(" parameter_list ")" return_type_specifier
        { $$ = new FuncDecl($2, std::move(*$4), $6, "", loc(@2));
          addToSymbolTable($$->getFuncDecl()); };

member_function_prototype:
    "func" IDENTIFIER "::" IDENTIFIER "(" parameter_list ")" return_type_specifier
        { $$ = new FuncDecl($4, std::move(*$6), $8, $2, loc(@2));
          addToSymbolTable($$->getFuncDecl()); };

generic_function_prototype:
    "func" IDENTIFIER "<" generic_parameter_list ">" "(" parameter_list ")" return_type_specifier
        { auto p = new FuncDecl($2, std::move(*$7), $9, "", loc(@2));
          $$ = new GenericFuncDecl(std::shared_ptr<FuncDecl>(p), std::move(*$4));
          addToSymbolTable($$->getGenericFuncDecl()); };

extern_function_declaration:
    "extern" function_prototype ";" { $$ = $2; };

return_type_specifier:
    /* empty */ { $$ = Type::getVoid(); }
|   "->" return_type_list { $$ = $2; };

return_type_list:
    type { $$ = $1; }
|   return_type_list "," type { $$ = $1; $$.appendType($3); };

parameter_list:
    /* empty */ { $$ = new std::vector<ParamDecl>(); }
|   nonempty_parameter_list { $$ = $1; };

nonempty_parameter_list:
    parameter { $$ = new std::vector<ParamDecl>(); $$->push_back(std::move(*$1)); }
|   nonempty_parameter_list "," parameter { $$ = $1; $$->push_back(std::move(*$3)); };

parameter:
    type IDENTIFIER { $$ = new ParamDecl("", $1, $2, loc(@2)); }
|   IDENTIFIER ":" type IDENTIFIER { $$ = new ParamDecl($1, $3, $4, loc(@4)); };

generic_parameter_list:
    generic_parameter { $$ = new std::vector<GenericParamDecl>(); $$->push_back(std::move(*$1)); }
|   generic_parameter_list "," generic_parameter { $$ = $1; $$->push_back(std::move(*$3)); };

generic_parameter:
    IDENTIFIER { $$ = new GenericParamDecl($1, loc(@1)); };

statement_list:
    /* empty */ { $$ = new std::vector<std::unique_ptr<Stmt>>(); }
|   statement_list statement { $$ = $1; $1->emplace_back($2); };

nonempty_statement_list:
    statement { $$ = new std::vector<std::unique_ptr<Stmt>>(); $$->emplace_back($1); }
|   nonempty_statement_list statement { $$ = $1; $$->emplace_back($2); };

type:
    IDENTIFIER { $$ = BasicType::get($1); }
|   IDENTIFIER "[" NUMBER "]" { $$ = ArrayType::get(BasicType::get($1), $3); }
|   "mutable" IDENTIFIER { $$ = BasicType::get($2, true); }
|   "mutable" IDENTIFIER "[" NUMBER "]" { $$ = ArrayType::get(BasicType::get($2, true), $4, true); }
|   "mutable" "(" type "&" ")" { $$ = PtrType::get($3, true, true); }
|   type "&" { $$ = PtrType::get($1, true); }
|   "mutable" "(" type "*" ")" { $$ = PtrType::get($3, false, true); }
|   type "*" { $$ = PtrType::get($1, false); };

composite_type_declaration:
    "struct" IDENTIFIER "{" member_list "}" { $$ = new TypeDecl(TypeTag::Struct, $2, std::move(*$4), loc(@2));
                                              addToSymbolTable($$->getTypeDecl()); }
|   "class"  IDENTIFIER "{" member_list "}" { $$ = new TypeDecl(TypeTag::Class, $2, std::move(*$4), loc(@2));
                                              addToSymbolTable($$->getTypeDecl()); };

member_list:
    /* empty */ { $$ = new std::vector<FieldDecl>(); }
|   member_list field_declaration { $$ = $1; $$->push_back(std::move(*$2)); };

field_declaration:
    type IDENTIFIER ";" { $$ = new FieldDecl($1, $2, loc(@2)); };

initializer_definition:
    IDENTIFIER "::" "init" "(" parameter_list ")" "{" statement_list "}"
        { $$ = new InitDecl($1, std::move(*$5), u($8), loc(@3));
          addToSymbolTable($$->getInitDecl()); };

deinitializer_definition:
    IDENTIFIER "::" "deinit" "(" ")" "{" statement_list "}"
        { $$ = new DeinitDecl($1, u($7), loc(@3));
          addToSymbolTable($$->getDeinitDecl()); };

import_declaration:
    "import" STRING_LITERAL ";" { $$ = new ImportDecl($2, loc(@2)); };

// Statements //////////////////////////////////////////////////////////////////

statement:
    variable_definition { $$ = new VariableStmt(&$1->getVarDecl()); }
|   assignment_statement{ $$ = $1; }
|   compound_assignment_statement { $$ = $1; }
|   return_statement    { $$ = $1; }
|   increment_statement { $$ = $1; }
|   decrement_statement { $$ = $1; }
|   call_statement      { $$ = $1; }
|   defer_statement     { $$ = $1; }
|   if_statement        { $$ = $1; }
|   switch_statement    { $$ = $1; }
|   while_statement     { $$ = $1; }
|   break_statement     { $$ = $1; };

variable_definition:
    immutable_variable_definition { $$ = $1; }
|   mutable_variable_definition   { $$ = $1; }
|   typed_variable_definition     { $$ = $1; };

immutable_variable_definition:
    "const" IDENTIFIER "=" expression ";"
        { $$ = new VarDecl(false, $2, std::shared_ptr<Expr>($4), loc(@2)); };

mutable_variable_definition:
    "var"   IDENTIFIER "=" expression ";"
        { $$ = new VarDecl(true, $2, std::shared_ptr<Expr>($4), loc(@2)); };

typed_variable_definition:
    type    IDENTIFIER "=" expression ";"
        { $$ = new VarDecl(Type($1), $2, std::shared_ptr<Expr>($4), loc(@2)); }
|   type    IDENTIFIER "=" "uninitialized" ";"
        { $$ = new VarDecl(Type($1), $2, nullptr, loc(@2)); };

assignment_statement:
    assignment_lhs_expression "=" expression ";"
        { $$ = new AssignStmt(u($1), u($3), loc(@2)); }
|   "_" "=" expression ";"
        { $$ = new ExprStmt(u($3)); };

return_statement:
    "return" expression_list ";" { $$ = new ReturnStmt(std::move(*$2), loc(@1)); };

expression_list:
    /* empty */ { $$ = new std::vector<std::unique_ptr<Expr>>(); }
|   nonempty_expression_list { $$ = $1; };

nonempty_expression_list:
    expression { $$ = new std::vector<std::unique_ptr<Expr>>(); $$->emplace_back($1); }
|   nonempty_expression_list "," expression { $$ = $1; $$->emplace_back($3); };

increment_statement: expression "++" ";" { $$ = new IncrementStmt(u($1), loc(@2)); };

decrement_statement: expression "--" ";" { $$ = new DecrementStmt(u($1), loc(@2)); };

call_statement:
    call_expression ";" { $$ = new ExprStmt(u($1)); };

defer_statement:
    "defer" call_expression ";" { $$ = new DeferStmt(u($2)); };

if_statement:
    "if" "(" expression ")" "{" statement_list "}"
        { $$ = new IfStmt(u($3), std::move(*$6), {}); }
|   "if" "(" expression ")" "{" statement_list "}" "else" else_body
        { $$ = new IfStmt(u($3), std::move(*$6), std::move(*$9)); };

else_body:
    if_statement { $$ = new std::vector<std::unique_ptr<Stmt>>(); $$->emplace_back($1); }
|   "{" statement_list "}" { $$ = $2; };

switch_statement:
    "switch" "(" expression ")" "{" case_list "}"
        { $$ = new SwitchStmt(u($3), std::move(*$6), {}); }
|   "switch" "(" expression ")" "{" case_list "default" ":" statement_list "}"
        { $$ = new SwitchStmt(u($3), std::move(*$6), std::move(*$9)); };

case_list:
    case { $$ = new std::vector<SwitchCase>(); $$->push_back(std::move(*$1)); }
|   case_list case { $$ = $1; $$->push_back(std::move(*$2)); };

case:
    "case" expression ":" nonempty_statement_list { $$ = new SwitchCase{u($2), std::move(*$4)}; };

while_statement:
    "while" "(" expression ")" "{" statement_list "}"
        { $$ = new WhileStmt(u($3), std::move(*$6)); };

break_statement:
    "break" ";" { $$ = new BreakStmt(loc(@1)); };

// Expressions /////////////////////////////////////////////////////////////////

expression:
    STRING_LITERAL { $$ = new StrLiteralExpr($1, loc(@1)); }
|   NUMBER { $$ = new IntLiteralExpr($1, loc(@1)); }
|   TRUE { $$ = new BoolLiteralExpr(true, loc(@1)); }
|   FALSE { $$ = new BoolLiteralExpr(false, loc(@1)); }
|   "null" { $$ = new NullLiteralExpr(loc(@1)); }
|   "this" { $$ = new VariableExpr("this", loc(@1)); }
|   array_literal { $$ = $1; }
|   binary_expression { $$ = $1; }
|   assignment_lhs_expression { $$ = $1; };

assignment_lhs_expression:
    IDENTIFIER { $$ = new VariableExpr($1, loc(@1)); }
|   prefix_expression { $$ = $1; }
|   parenthesized_expression { $$ = $1; }
|   call_expression { $$ = $1; }
|   cast_expression { $$ = $1; }
|   member_access_expression { $$ = $1; }
|   subscript_expression { $$ = $1; };

prefix_expression: "+" expression { $$ = new PrefixExpr(PLUS, u($2), loc(@1)); };
prefix_expression: "-" expression { $$ = new PrefixExpr(MINUS, u($2), loc(@1)); };
prefix_expression: "*" expression { $$ = new PrefixExpr(STAR, u($2), loc(@1)); };
prefix_expression: "&" expression { $$ = new PrefixExpr(AND, u($2), loc(@1)); };
prefix_expression: "!" expression { $$ = new PrefixExpr(NOT, u($2), loc(@1)); };
prefix_expression: "~" expression { $$ = new PrefixExpr(COMPL, u($2), loc(@1)); };
binary_expression: expression "==" expression { $$ = new BinaryExpr(EQ, u($1), u($3), loc(@2)); };
binary_expression: expression "!=" expression { $$ = new BinaryExpr(NE, u($1), u($3), loc(@2)); };
binary_expression: expression "<"  expression { $$ = new BinaryExpr(LT, u($1), u($3), loc(@2)); };
binary_expression: expression "<=" expression { $$ = new BinaryExpr(LE, u($1), u($3), loc(@2)); };
binary_expression: expression ">"  expression { $$ = new BinaryExpr(GT, u($1), u($3), loc(@2)); };
binary_expression: expression ">=" expression { $$ = new BinaryExpr(GE, u($1), u($3), loc(@2)); };
binary_expression: expression "+"  expression { $$ = new BinaryExpr(PLUS, u($1), u($3), loc(@2)); };
binary_expression: expression "-"  expression { $$ = new BinaryExpr(MINUS, u($1), u($3), loc(@2)); };
binary_expression: expression "*"  expression { $$ = new BinaryExpr(STAR, u($1), u($3), loc(@2)); };
binary_expression: expression "/"  expression { $$ = new BinaryExpr(SLASH, u($1), u($3), loc(@2)); };
binary_expression: expression "&"  expression { $$ = new BinaryExpr(AND, u($1), u($3), loc(@2)); };
binary_expression: expression "&&" expression { $$ = new BinaryExpr(AND_AND, u($1), u($3), loc(@2)); };
binary_expression: expression "|"  expression { $$ = new BinaryExpr(OR, u($1), u($3), loc(@2)); };
binary_expression: expression "||" expression { $$ = new BinaryExpr(OR_OR, u($1), u($3), loc(@2)); };
binary_expression: expression "^"  expression { $$ = new BinaryExpr(XOR, u($1), u($3), loc(@2)); };
binary_expression: expression "<<" expression { $$ = new BinaryExpr(LSHIFT, u($1), u($3), loc(@2)); };
binary_expression: expression ">>" expression { $$ = new BinaryExpr(RSHIFT, u($1), u($3), loc(@2)); };

compound_assignment_statement:
    assignment_lhs_expression "+="  expression ";" { $$ = new AugAssignStmt(u($1), u($3), PLUS, loc(@2)); }
|   assignment_lhs_expression "-="  expression ";" { $$ = new AugAssignStmt(u($1), u($3), MINUS, loc(@2)); }
|   assignment_lhs_expression "*="  expression ";" { $$ = new AugAssignStmt(u($1), u($3), STAR, loc(@2)); }
|   assignment_lhs_expression "/="  expression ";" { $$ = new AugAssignStmt(u($1), u($3), SLASH, loc(@2)); }
|   assignment_lhs_expression "&="  expression ";" { $$ = new AugAssignStmt(u($1), u($3), AND, loc(@2)); }
|   assignment_lhs_expression "&&=" expression ";" { $$ = new AugAssignStmt(u($1), u($3), AND_AND, loc(@2)); }
|   assignment_lhs_expression "|="  expression ";" { $$ = new AugAssignStmt(u($1), u($3), OR, loc(@2)); }
|   assignment_lhs_expression "||=" expression ";" { $$ = new AugAssignStmt(u($1), u($3), OR_OR, loc(@2)); }
|   assignment_lhs_expression "^="  expression ";" { $$ = new AugAssignStmt(u($1), u($3), XOR, loc(@2)); }
|   assignment_lhs_expression "<<=" expression ";" { $$ = new AugAssignStmt(u($1), u($3), LSHIFT, loc(@2)); }
|   assignment_lhs_expression ">>=" expression ";" { $$ = new AugAssignStmt(u($1), u($3), RSHIFT, loc(@2)); };

array_literal: "[" expression_list "]" { $$ = new ArrayLiteralExpr(std::move(*$2), loc(@1)); }

parenthesized_expression: "(" expression ")" { $$ = $2; };

call_expression:
    IDENTIFIER "(" argument_list ")"
        { $$ = new CallExpr($1, std::move(*$3), false, nullptr, {}, loc(@1)); }
|   expression "." IDENTIFIER "(" argument_list ")"
        { $$ = new CallExpr($3, std::move(*$5), false, u($1), {}, loc(@1)); }
|   IDENTIFIER "<" generic_argument_list ">" "(" argument_list ")"
        { $$ = new CallExpr($1, std::move(*$6), false, nullptr, std::move(*$3), loc(@1)); };

cast_expression:
    "cast" "<" type ">" "(" expression ")" { $$ = new CastExpr($3, u($6), loc(@1)); };

argument_list:
    /* empty */ { $$ = new std::vector<Arg>(); }
|   nonempty_argument_list { $$ = $1; };

nonempty_argument_list:
    argument { $$ = new std::vector<Arg>(); $$->push_back(std::move(*$1)); }
|   nonempty_argument_list "," argument { $$ = $1; $$->push_back(std::move(*$3)); };

argument:
    expression { $$ = new Arg{"", u($1), loc(@1)};  }
|   IDENTIFIER ":" expression { $$ = new Arg{$1, u($3), loc(@1)}; };

generic_argument_list:
    type { $$ = new std::vector<Type>(); $$->emplace_back($1); }
|   generic_argument_list "," type { $$ = $1; $$->emplace_back($3); };

member_access_expression:
    expression "." IDENTIFIER { $$ = new MemberExpr(u($1), $3, loc(@3)); };

subscript_expression:
    expression "[" expression "]" { $$ = new SubscriptExpr(u($1), u($3), loc(@2)); };

%%

int yyerror(const char* message) {
    std::cout << "error: " << message << '\n';
    return 1;
}

std::vector<std::unique_ptr<Decl>> delta::globalAST;

#include "lexer.cpp"
#include "operators.cpp"

SrcLoc delta::loc(YYLTYPE loc) { return SrcLoc(currentFileName, loc.first_line, loc.first_column); }
