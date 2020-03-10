
%X = type { i32 }

define void @_EN4main3fooE1pP1X1x1X(%X* %p, %X %x) {
  store %X %x, %X* %p
  ret void
}

define void @_EN4main1X6deinitE(%X* %this) {
  ret void
}
