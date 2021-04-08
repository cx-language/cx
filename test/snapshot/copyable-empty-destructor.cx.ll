
define i32 @main() {
  %foo = alloca {}, align 8
  call void @_EN4main3Foo6deinitE({}* %foo)
  ret i32 0
}

define void @_EN4main3Foo6deinitE({}* %this) {
  ret void
}
