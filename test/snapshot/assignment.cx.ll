
define i32 @main() {
  %foo = alloca i32, align 4
  store i32 0, i32* %foo, align 4
  %foo.load = load i32, i32* %foo, align 4
  %1 = add i32 %foo.load, -1
  store i32 %1, i32* %foo, align 4
  store i32 666, i32* %foo, align 4
  %foo.load1 = load i32, i32* %foo, align 4
  %2 = add i32 %foo.load1, 1
  store i32 %2, i32* %foo, align 4
  ret i32 0
}
