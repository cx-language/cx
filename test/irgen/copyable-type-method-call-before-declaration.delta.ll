
define i32 @main() {
  %1 = alloca {}
  call void @_ENM4main1B4initE({}* %1)
  %.load = load {}, {}* %1
  call void @_EN4main1B1fE({} %.load)
  ret i32 0
}

define void @_EN4main1B1fE({} %this) {
  ret void
}

define void @_ENM4main1B4initE({}* %this) {
  ret void
}
