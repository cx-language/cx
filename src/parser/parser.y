%{
    #include <iostream>

    int yylex();

    int yyerror(const char* message) {
        std::cout << "error: " << message << '\n';
        return 1;
    }
%}

%error-verbose

// Keywords
%token CONST    "const"
%token ELSE     "else"
%token EXTERN   "extern"
%token FUNC     "func"
%token IF       "if"
%token RETURN   "return"
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

%token IDENTIFIER
%token NUMBER

%union {
    char* string;
    long long number;
};

%%

source_file: /* empty */ | declaration source_file;
declaration: function_definition | variable_definition;

// Declarations
function_definition: "func" IDENTIFIER "(" parameter_list ")"
    return_type_specifier "{" function_body "}";
return_type_specifier: /* empty */ | "->" return_type_list;
return_type_list: IDENTIFIER | IDENTIFIER "," return_type_list;
parameter_list: /* empty */ | nonempty_parameter_list;
nonempty_parameter_list: parameter | parameter "," nonempty_parameter_list;
parameter: IDENTIFIER IDENTIFIER;
function_body: /* empty */ | statement function_body;

// Statements
statement: variable_definition | return_statement | increment_statement | decrement_statement;
variable_definition: immutable_variable_definition | mutable_variable_definition;
immutable_variable_definition: "const" IDENTIFIER "=" expression ";";
mutable_variable_definition: "var" IDENTIFIER "=" expression ";";
return_statement: "return" return_value_list ";";
return_value_list: /* empty */ | nonempty_return_value_list;
nonempty_return_value_list: expression | expression "," nonempty_return_value_list;
increment_statement: expression "++" ";";
decrement_statement: expression "--" ";";

// Expressions
expression: IDENTIFIER | NUMBER | prefix_expression | binary_expression | parenthesized_expression;
prefix_expression: "+" expression;
prefix_expression: "-" expression;
binary_expression: expression "+" expression;
binary_expression: expression "-" expression;
binary_expression: expression "*" expression;
binary_expression: expression "/" expression;
parenthesized_expression: "(" expression ")";

%%
