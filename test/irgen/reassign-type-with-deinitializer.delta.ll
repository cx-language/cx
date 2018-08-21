
define i32 @main() {
  %x = alloca {}
  %1 = alloca {}
  call void @_ENM4main1X4initE({}* %x)
  call void @_ENM4main1X6deinitE({}* %x)
  call void @_ENM4main1X4initE({}* %1)
  %2 = load {}, {}* %1
  store {} %2, {}* %x
  call void @_ENM4main1X6deinitE({}* %x)
  ret i32 0
}

define void @_ENM4main1X6deinitE({}* %this) {
  ret void
}

define void @_ENM4main1X4initE({}* %this) {
  ret void
}
