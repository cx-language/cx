
%Y = type {}

define void @_EN4main1fE1YP1Y(%Y %a, ptr %b) {
  %a1 = alloca %Y, align 8
  %b2 = alloca ptr, align 8
  store %Y %a, ptr %a1, align 1
  store ptr %b, ptr %b2, align 8
  %b.load = load ptr, ptr %b2, align 8
  call void @_EN4main1Y6deinitE(ptr %b.load)
  %a.load = load %Y, ptr %a1, align 1
  store %Y %a.load, ptr %b.load, align 1
  ret void
}

define void @_EN4main1Y6deinitE(ptr %this) {
  ret void
}

define void @_EN4main1gE1Y(%Y %a) {
  %a1 = alloca %Y, align 8
  %b = alloca %Y, align 8
  store %Y %a, ptr %a1, align 1
  %a.load = load %Y, ptr %a1, align 1
  store %Y %a.load, ptr %b, align 1
  call void @_EN4main1Y6deinitE(ptr %b)
  ret void
}
