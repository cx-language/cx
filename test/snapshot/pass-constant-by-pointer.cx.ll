
define i32 @main() {
  %s = alloca {}, align 8
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  call void @_EN4main1SI3intE4initE({}* %s)
  store i32 0, i32* %1, align 4
  call void @_EN4main1SI3intE1fEP3int({}* %s, i32* %1)
  store i32 0, i32* %2, align 4
  call void @_EN4main1SI3intE1fEP3int({}* %s, i32* %2)
  ret i32 0
}

define void @_EN4main1SI3intE4initE({}* %this) {
  ret void
}

define void @_EN4main1SI3intE1fEP3int({}* %this, i32* %t) {
  ret void
}
