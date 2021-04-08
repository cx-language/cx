
define i32 @main() {
  %x = alloca {}, align 8
  %1 = alloca {}, align 8
  call void @_EN4main1X4initE({}* %x)
  call void @_EN4main1X6deinitE({}* %x)
  call void @_EN4main1X4initE({}* %1)
  %.load = load {}, {}* %1, align 1
  store {} %.load, {}* %x, align 1
  call void @_EN4main1X6deinitE({}* %x)
  ret i32 0
}

define void @_EN4main1X4initE({}* %this) {
  ret void
}

define void @_EN4main1X6deinitE({}* %this) {
  ret void
}
