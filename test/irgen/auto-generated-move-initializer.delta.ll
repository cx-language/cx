
%X = type { i32 }

define void @_ENM4main1X6deinitE(%X* %this) {
  ret void
}

define void @_EN4main3fooE1pPM1X1x1X(%X* %p, %X %x) {
  store %X %x, %X* %p
  ret void
}
