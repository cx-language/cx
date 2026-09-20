
define void @_EN4main1fEP1A(ptr %a) {
  %a1 = alloca ptr, align 8
  store ptr %a, ptr %a1, align 8
  %a.load = load ptr, ptr %a1, align 8
  call void @_EN4main1A6deinitE(ptr %a.load)
  ret void
}

define void @_EN4main1A6deinitE(ptr %this) {
  ret void
}

define void @_EN4main1gEP1B(ptr %b) {
  %b1 = alloca ptr, align 8
  store ptr %b, ptr %b1, align 8
  ret void
}

define void @_EN4main1hEP4char(ptr %c) {
  %c1 = alloca ptr, align 8
  store ptr %c, ptr %c1, align 8
  ret void
}
