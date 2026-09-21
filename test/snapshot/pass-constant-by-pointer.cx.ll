
%"S<int>" = type {}

define i32 @main() {
  %s = alloca %"S<int>", align 8
  %1 = alloca i32, align 4
  call void @_EN4main1SI3intE4initE(ptr %s)
  store i32 0, ptr %1, align 4
  call void @_EN4main1SI3intE1fEP3int(ptr %s, ptr %1)
  ret i32 0
}

define void @_EN4main1SI3intE4initE(ptr %this) {
  ret void
}

define void @_EN4main1SI3intE1fEP3int(ptr %this, ptr %t) {
  %t1 = alloca ptr, align 8
  store ptr %t, ptr %t1, align 8
  ret void
}
