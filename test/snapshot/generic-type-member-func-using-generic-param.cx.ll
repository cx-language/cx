
%"A<int>" = type {}

define i32 @main() {
  %a = alloca %"A<int>", align 8
  call void @_EN4main1AI3intE4initE(ptr %a)
  call void @_EN4main1AI3intE1aE3int(ptr %a, i32 5)
  ret i32 0
}

define void @_EN4main1AI3intE4initE(ptr %this) {
  ret void
}

define void @_EN4main1AI3intE1aE3int(ptr %this, i32 %n) {
  %n1 = alloca i32, align 4
  store i32 %n, ptr %n1, align 4
  ret void
}
