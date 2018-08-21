
define i32 @main() {
  %foo = alloca {}
  call void @_ENM4main3Foo6deinitE({}* %foo)
  ret i32 0
}

define void @_ENM4main3Foo6deinitE({}* %this) {
  ret void
}
