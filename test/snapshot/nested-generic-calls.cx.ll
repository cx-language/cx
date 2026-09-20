
define i32 @main() {
  call void @_EN4main1gI3intEE3int(i32 42)
  call void @_EN4main1gI4boolEE4bool(i1 false)
  ret i32 0
}

define void @_EN4main1gI3intEE3int(i32 %t) {
  %t1 = alloca i32, align 4
  store i32 %t, ptr %t1, align 4
  %t.load = load i32, ptr %t1, align 4
  call void @_EN4main1fI3intEE3int(i32 %t.load)
  ret void
}

define void @_EN4main1gI4boolEE4bool(i1 %t) {
  %t1 = alloca i1, align 1
  store i1 %t, ptr %t1, align 1
  %t.load = load i1, ptr %t1, align 1
  call void @_EN4main1fI4boolEE4bool(i1 %t.load)
  ret void
}

define void @_EN4main1fI3intEE3int(i32 %t) {
  %t1 = alloca i32, align 4
  store i32 %t, ptr %t1, align 4
  ret void
}

define void @_EN4main1fI4boolEE4bool(i1 %t) {
  %t1 = alloca i1, align 1
  store i1 %t, ptr %t1, align 1
  ret void
}
