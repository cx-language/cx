
%"A<int>" = type { i32 }

define void @_EN4main1AI3intE6deinitE(%"A<int>"* %this) {
  ret void
}

define void @_EN4main1AI3intE4initE1i3int(%"A<int>"* %this, i32 %i) {
  %i1 = getelementptr inbounds %"A<int>", %"A<int>"* %this, i32 0, i32 0
  store i32 %i, i32* %i1
  ret void
}

define void @_EN4main3fooE1a1AI3intE(%"A<int>" %a) {
  %1 = alloca %"A<int>"
  store %"A<int>" %a, %"A<int>"* %1
  call void @_EN4main1AI3intE6deinitE(%"A<int>"* %1)
  ret void
}
