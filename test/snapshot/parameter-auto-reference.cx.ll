
define void @_EN4main1fE1S({} %s) {
  %1 = alloca {}, align 8
  store {} %s, {}* %1, align 1
  call void @_EN4main1gEP1S({}* %1)
  ret void
}

define void @_EN4main1gEP1S({}* %s) {
  ret void
}
