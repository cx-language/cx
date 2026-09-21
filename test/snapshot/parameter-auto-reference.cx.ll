
%S = type {}

define void @_EN4main1fE1S(%S %s) {
  %s1 = alloca %S, align 8
  store %S %s, ptr %s1, align 1
  call void @_EN4main1gER1S(ptr %s1)
  ret void
}

define void @_EN4main1gER1S(ptr %s) {
  %s1 = alloca ptr, align 8
  store ptr %s, ptr %s1, align 8
  ret void
}
