
define void @_EN4main1fE1s1S({} %s) {
  %1 = alloca {}
  store {} %s, {}* %1
  call void @_EN4main1gE1sP1S({}* %1)
  ret void
}

define void @_EN4main1gE1sP1S({}* %s) {
  ret void
}
