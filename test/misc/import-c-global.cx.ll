
@my_global = external global i32

define i32 @main() {
  call void @_EN3std5printI3intEER3int(ptr @my_global)
  ret i32 0
}

declare void @_EN3std5printI3intEER3int(ptr)
