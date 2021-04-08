
%X = type {}

define i32 @main() {
  %x = alloca %X, align 8
  call void @_EN4main1X4initE3int3int(%X* %x, i32 4, i32 2)
  ret i32 0
}

define void @_EN4main1X4initE3int3int(%X* %this, i32 %a, i32 %b) {
  call void @_EN4main1X4initE3int(%X* %this, i32 %a)
  ret void
}

define void @_EN4main1X4initE3int(%X* %this, i32 %a) {
  call void @_EN4main1X4initE(%X* %this)
  ret void
}

define void @_EN4main1X4initE(%X* %this) {
  ret void
}
