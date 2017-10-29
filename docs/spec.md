# Delta Language Specification

This document describes the syntax and semantics of the Delta programming
language.

__Note:__ This document is incomplete.

## Lexical structure

### Keywords

The following keywords are reserved and can't be used as identifiers.

    break
    case
    cast
    catch
    class
    const
    continue
    default
    defer
    deinit
    do
    else
    enum
    extern
    fallthrough
    false
    for
    func
    if
    import
    in
    init
    inout
    interface
    let
    move
    mutable
    mutating
    null
    private
    public
    return
    static
    struct
    switch
    this
    throw
    throws
    true
    try
    typealias
    undefined
    var
    while
    _

The keywords `class` and `struct` can be used as variable names but not as a
type names.

### Operators and delimiters

Binary arithmetic operators:

    +   -   *   /   **  %   &   &&  |   ||  ^   <<  >>
    +=  -=  *=  /=  **= %=  &=  &&= |=  ||= ^=  <<= >>=

Binary comparison operators:

    ==  !=  <   >   <=  >=

Miscellaneous binary operators:

    =   ~=  ..  ...

Unary prefix operators:

    +   -   *   &   !   ~

Unary postfix operators:

    ++  --  !

Delimiters:

    (   )
    [   ]
    {   }
    .   ,
    :   ;
    ->

From the above set of operators, the following are overloadable by user code:
`+` (both unary and binary), `-` (both unary and binary), `*`, `/`, `%`, `==`,
`<`.

### Comments

Delta has two kinds of comments:

- Line comments that start with `//` and continue until the end of the line.
- Block comments that start with `/*` and end with `*/`. Block comments can be
  nested.

### Literals

#### Integer literal

> _binary-digit_ → `0` | `1`<br>
> _octal-digit_ → `0` | `1` | `2` | `3` | `4` | `5` | `6` | `7`<br>
> _decimal-digit_ → `0` | `1` | `2` | `3` | `4` | `5` | `6` | `7` | `8` | `9`<br>
> _nonzero-decimal-digit_ → `1` | `2` | `3` | `4` | `5` | `6` | `7` | `8` | `9`<br>
> _lowercase-hex-digit_ → `0` | `1` | `2` | `3` | `4` | `5` | `6` | `7` | `8` | `9` | `a` | `b` | `c` | `d` | `e` | `f`<br>
> _uppercase-hex-digit_ → `0` | `1` | `2` | `3` | `4` | `5` | `6` | `7` | `8` | `9` | `A` | `B` | `C` | `D` | `E` | `F`<br>
>
> _binary-integer-literal_ → `0b` _binary-digit_+<br>
> _octal-integer-literal_ → `0o` _octal-digit_+<br>
> _decimal-integer-literal_ → _nonzero-decimal-digit_ _decimal-digit_* | `0`<br>
> _hex-integer-literal_ → `0x` ( _lowercase-hex-digit_+ | _uppercase-hex-digit_+ )<br>
>
> _integer-literal_ → _binary-integer-literal_ | _octal-integer-literal_ | _decimal-integer-literal_ | _hex-integer-literal_<br>

#### Floating-point literal

Floating-point literals have the form `[1-9][0-9]*\.[0-9]+` or `0\.[0-9]+`:

> _floating-point-literal_ → _nonzero-decimal-digit_ _decimal-digit_* `.` _decimal-digit_+<br>
> _floating-point-literal_ → `0` `.` _decimal-digit_+<br>

#### Boolean literal

> _boolean-literal_ → `true` | `false`<br>

#### Null literal

> _null-literal_ → `null`<br>

#### String literal

> _string-literal_ → `"` (_character_ | _interpolated-expression_)* `"`<br>
> _interpolated-expression_ → `${` _expression_ `}`<br>

#### Array literal

> _array-literal_ → `[` _elements_ `]`<br>

where _elements_ is a comma-separated list of zero or more expressions of the
same type.

#### Object literal

> _object-literal_ → `(` _elements_ `)`<br>

where _elements_ is a comma-separated list of zero or more
_object-literal-elements_:

