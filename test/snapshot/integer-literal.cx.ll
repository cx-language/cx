
define i32 @main() {
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  %k = alloca i32, align 4
  store i32 42, i32* %i, align 4
  store i32 1, i32* %j, align 4
  store i32 105, i32* %k, align 4
  ret i32 0
}
