
%S = type {}

define i32 @main() {
  %s = alloca %S, align 8
  %t = alloca %S, align 8
  call void @_EN4main1S4initE(%S* %s)
  call void @_EN4main1S4initE3int(%S* %t, i32 1)
  ret i32 0
}

define void @_EN4main1S4initE(%S* %this) {
  %i = alloca i32, align 4
  store i32 4, i32* %i, align 4
  ret void
}

define void @_EN4main1S4initE3int(%S* %this, i32 %foo) {
  %j = alloca i32, align 4
  store i32 5, i32* %j, align 4
  ret void
}
