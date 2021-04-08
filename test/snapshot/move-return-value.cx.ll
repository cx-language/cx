
%X = type {}

define %X @_EN4main1fE() {
  %x = alloca %X, align 8
  call void @_EN4main1X4initE(%X* %x)
  %x.load = load %X, %X* %x, align 1
  ret %X %x.load
}

define void @_EN4main1X4initE(%X* %this) {
  ret void
}

define void @_EN4main1X6deinitE(%X* %this) {
  ret void
}

define i32 @main() {
  %a = alloca %X, align 8
  %1 = call %X @_EN4main1fE()
  store %X %1, %X* %a, align 1
  call void @_EN4main1X6deinitE(%X* %a)
  ret i32 0
}
