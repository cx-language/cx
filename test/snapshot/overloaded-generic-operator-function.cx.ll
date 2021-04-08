
%S = type {}

define void @_EN4maineqE1S1S(%S %a, %S %b) {
  ret void
}

define void @_EN4main1fE1S(%S %s) {
  call void @_EN4maineqE1S1S(%S %s, %S %s)
  ret void
}
