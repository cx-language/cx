define i32 @main() {
  %1 = call i32 @foo()
  ret i32 %1
}

define i32 @foo() {
  %1 = call i32 @foo()
  ret i32 42
}
