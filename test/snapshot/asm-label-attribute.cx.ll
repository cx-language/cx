
define i32 @main() {
  call void @"\01test"()
  ret i32 0
}

declare void @"\01test"()
