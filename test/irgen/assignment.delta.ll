
define i32 @main() {
  %foo = alloca i32
  store i32 0, i32* %foo
  %1 = load i32, i32* %foo
  %2 = sub i32 %1, 1
  store i32 %2, i32* %foo
  store i32 666, i32* %foo
  %3 = load i32, i32* %foo
  %4 = add i32 %3, 1
  store i32 %4, i32* %foo
  ret i32 0
}
