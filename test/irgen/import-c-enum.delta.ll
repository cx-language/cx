
define i32 @main() {
  %foo = alloca i32
  %bar = alloca i32
  %qux = alloca i64
  %qux2 = alloca i8
  store i32 -1, i32* %foo
  store i32 0, i32* %bar
  store i64 0, i64* %qux
  store i8 0, i8* %qux2
  ret i32 0
}
