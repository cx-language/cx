
%"X<int>" = type {}

define i32 @_EN4main1gE4bool(i1 %i) {
  %i1 = alloca i1, align 1
  store i1 %i, ptr %i1, align 1
  %i.load = load i1, ptr %i1, align 1
  %1 = zext i1 %i.load to i32
  ret i32 %1
}

define i32 @main() {
  %x = alloca %"X<int>", align 8
  call void @_EN4main1XI3intE4initE(ptr %x)
  call void @_EN4main1XI3intE1fI4boolEEF4bool_3int(ptr %x, ptr @_EN4main1gE4bool)
  ret i32 0
}

define void @_EN4main1XI3intE4initE(ptr %this) {
  ret void
}

define void @_EN4main1XI3intE1fI4boolEEF4bool_3int(ptr %this, ptr %u) {
  %u1 = alloca ptr, align 8
  store ptr %u, ptr %u1, align 8
  ret void
}
