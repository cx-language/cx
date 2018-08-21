
define i32 @main() {
  call void @_EN4main1fI3intEE()
  ret i32 0
}

define void @_EN4main1fI3intEE() {
  %i = alloca i32
  store i32 0, i32* %i
  %1 = load i32, i32* %i
  %2 = add i32 %1, 1
  store i32 %2, i32* %i
  ret void
}
