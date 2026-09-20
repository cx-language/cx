
%S = type {}

define void @_EN4maineqE1S1S(%S %a, %S %b) {
  %a1 = alloca %S, align 8
  %b2 = alloca %S, align 8
  store %S %a, ptr %a1, align 1
  store %S %b, ptr %b2, align 1
  ret void
}

define void @_EN4main1fE1S(%S %s) {
  %s1 = alloca %S, align 8
  store %S %s, ptr %s1, align 1
  %s.load = load %S, ptr %s1, align 1
  %s.load2 = load %S, ptr %s1, align 1
  call void @_EN4maineqE1S1S(%S %s.load, %S %s.load2)
  ret void
}
