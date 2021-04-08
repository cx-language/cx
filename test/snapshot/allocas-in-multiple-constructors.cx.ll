
define i32 @main() {
  %s = alloca {}, align 8
  %t = alloca {}, align 8
  call void @_EN4main1S4initE({}* %s)
  call void @_EN4main1S4initE3int({}* %t, i32 1)
  ret i32 0
}

define void @_EN4main1S4initE({}* %this) {
  %i = alloca i32, align 4
  store i32 4, i32* %i, align 4
  ret void
}

define void @_EN4main1S4initE3int({}* %this, i32 %foo) {
  %j = alloca i32, align 4
  store i32 5, i32* %j, align 4
  ret void
}
