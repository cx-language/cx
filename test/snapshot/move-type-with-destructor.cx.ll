
%Y = type {}

define void @_EN4main1fE1YP1Y(%Y %a, %Y* %b) {
  call void @_EN4main1Y6deinitE(%Y* %b)
  store %Y %a, %Y* %b, align 1
  ret void
}

define void @_EN4main1Y6deinitE(%Y* %this) {
  ret void
}

define void @_EN4main1gE1Y(%Y %a) {
  %b = alloca %Y, align 8
  store %Y %a, %Y* %b, align 1
  call void @_EN4main1Y6deinitE(%Y* %b)
  ret void
}
