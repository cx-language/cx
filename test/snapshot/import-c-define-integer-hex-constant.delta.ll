
define i32 @main() {
  %foo = alloca i32, align 4
  store i32 35044, i32* %foo, align 4
  ret i32 0
}
