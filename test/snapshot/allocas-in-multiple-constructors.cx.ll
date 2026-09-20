
%S = type {}

define i32 @main() {
  %s = alloca %S, align 8
  %t = alloca %S, align 8
  call void @_EN4main1S4initE(ptr %s)
  call void @_EN4main1S4initE3int(ptr %t, i32 1)
  ret i32 0
}

define void @_EN4main1S4initE(ptr %this) {
  %i = alloca i32, align 4
  store i32 4, ptr %i, align 4
  ret void
}

define void @_EN4main1S4initE3int(ptr %this, i32 %foo) {
  %foo1 = alloca i32, align 4
  %j = alloca i32, align 4
  store i32 %foo, ptr %foo1, align 4
  store i32 5, ptr %j, align 4
  ret void
}
