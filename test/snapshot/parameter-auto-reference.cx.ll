
%S = type {}

define void @_EN4main1fE1S(%S %s) {
  %1 = alloca %S, align 8
  store %S %s, %S* %1, align 1
  call void @_EN4main1gEP1S(%S* %1)
  ret void
}

define void @_EN4main1gEP1S(%S* %s) {
  ret void
}
