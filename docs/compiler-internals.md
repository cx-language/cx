# Compiler Internals

## Nullability analysis

During type-checking, optional types are implicitly unwrapped when needed.
After type-checking, nullability analysis phase emits warnings for null-safety violations. 
