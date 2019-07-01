
define i32 @_EN4main1gE1i4bool(i1 %i) {
  %1 = zext i1 %i to i32
  ret i32 %1
}

define i32 @main() {
  %x = alloca {}
  call void @_EN4main1XI3intE4initE({}* %x)
  call void @_EN4main1XI3intE1fI4boolEE1uF4bool_3int({}* %x, i32 (i1)* @_EN4main1gE1i4bool)
  ret i32 0
}

define void @_EN4main1XI3intE4initE({}* %this) {
  ret void
}

define void @_EN4main1XI3intE1fI4boolEE1uF4bool_3int({}* %this, i32 (i1)* %u) {
  ret void
}
