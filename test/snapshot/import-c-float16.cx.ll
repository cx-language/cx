
define i32 @main() {
  %1 = call i32 @plain(i32 41)
  call void @_EN3std7printlnI3intEE3int(i32 %1)
  ret i32 0
}

declare i32 @plain(i32)

define void @_EN3std7printlnI3intEE3int(i32 %value) {
  %value1 = alloca i32, align 4
  %1 = alloca i8, align 1
  store i32 %value, ptr %value1, align 4
  call void @_EN3std5printI3intEER3int(ptr %value1)
  store i8 10, ptr %1, align 1
  call void @_EN3std5printI4charEER4char(ptr %1)
  ret void
}

declare void @_EN3std5printI3intEER3int(ptr)

declare void @_EN3std5printI4charEER4char(ptr)
