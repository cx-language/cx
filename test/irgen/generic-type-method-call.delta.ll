
define i32 @main() {
  %x = alloca {}
  call void @_ENM4main1SI1AE4initE({}* %x)
  call void @_EN4main1SI1AE1sE({}* %x)
  ret i32 0
}

define void @_ENM4main1SI1AE4initE({}* %this) {
  %1 = alloca {}
  call void @_ENM4main1A4initE({}* %1)
  %2 = load {}, {}* %1
  call void @_EN4main1A1hE({} %2)
  ret void
}

define void @_EN4main1SI1AE1sE({}* %this) {
  ret void
}

define void @_EN4main1A1hE({} %this) {
  ret void
}

define void @_ENM4main1A4initE({}* %this) {
  ret void
}
