define i32 @main() {
  call void @"foo<int>"(i32 1)
  call void @"foo<bool>"(i1 false)
  call void @"foo<bool>"(i1 true)
  ret i32 0
}

define void @"foo<int>"(i32 %t) {
  ret void
}

define void @"foo<bool>"(i1 %t) {
  ret void
}
