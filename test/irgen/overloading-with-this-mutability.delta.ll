
define i32 @main() {
  %aa = alloca {}
  %a = alloca {}*
  %b = alloca {}
  call void @_ENM4main1A4initE({}* %aa)
  store {}* %aa, {}** %a
  call void @_ENM4main1A4initE({}* %b)
  %a.load = load {}*, {}** %a
  call void @_EN4main1A3fooE({}* %a.load)
  call void @_ENM4main1A3fooE({}* %b)
  ret i32 0
}

define void @_ENM4main1A4initE({}* %this) {
  ret void
}

define void @_EN4main1A3fooE({}* %this) {
  %a = alloca i32
  store i32 1, i32* %a
  ret void
}

define void @_ENM4main1A3fooE({}* %this) {
  %b = alloca i32
  store i32 1, i32* %b
  ret void
}
