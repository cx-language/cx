
@my_global = external global i32

define i32 @main() {
  %my_global.load = load i32, ptr @my_global, align 4
  call void @_EN3std5printI3intEE3int(i32 %my_global.load)
  ret i32 0
}

declare void @_EN3std5printI3intEE3int(i32)
