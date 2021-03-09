
define i32 @main() {
  call void @_EN4main1fI3intEE()
  ret i32 0
}

define void @_EN4main1fI3intEE() {
  %i = alloca i32, align 4
  store i32 0, i32* %i, align 4
  %i.load = load i32, i32* %i, align 4
  %1 = add i32 %i.load, 1
  store i32 %1, i32* %i, align 4
  ret void
}
