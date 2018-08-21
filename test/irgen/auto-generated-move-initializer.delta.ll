
%X = type { i32 }

define void @_ENM4main1X4initE(%X* %this) {
  %i = getelementptr inbounds %X, %X* %this, i32 0, i32 0
  store i32 0, i32* %i
  ret void
}

define void @_ENM4main1X6deinitE(%X* %this) {
  ret void
}

define void @_EN4main3fooE1pPM1X1x1X(%X* %p, %X %x) {
  store %X %x, %X* %p
  ret void
}

define i32 @main() {
  %x = alloca %X
  %y = alloca %X
  call void @_ENM4main1X4initE(%X* %x)
  call void @_ENM4main1X4initE(%X* %y)
  %y1 = load %X, %X* %y
  call void @_EN4main3fooE1pPM1X1x1X(%X* %x, %X %y1)
  call void @_ENM4main1X6deinitE(%X* %x)
  ret i32 0
}
