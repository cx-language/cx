
define i32 @main() {
  %x = alloca {}
  call void @_ENM4main1X4initE1a3int1b3int({}* %x, i32 4, i32 2)
  ret i32 0
}

define void @_ENM4main1X4initE1a3int1b3int({}* %this, i32 %a, i32 %b) {
  call void @_ENM4main1X4initE1a3int({}* %this, i32 %a)
  ret void
}

define void @_ENM4main1X4initE1a3int({}* %this, i32 %a) {
  call void @_ENM4main1X4initE({}* %this)
  ret void
}

define void @_ENM4main1X4initE({}* %this) {
  ret void
}
