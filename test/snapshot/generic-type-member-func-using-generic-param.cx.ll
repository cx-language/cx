
%"A<int>" = type {}

define i32 @main() {
  %a = alloca %"A<int>", align 8
  call void @_EN4main1AI3intE4initE(%"A<int>"* %a)
  call void @_EN4main1AI3intE1aE3int(%"A<int>"* %a, i32 5)
  ret i32 0
}

define void @_EN4main1AI3intE4initE(%"A<int>"* %this) {
  ret void
}

define void @_EN4main1AI3intE1aE3int(%"A<int>"* %this, i32 %n) {
  ret void
}
