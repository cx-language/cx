define i32 @main() {
  call void @foo(i32 1)
  call void @foo.1(i1 false)
  ret i32 0
}

define void @foo(i32 %t) {
  ret void
}

define void @foo.1(i1 %t) {
  ret void
}
