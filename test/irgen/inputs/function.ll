define void @main() {
  call void @bar()
  ret void
}

declare void @bar()

define void @foo() {
  ret void
}
