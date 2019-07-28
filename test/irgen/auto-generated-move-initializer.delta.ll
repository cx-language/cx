
%X = type { i32 }

define void @_EN4main1X6deinitE(%X* %this) {
  ret void
}

define void @_EN4main1X4initE1i3int(%X* %this, i32 %i) {
  %i1 = getelementptr inbounds %X, %X* %this, i32 0, i32 0
  store i32 %i, i32* %i1
  ret void
}

define void @_EN4main3fooE1pP1X1x1X(%X* %p, %X %x) {
  store %X %x, %X* %p
  ret void
}
