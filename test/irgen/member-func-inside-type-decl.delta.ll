
%A = type { i32 }

define void @_ENM4main1A4initE(%A* %this) {
  %i = getelementptr inbounds %A, %A* %this, i32 0, i32 0
  store i32 0, i32* %i
  ret void
}

define void @_ENM4main1A6deinitE(%A* %this) {
  ret void
}

define void @_EN4main1A3fooE(%A* %this) {
  ret void
}

define void @_ENM4main1A6mutateE(%A* %this) {
  ret void
}

define i32 @main() {
  %a = alloca %A
  call void @_ENM4main1A4initE(%A* %a)
  call void @_EN4main1A3fooE(%A* %a)
  call void @_ENM4main1A6mutateE(%A* %a)
  call void @_ENM4main1A6deinitE(%A* %a)
  ret i32 0
}
