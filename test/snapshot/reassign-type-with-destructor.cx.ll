
%X = type {}

define i32 @main() {
  %x = alloca %X, align 8
  %1 = alloca %X, align 8
  call void @_EN4main1X4initE(%X* %x)
  call void @_EN4main1X6deinitE(%X* %x)
  call void @_EN4main1X4initE(%X* %1)
  %.load = load %X, %X* %1, align 1
  store %X %.load, %X* %x, align 1
  call void @_EN4main1X6deinitE(%X* %x)
  ret i32 0
}

define void @_EN4main1X4initE(%X* %this) {
  ret void
}

define void @_EN4main1X6deinitE(%X* %this) {
  ret void
}
