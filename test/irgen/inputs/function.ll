define i32 @main() {
  call void @bar()
  ret i32 0
}

declare void @bar()

define void @foo() {
  ret void
}
