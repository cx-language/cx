// RUN: %delta -print-ir -Iinputs %s | %FileCheck %s

typedef enum {
    FOO = 42
} Foo;
