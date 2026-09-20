
%X = type {}

define i32 @_EN4main1gE() {
  ret i32 0
}

define i1 @_EN4main1hE() {
  ret i1 false
}

define i32 @main() {
  %x = alloca %X, align 8
  call void @_EN4main1X4initE(ptr %x)
  %1 = call i32 @_EN4main1X1fI3intEEF_3int(ptr %x, ptr @_EN4main1gE)
  %2 = call i1 @_EN4main1X1fI4boolEEF_4bool(ptr %x, ptr @_EN4main1hE)
  ret i32 0
}

define void @_EN4main1X4initE(ptr %this) {
  ret void
}

define i32 @_EN4main1X1fI3intEEF_3int(ptr %this, ptr %t) {
  %t1 = alloca ptr, align 8
  store ptr %t, ptr %t1, align 8
  %t.load = load ptr, ptr %t1, align 8
  %1 = call i32 %t.load()
  ret i32 %1
}

define i1 @_EN4main1X1fI4boolEEF_4bool(ptr %this, ptr %t) {
  %t1 = alloca ptr, align 8
  store ptr %t, ptr %t1, align 8
  %t.load = load ptr, ptr %t1, align 8
  %1 = call i1 %t.load()
  ret i1 %1
}
