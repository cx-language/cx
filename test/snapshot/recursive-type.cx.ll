
%S = type { ptr }

define void @_EN4main1fE1S(%S %s) {
  %s1 = alloca %S, align 8
  store %S %s, ptr %s1, align 8
  ret void
}
