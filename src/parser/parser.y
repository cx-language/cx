%{
    #include <iostream>
    #include <vector>
    #include "../ast/expr.h"
    #include "../ast/decl.h"

    int yylex();

    int yyerror(const char* message) {
        std::cout << "error: " << message << '\n';
        return 1;
    }

    /// The root of the abstract syntax tree, for passing from Bison to compiler.
    std::vector<Decl> globalAST;

    /// Shorthand to avoid overly long lines.
    template<typename T>
    std::unique_ptr<T> u(T* ptr) { return std::unique_ptr<T>(ptr); }
%}

%error-verbose

// Keywords
%token CONST    "const"
%token ELSE     "else"
%token EXTERN   "extern"
%token FALSE    "false"
%token FUNC     "func"
%token IF       "if"
%token RETURN   "return"
%token TRUE     "true"
%token VAR      "var"

// Operators
%token EQ       "=="
%token NE       "!="
%token LT       "<"
%token LE       "<="
%token GT       ">"
%token GE       ">="
%token PLUS     "+"
%token MINUS    "-"
%token ASTERISK "*"
%token SLASH    "/"
%token INCREMENT"++"
%token DECREMENT"--"
%token AND      "&"
%token AND_AND  "&&"
%token OR       "|"
%token OR_OR    "||"
%token LSHIFT   "<<"
%token RSHIFT   ">>"

// Miscellaneous
%token ASSIGN   "="
%token LPAREN   "("
%token RPAREN   ")"
%token LBRACE   "{"
%token RBRACE   "}"
%token DOT      "."
%token COMMA    ","
%token COLON    ":"
%token SEMICOLON";"
%token RARROW   "->"

%token <string> IDENTIFIER
%token <number> NUMBER
%type <expr> expression prefix_expression binary_expression parenthesized_expression
             call_expression
%type <declList> declaration_list
%type <decl> declaration function_definition variable_definition
             immutable_variable_definition mutable_variable_definition
%type <stmtList> function_body
%type <stmt> statement return_statement increment_statement decrement_statement
             call_statement
%type <exprList> return_value_list nonempty_return_value_list
                 argument_list nonempty_argument_list
%type <paramDeclList> parameter_list nonempty_parameter_list
%type <paramDecl> parameter
%type <type> return_type_specifier return_type_list

%union {
    char* string;
    long long number;
    std::vector<Decl>* declList;
    std::vector<ParamDecl>* paramDeclList;
    std::vector<Stmt>* stmtList;
    std::vector<Expr>* exprList;
    Decl* decl;
    ParamDecl* paramDecl;
    Stmt* stmt;
    Expr* expr;
    Type* type;
};

%%

source_file: declaration_list { globalAST = std::move(*$1); };

declaration_list:
    /* empty */ { $$ = new std::vector<Decl>(); }
|   declaration_list declaration { $$ = $1; $1->push_back(std::move(*$2)); };

declaration:
    function_definition { $$ = $1; }
|   variable_definition { $$ = $1; };

// Declarations ////////////////////////////////////////////////////////////////

function_definition:
    "func" IDENTIFIER "(" parameter_list ")" return_type_specifier "{" function_body "}"
        { $$ = new Decl(FuncDecl{$2, std::move(*$4), std::move(*$6), std::move(*$8)}); };

return_type_specifier:
    /* empty */ { $$ = new Type("void"); }
|   "->" return_type_list { $$ = $2; };

return_type_list:
    IDENTIFIER { $$ = new Type($1); }
|   return_type_list "," IDENTIFIER { $$ = $1; $$->appendType($3); };

parameter_list:
    /* empty */ { $$ = new std::vector<ParamDecl>(); }
|   nonempty_parameter_list { $$ = $1; };

nonempty_parameter_list:
    parameter { $$ = new std::vector<ParamDecl>(); $$->push_back(std::move(*$1)); }
|   nonempty_parameter_list "," parameter { $$ = $1; $$->push_back(std::move(*$3)); };

parameter: IDENTIFIER IDENTIFIER { $$ = new ParamDecl{$1, $2}; };

function_body:
    /* empty */ { $$ = new std::vector<Stmt>(); }
|   function_body statement { $$ = $1; $1->push_back(std::move(*$2)); };

// Statements //////////////////////////////////////////////////////////////////

statement:
    variable_definition { $$ = new Stmt(VariableStmt{&$1->getVarDecl()}); }
|   return_statement    { $$ = $1; }
|   increment_statement { $$ = $1; }
|   decrement_statement { $$ = $1; }
|   call_statement      { $$ = $1; };

variable_definition:
    immutable_variable_definition { $$ = $1; }
|   mutable_variable_definition   { $$ = $1; };

immutable_variable_definition:
    "const" IDENTIFIER "=" expression ";" { $$ = new Decl(VarDecl{$2, std::move(*$4)}); };

mutable_variable_definition:
    "var"   IDENTIFIER "=" expression ";" { $$ = new Decl(VarDecl{$2, std::move(*$4)}); };

return_statement:
    "return" return_value_list ";" { $$ = new Stmt(ReturnStmt{std::move(*$2)}); };

return_value_list:
    /* empty */ { $$ = new std::vector<Expr>(); }
|   nonempty_return_value_list { $$ = $1; };

nonempty_return_value_list:
    expression { $$ = new std::vector<Expr>(); $$->push_back(std::move(*$1)); }
|   nonempty_return_value_list "," expression { $$ = $1; $$->push_back(std::move(*$3)); };

increment_statement: expression "++" ";" { $$ = new Stmt(IncrementStmt{std::move(*$1)}); };

decrement_statement: expression "--" ";" { $$ = new Stmt(DecrementStmt{std::move(*$1)}); };

call_statement:
    call_expression ";" { $$ = new Stmt(CallStmt{std::move($1->getCallExpr())}); };

// Expressions /////////////////////////////////////////////////////////////////

expression:
    IDENTIFIER { $$ = new Expr(VariableExpr{$1}); }
|   NUMBER { $$ = new Expr(IntLiteralExpr{$1}); }
|   TRUE { $$ = new Expr(BoolLiteralExpr{true}); }
|   FALSE { $$ = new Expr(BoolLiteralExpr{false}); }
|   prefix_expression { $$ = $1; }
|   binary_expression { $$ = $1; }
|   parenthesized_expression { $$ = $1; }
|   call_expression { $$ = $1; };

prefix_expression: "+" expression { $$ = new Expr(PrefixExpr{'+', u($2)}); };

prefix_expression: "-" expression { $$ = new Expr(PrefixExpr{'-', u($2)}); };

binary_expression:
    expression "+" expression { $$ = new Expr(BinaryExpr{'+', u($1), u($3)}); };

binary_expression:
    expression "-" expression { $$ = new Expr(BinaryExpr{'-', u($1), u($3)}); };

binary_expression:
    expression "*" expression { $$ = new Expr(BinaryExpr{'*', u($1), u($3)}); };

binary_expression:
    expression "/" expression { $$ = new Expr(BinaryExpr{'/', u($1), u($3)}); };

parenthesized_expression: "(" expression ")" { $$ = $2; };

call_expression:
    IDENTIFIER "(" argument_list ")" { $$ = new Expr(CallExpr{$1, std::move(*$3)}); };

argument_list:
    /* empty */ { $$ = new std::vector<Expr>(); }
|   nonempty_argument_list { $$ = $1; };

nonempty_argument_list:
    expression { $$ = new std::vector<Expr>(); $$->push_back(std::move(*$1)); }
|   nonempty_argument_list "," expression { $$ = $1; $$->push_back(std::move(*$3)); };

%%