> _object-literal-element_ → _identifier_ `:` _expression_<br>
> _object-literal-element_ → _identifier_<br>

The second form is a shorthand for object-literal-elements of the form `x: x`.

Initializing a non-type-annotated variable with an object-literal causes the
variable's type to be the tuple type corresponding to the object-literal.

### Identifiers

> _identifier-head_ → upper- or lowercase letter `A` through `Z`<br>
> _identifier-head_ → `_`<br>
> _identifier-head_ → U+00A8 | U+00AA | U+00AD | U+00AF | U+00B2–U+00B5 | U+00B7–U+00BA<br>
> _identifier-head_ → U+00BC–U+00BE | U+00C0–U+00D6 | U+00D8–U+00F6 | U+00F8–U+00FF<br>
> _identifier-head_ → U+0100–U+02FF | U+0370–U+167F | U+1681–U+180D | U+180F–U+1DBF<br>
> _identifier-head_ → U+1E00–U+1FFF<br>
> _identifier-head_ → U+200B–U+200D | U+202A–U+202E | U+203F–U+2040 | U+2054 | U+2060–U+206F<br>
> _identifier-head_ → U+2070–U+20CF | U+2100–U+218F | U+2460–U+24FF | U+2776–U+2793<br>
> _identifier-head_ → U+2C00–U+2DFF | U+2E80–U+2FFF<br>
> _identifier-head_ → U+3004–U+3007 | U+3021–U+302F | U+3031–U+303F | U+3040–U+D7FF<br>
> _identifier-head_ → U+F900–U+FD3D | U+FD40–U+FDCF | U+FDF0–U+FE1F | U+FE30–U+FE44<br>
> _identifier-head_ → U+FE47–U+FFFD<br>
> _identifier-head_ → U+10000–U+1FFFD | U+20000–U+2FFFD | U+30000–U+3FFFD | U+40000–U+4FFFD<br>
> _identifier-head_ → U+50000–U+5FFFD | U+60000–U+6FFFD | U+70000–U+7FFFD | U+80000–U+8FFFD<br>
> _identifier-head_ → U+90000–U+9FFFD | U+A0000–U+AFFFD | U+B0000–U+BFFFD | U+C0000–U+CFFFD<br>
> _identifier-head_ → U+D0000–U+DFFFD | U+E0000–U+EFFFD<br>
>
> _identifier-character_ → `0` | `1` | `2` | `3` | `4` | `5` | `6` | `7` | `8` | `9`<br>
> _identifier-character_ → U+0300–U+036F | U+1DC0–U+1DFF | U+20D0–U+20FF | U+FE20–U+FE2F<br>
> _identifier-character_ → _identifier-head­_<br>
>
> _identifier_ → _identifier-head_ _identifier-character_*<br>

## Types

### Integer types

There are eight built-in integer types: `int8`, `int16`, `int32`, `int64`, and
their unsigned counterparts `uint8`, `uint16`, `uint32`, `uint64`. The standard
library also provides `int` and `uint` which are type aliases for `int32` and
`uint32`, respectively.

### Floating-point types

There are three built-in floating-point types: `float32`, `float64`, and
`float80`. The standard library also provides `float` and `double` which are
type aliases for `float32` and `float64`, respectively.

### Pointer types

Pointers are values that point to other values. They can be reassigned to point
to another value (if the pointer type itself is declared as `mutable`), but they
must always refer to some value, i.e. they cannot be null by default (nullable
pointers can be created using the optional type, see below). Member access,
member function calls, and subscript operations via pointers are allowed: they
will be forwarded to the pointee value.

> _pointer-type_ → _pointee-type_ `*`<br>
> _pointer-type_ → `mutable` _pointee-type_ `*`<br>
> _pointer-type_ → `mutable` `(` _pointee-type_ `*` `)`<br>
> _pointer-type_ → `mutable` `(` `mutable` _pointee-type_ `*` `)`<br>

Prefixing the _pointee-type_ with `mutable` makes the _pointee-type_ mutable.
Enclosing the _pointer-type_ in parentheses and prefixing the parentheses with
`mutable` makes the _pointer-type_ itself mutable.

