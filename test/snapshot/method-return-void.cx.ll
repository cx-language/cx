
%C = type {}

define i32 @main() {
  %c = alloca %C, align 8
  call void @_EN4main1C4initE(%C* %c)
  call void @_EN4main1C1fE(%C* %c)
  ret i32 0
}

define void @_EN4main1C4initE(%C* %this) {
  ret void
}

define void @_EN4main1C1fE(%C* %this) {
  ret void
}
