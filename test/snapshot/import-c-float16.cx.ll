
define i32 @main() {
  %1 = call i32 @plain(i32 41)
  call void @_EN3std7printlnI3intEE3int(i32 %1)
  ret i32 0
}

declare i32 @plain(i32)

define void @_EN3std7printlnI3intEE3int(i32 %value) {
  %value1 = alloca i32, align 4
  store i32 %value, ptr %value1, align 4
  %value.load = load i32, ptr %value1, align 4
  call void @_EN3std5printI3intEE3int(i32 %value.load)
  call void @_EN3std5printI4charEE4char(i8 10)
  ret void
}

declare void @_EN3std5printI3intEE3int(i32)

declare void @_EN3std5printI4charEE4char(i8)
