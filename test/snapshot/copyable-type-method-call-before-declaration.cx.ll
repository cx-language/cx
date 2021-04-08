
%B = type {}

define i32 @main() {
  %1 = alloca %B, align 8
  call void @_EN4main1B4initE(%B* %1)
  call void @_EN4main1B1fE(%B* %1)
  ret i32 0
}

define void @_EN4main1B4initE(%B* %this) {
  ret void
}

define void @_EN4main1B1fE(%B* %this) {
  ret void
}