Pointer arithmetic is supported in the form of the following operations:

- `pointer + integer`
- `pointer - integer`
- `pointer++`
- `pointer--`
- `pointer - pointer`

### Array types

> _sized-array-type_ → _element-type_ `[` _size_ `]`<br>
> _unsized-array-type_ → _element-type_ `[` `]`<br>

_sized-array-types_ are contiguous blocks of _size_ elements of type
_element-type_. _unsized-array-types_ may only be used as return types and
parameters types, and are implemented as pointer-and-size pairs under the hood,
unless the compiler detects that the size is unused, in which case it will be
optimized out.

### String type

The type `string` holds sequences of Unicode characters.

### Composite types

There are two kinds of composite types: classes and structs.

#### Member variables

Composite types contain a set of member variables. Each member variable may
contain an associated getter and/or setter, that are called when the member
variable is accessed or assigned to, respectively. The syntax of a member
variable definition is as follows:

> _let-or-var_ → `let` | `var`<br>
> _member-variable-declaration_ → _let-or-var_ _member-variable-name_ `:` _type_ `;`<br>
> _member-variable-declaration_ → _let-or-var_ _member-variable-name_ `:` _type_ `{` _getter-setter-definitions_ `}`<br>
> _getter-setter-definitions_ → _getter-definition_ | _setter-definition_ | _getter-definition_ _setter-definition__<br>
> _getter-definition_ → `get` `{` _getter-body_ `}`<br>
> _setter-definition_ → `set` `{` _setter-body_ `}`<br>

#### Class types

Types declared using the `class` keyword are _class types_, also called
_reference types_. By default, they're allocated on the stack. When passed as
functions arguments, they're passed by reference. When class types are returned
from functions, they're moved if they're a local variable, or otherwise returned
by reference.

Sometimes it's useful to pass classes by explicitly moving or copying them. This
can be accomplished by using the `move` keyword, or constructing a new object by
calling the copy constructor.

#### Struct types

Types declared using the `struct` keyword are _struct types_, also called _value
types_. They're always allocated on the stack. When passed as function arguments
and returned from functions, they're passed by copy.

Sometimes it's useful to explicitly pass structs by reference, for example in
order to modify them inside the function, and have the changes affect the actual
value outside the function. This can be accomplished by using the `inout` keyword.

### Interface types

The `interface` keyword declares an interface, i.e. a set of requirements (member
functions and properties). Types that fulfill the requirements of an interface
`I` can be used as values for a variable of type `I`. This enables runtime
polymorphism. Like classes and structs, interfaces may be generic.

### Optional type

> I call it my billion-dollar mistake. It was the invention of the null
> reference in 1965. At that time, I was designing the first comprehensive type
> system for references in an object oriented language (ALGOL W). My goal was to
> ensure that all use of references should be absolutely safe, with checking
> performed automatically by the compiler. But I couldn't resist the temptation
> to put in a null reference, simply because it was so easy to implement. This
> has led to innumerable errors, vulnerabilities, and system crashes, which have
> probably caused a billion dollars of pain and damage in the last forty years.
>
> — C. A. R. Hoare

An object of the optional type `T?` (where `T` is an arbitrary type) may contain
a value of type `T` or the value `null`.

> _optional-type_ → _wrapped-type_ `?`<br>

### Function types

Function types are written out as follows:

> _function-type_ → `(` _parameter-type-list_ `)` `->` _return-type_<br>

where _parameter-type-list_ is a comma-separated list of parameter types.

### Tuple types

> _tuple-type_ → `(`<sub>opt</sub> _tuple-element-list_ `)`<sub>opt</sub><br>
> _tuple-element-list_ → comma-separated list of one or more _tuple-elements_<br>
> _tuple-element_ → _name_ `:` _type_<br>

Tuples behave like structs, but they're defined inline. Tuples are intended as
a lightweight alternative for situations where defining a whole new struct feels
overkill or inappropriate, e.g. returning multiple values from a function. The
optional parentheses may be used for syntax disambiguation.

#### Tuple unpacking

