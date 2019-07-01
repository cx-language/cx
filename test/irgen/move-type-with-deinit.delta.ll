
define void @_EN4main1fE1a1Y1bP1Y({} %a, {}* %b) {
  call void @_EN4main1Y6deinitE({}* %b)
  store {} %a, {}* %b
  ret void
}

define void @_EN4main1Y6deinitE({}* %this) {
  ret void
}

define void @_EN4main1gE1a1Y({} %a) {
  %b = alloca {}
  store {} %a, {}* %b
  call void @_EN4main1Y6deinitE({}* %b)
  ret void
}
