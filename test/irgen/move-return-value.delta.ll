
define {} @_EN4main1fE() {
  %x = alloca {}
  call void @_ENM4main1X4initE({}* %x)
  %x1 = load {}, {}* %x
  ret {} %x1
}

define void @_ENM4main1X6deinitE({}* %this) {
  ret void
}

define void @_ENM4main1X4initE({}* %this) {
  ret void
}

define i32 @main() {
  %a = alloca {}
  %1 = call {} @_EN4main1fE()
  store {} %1, {}* %a
  call void @_ENM4main1X6deinitE({}* %a)
  ret i32 0
}