The elements of a tuple value may be unpacked into individual variables as follows:

> _tuple-unpack-statement_ → _variable-list_ `=` _tuple-expression_ `;`<br>

_variable-list_ is a comma-separated list of one or more variable names. The
variable names must match the element names of the _tuple-expression_, and be in
the same order.

???

### Range types

The standard library defines the following two generic structs to represent
range types:

- `Range<T>` for ranges with an exclusive upper bound
- `ClosedRange<T>` for ranges with an inclusive upper bound

## Declarations

### Variables

Variable declarations introduce a new variable into the enclosing scope. The
syntax is as follows:

> _immutable-variable-declaration → `let` _variable-name_ `:` _type_ `;`<br>
> _mutable-variable-declaration → `var` _variable-name_ `:` _type_ `;`<br>
> _immutable-variable-definition_ → `let` _variable-name_ (`:` _type_)<sub>opt</sub> `=` _initializer_ `;`<br>
> _mutable-variable-definition_ → `var` _variable-name_ (`:` _type_)<sub>opt</sub> `=` _initializer_ `;`<br>

In the first two forms, the variable is declared but not initialized. This
allows delayed initialization, which causes the compiler to enforce that the
variable is always initialized properly before its value is accessed.

If _type_ is present, the variable has the specified type. The compiler ensures
that the given _initializer_ is compatible with this type. If no _type_ is
given, the compiler will infer the type of the variable from the _initializer_.
The _initializer_ is an expression that provides the initial value for the
variable.

If _type_ has been specified, _initializer_ may also be the keyword `undefined`,
in which case the variable is not initialized and all use-before-initialization
warnings for the variable will be suppressed. Reading from an uninitialized
variable causes undefined behavior.

### Functions

A function can be defined with either of the following syntaxes:

> `func` _function-name_ `(` _parameter-list_ `)` `{` _function-body_ `}`<br>
> `func` _function-name_ `(` _parameter-list_ `)` `->` _return-type_ `{` _function-body_ `}`<br>

The return type of the first version is `void`. The ___parameter-list___ is a
comma-separated list of parameter declarations. A parameter declaration has one
of the following syntaxes:

> _parameter-name_ `:` _parameter-type_<br>

___parameter-name___ is an identifier specifying the name of the parameter. A
function cannot have multiple parameters with the same name.

___return-type___ defines what kind of values the function can return. This may
be a tuple type to allow the function to return multiple values with a
lightweight syntax.

Parameter and return types may not have a top-level `mutable` modifier.

#### Member functions

Member functions are just like normal functions, except that they receive an
additional parameter (called the "receiver") on the left-hand-side of the
function call, separated by a period:

> _member-function-call_ → _receiver_ `.` _member-function-name_ `(` _argument-list_ `)`<br>

Member functions are defined with the same syntax as non-member functions, but
are written inside a type declaration. That type declaration defines the member
function's receiver type. Inside member functions, the receiver can be accessed
with the keyword `this`.

##### Initializers

Initializers are a special kind of member functions that are used for
initializing newly created objects.

> _initializer-definition_ → `init` `(` _parameter-list_ `)` `{` _body_ `}`<br>

Initializers can be invoked with the following syntax:

> _initializer-call_ → _receiver-type_ `(` _argument-list_ `)`<br>

The _initializer-call_ expression returns a new instance of the specified
_receiver-type_ that has been initialized by calling the initializer function
with a matching parameter list.

##### Deinitializers

Deinitializers are another special kind of member functions. They are
automatically called on objects when they're destroyed. They can be used e.g. to
deallocate resources allocated in an initializer. They are declared as follows:

> _deinitializer-definition_ → `deinit` `(` `)` `{` _body_ `}`<br>

#### Parameters

Unlike C++ and C, Delta is not a copy-by-default language. When you declare a
parameter to be of type `T`, the parameter is not automatically pass-by-value.
Instead, whether `T` is a class or struct will determine how it's passed:
classes are passed by reference (by default), while structs are passed by copy
(by default).

To override the default behavior, the keywords `move` and `inout` may be used:

