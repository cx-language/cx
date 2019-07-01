
define i32 @main() {
  call void @_EN4main1gI3intEE1t3int(i32 42)
  call void @_EN4main1gI4boolEE1t4bool(i1 false)
  ret i32 0
}

define void @_EN4main1gI3intEE1t3int(i32 %t) {
  call void @_EN4main1fI3intEE1t3int(i32 %t)
  ret void
}

define void @_EN4main1gI4boolEE1t4bool(i1 %t) {
  call void @_EN4main1fI4boolEE1t4bool(i1 %t)
  ret void
}

define void @_EN4main1fI3intEE1t3int(i32 %t) {
  ret void
}

define void @_EN4main1fI4boolEE1t4bool(i1 %t) {
  ret void
}
