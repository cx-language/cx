
%X = type {}

define i32 @main() {
  %x = alloca %X, align 8
  call void @_EN4main1X4initE3int3int(ptr %x, i32 4, i32 2)
  ret i32 0
}

define void @_EN4main1X4initE3int3int(ptr %this, i32 %a, i32 %b) {
  %a1 = alloca i32, align 4
  %b2 = alloca i32, align 4
  store i32 %a, ptr %a1, align 4
  store i32 %b, ptr %b2, align 4
  %a.load = load i32, ptr %a1, align 4
  call void @_EN4main1X4initE3int(ptr %this, i32 %a.load)
  ret void
}

define void @_EN4main1X4initE3int(ptr %this, i32 %a) {
  %a1 = alloca i32, align 4
  store i32 %a, ptr %a1, align 4
  call void @_EN4main1X4initE(ptr %this)
  ret void
}

define void @_EN4main1X4initE(ptr %this) {
  ret void
}