- `move` can be applied to class parameters, and forces the argument to be a
  pass-by-value temporary, i.e. a freshly copied or moved value.
- `inout` can be applied to struct parameters, and causes any changes to the
  parameter to be reflected at the call site.

#### Private and public functions

Both member functions and global functions may be declared private or public by
prefixing the function definition with the keyword `private` or `public`.
Private functions are only accessible from the file they're declared in. Public
functions are accessible from anywhere, including other modules. Functions not
marked private or public are _module-private_, i.e. only accessible within the
module they're declared in.

### Classes

Classes are defined as follows:

> _class-definition_ → `class` _class-name_ `{` _member-list_ `}`<br>

_class-name_ becomes the name of the class. _member-list_ is a list of member
variable declarations. Classes can be declared to implement interfaces by
listing the interfaces after a `:` following the class name:

> `class` _class-name_ `:` _interface-list_ `{` _member-list_ `}`<br>

The _interface-list_ is a comma-separated list of one or more interface names.
The compiler will emit an error if the class doesn't actually implement the
specified interfaces.

#### Generic classes

Generic classes can be declared as follows:

> `class` _class-name_ `<` _generic-parameter-list_ `>` `{` _member-list_ `}`<br>

where _generic-parameter-list_ is a comma separated list of one or more generic
parameters. A generic parameter is one of the following:

> _generic-type-parameter_ → _identifier_<br>

The identifier of a _generic-type-parameter_ serves as a placeholder for types
used to instantiate the generic class.

### Structs

The syntax for declaring structs is exactly the same as for classes, except the
keyword `class` is substituted with `struct`. Like classes, structs can be
generic.

### Type aliases

Unlike `typedef` in C++ and C, and `using` in C++, type aliases in Delta are
strongly-typed, i.e. they define a whole new type. The new type behaves
exactly as the target (aliased) type, except that it cannot be implicitly
converted to or from that type. Explicit conversions on the other hand are
allowed. This can be used to create type-safe abstractions.

> _alias-declaration_ → `class` _identifier_ `=` _class-name_ `;`<br>
> _alias-declaration_ → `struct` _identifier_ `=` _struct-name_ `;`<br>
> _alias-declaration_ → `interface` _identifier_ `=` _interface-name_ `;`<br>

Type aliases can also be generic:

> _generic-alias-declaration_ → `class` _identifier_ `<` _generic-parameter-list_ `>` `=` _class-name_ `;`<br>
> _generic-alias-declaration_ → `struct` _identifier_ `<` _generic-parameter-list_ `>` `=` _struct-name_ `;`<br>
> _generic-alias-declaration_ → `interface` _identifier_ `<` _generic-parameter-list_ `>` `=` _interface-name_ `;`<br>

Generic parameters declared in the _generic-parameter-list_ are available for
use in the _class-name_ / _struct-name_ / _interface-name_ part.

## Statements

### Assignment statement

> _assignment-statement_ → _lvalue-expression_ (`=` | `~=`) _expression_ `;`<br>
> _assignment-statement_ → `_` `=` _expression_ `;`<br>

Assignments in Delta don't return any value. This applies to compound
assignments as well, including `++` and `--` (see below). Furthermore, this obsoletes the
two different forms of `++` and `--`, so only the postfix versions are valid as
syntactic sugar for `+= 1` and `-= 1`, respectively.

The assignment to `_`, called a _discarding assignment_, can be used to ignore
the result of the expression, suppressing any compilation errors or warnings
that would otherwise be emitted.

An assignment that uses the `~=` operator instead of `=` is called a _raw
assignment_. The left-hand-side of a raw assignment is treated as uninitialized
memory. This means that the assignment will not invoke the deinitializer of the
left-hand value, even if its type has a deinitializer.

### Increment and decrement statements

> increment-statement → _lvalue-expression_ `++` `;`<br>
> decrement-statement → _lvalue-expression_ `--` `;`<br>

### Block

> _block_ → `{` _statement_* `}`<br>

### `if` statement

> _if-statement_ → `if` `(` _expression_ `)` _block_ ( `else` _block_ )<sub>opt</sub><br>
> _if-statement_ → `if` `(` _expression_ `)` _block_ `else` _if-statement_<br>

