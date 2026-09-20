
%"A<int>" = type { i32 }

define void @_EN4main3fooE1AI3intE(%"A<int>" %a) {
  %a1 = alloca %"A<int>", align 8
  store %"A<int>" %a, ptr %a1, align 4
  call void @_EN4main1AI3intE6deinitE(ptr %a1)
  ret void
}

define void @_EN4main1AI3intE6deinitE(ptr %this) {
  ret void
}
