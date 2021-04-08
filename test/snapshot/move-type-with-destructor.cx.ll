
define void @_EN4main1fE1YP1Y({} %a, {}* %b) {
  call void @_EN4main1Y6deinitE({}* %b)
  store {} %a, {}* %b, align 1
  ret void
}

define void @_EN4main1Y6deinitE({}* %this) {
  ret void
}

define void @_EN4main1gE1Y({} %a) {
  %b = alloca {}, align 8
  store {} %a, {}* %b, align 1
  call void @_EN4main1Y6deinitE({}* %b)
  ret void
}