### `return` statement

> _return-statement_ → `return` _return-value-list_ `;`<br>

_return-value-list_ is a comma-separated list of zero or more return values.

### `for` statement

The `for` statement loop over a range. The syntax is as follows:

> _for-statement_ → `for` `(` _identifier_ `in` _range-expression_ `)` _block_<br>

### `while` statement

The `while` statement loops until a condition evaluates to `false`. The syntax
is as follows:

> _while-statement_ → `while` `(` _condition_ `)` _block_<br>

### `switch` statement

> _switch-statement_ → `switch` `(` _expression_ `)` `{` _case_+ `}`<br>
> _case_ → `case` _expression_ `:` _statement_+<br>
> _case_ → `default` `:` _statement_+<br>

In addition to integer types, the `switch` statement can be used to match
strings.

The cases in a `switch` statement don't fall through by default. The
fall-through behavior can be enabled for a individual cases with the
`fallthrough` keyword.

`switch` statements must be exhaustive if _expression_ is of an enum type. This
is enforced by the compiler.

### `defer` statement

The `defer` statement has the following syntax:

> _defer-statement_ → `defer` _block_<br>

The _block_ will be executed when leaving the scope where the _defer-statement_
is located. Multiple deferred blocks are executed in the reverse of the order
they were declared in. Return statements are disallowed inside the defer block.

## Expressions

### Unary expressions

> _prefix-unary-expression_ → _operator_ _operand_<br>
> _postfix-unary-expression_ → _operand_ _operator_<br>

#### Unwrap expression

> _unwrap-expression_ → _operand_ `!`<br>

The _unwrap expression_ takes an operand of an optional type, and returns the
value wrapped by the optional. If the operand is null, a runtime error will be
triggered, except in unchecked mode, where the compiler may assume that the
operand is never null.

### Binary expression

> _binary-expression_ → _left-hand-side_ _binary-operator_ _right-hand-side_<br>

#### Integer arithmetic expressions

Arithmetic operations on integers (both signed and unsigned) are checked for
overflow by default. In unchecked builds, integer overflow causes undefined
behavior (for both signed and unsigned operations).

### Conditional expression

> _conditional-expression_ → _condition_ `?` _then-expression_ `:` _else-expression_<br>

### Member access expression

> _member-access-expression_ → _expression_ `.` _identifier_<br>

### Subscript expression

> _subscript-expression_ → _expression_ `[` _expression_ `]`<br>

### Function call expression

> _call-expression_ → _function-expression_ `(` _argument-list_ `)` <br>

_argument-list_ is a comma-separated list of zero or more _argument-specifiers_:

> _argument-specifier_ → _unnamed-argument_ | _named-argument_<br>
> _unnamed-argument_ → _expression_<br>
> _named-argument_ → _argument-name_ `:` _expression_<br>

_argument-name_ is an identifier specifying the name of the parameter the
argument _expression_ is being assigned to.

### Range expression

> _exclusive-range-expression_ → _lower-bound_ `..` _upper-bound_<br>
> _inclusive-range-expression_ → _lower-bound_ `...` _upper-bound_<br>

### Closure expression

> _closure-expression_ → `(` _parameter-list_ `)` `->` _expression_<br>
> _closure-expression_ → `(` _parameter-list_ `)` `->` _block_<br>
> _closure-expression_ → _block_<br>

Specifying the type for parameters in a closure _parameter-list_ is optional.
Omitting the type (and the corresponding colon) causes the type for that
parameter to be inferred from the context.

If the closure _parameter-list_ only contains one parameter, the enclosing
parentheses may be omitted.

## Error handling

Functions that may throw errors are marked with the `throws` keyword,
immediately after the `)` that terminates the parameter list. Other functions
may never throw (the compiler enforces this), which helps generate more
efficient code. Calling a throwing function requires the use of the `try` keyword:

> _try-expression_ → `try` _throwing-call-expression_<br>

This makes it clear which individual calls may throw and stop the execution in
the enclosing function.
