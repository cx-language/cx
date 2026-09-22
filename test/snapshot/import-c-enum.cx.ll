
define i32 @main() {
  %foo = alloca i32, align 4
  %bar = alloca i32, align 4
  %qux = alloca i64, align 8
  %qux2 = alloca i8, align 1
  %qux3 = alloca i32, align 4
  %qux4 = alloca i32, align 4
  store i32 -1, ptr %foo, align 4
  store i32 0, ptr %bar, align 4
  store i64 0, ptr %qux, align 8
  store i8 0, ptr %qux2, align 1
  store i32 -1, ptr %qux3, align 4
  %qux3.load = load i32, ptr %qux3, align 4
  store i32 %qux3.load, ptr %qux4, align 4
  ret i32 0
}
