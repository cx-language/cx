
define i32 @main() {
  %foo = alloca i32, align 4
  %bar = alloca i32, align 4
  %qux = alloca i64, align 8
  %qux2 = alloca i8, align 1
  %qux3 = alloca i32, align 4
  %qux4 = alloca i32, align 4
  store i32 -1, i32* %foo, align 4
  store i32 0, i32* %bar, align 4
  store i64 0, i64* %qux, align 4
  store i8 0, i8* %qux2, align 1
  store i32 -1, i32* %qux3, align 4
  %qux3.load = load i32, i32* %qux3, align 4
  store i32 %qux3.load, i32* %qux4, align 4
  ret i32 0
}
