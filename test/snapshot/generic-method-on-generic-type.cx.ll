
%"X<int>" = type {}

define i32 @_EN4main1gE4bool(i1 %i) {
  %1 = zext i1 %i to i32
  ret i32 %1
}

define i32 @main() {
  %x = alloca %"X<int>", align 8
  call void @_EN4main1XI3intE4initE(%"X<int>"* %x)
  call void @_EN4main1XI3intE1fI4boolEEF4bool_3int(%"X<int>"* %x, i32 (i1)* @_EN4main1gE4bool)
  ret i32 0
}

define void @_EN4main1XI3intE4initE(%"X<int>"* %this) {
  ret void
}

define void @_EN4main1XI3intE1fI4boolEEF4bool_3int(%"X<int>"* %this, i32 (i1)* %u) {
  ret void
}
