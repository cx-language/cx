
%A = type { i32 }
%B = type { i32 }

define void @_EN4main1fEP1A(%A* %a) {
  ret void
}

define void @_EN4main1fEP1B(%B* %b) {
  ret void
}

define void @_EN4main1fEP3int(i32* %i) {
  ret void
}

define i32 @main() {
  %a = alloca %A, align 8
  %b = alloca %B, align 8
  %1 = alloca %A, align 8
  %2 = alloca %B, align 8
  %3 = alloca i32, align 4
  call void @_EN4main1A4initE(%A* %1)
  call void @_EN4main1fEP1A(%A* %1)
  call void @_EN4main1B4initE(%B* %2)
  call void @_EN4main1fEP1B(%B* %2)
  store i32 0, i32* %3, align 4
  call void @_EN4main1fEP3int(i32* %3)
  ret i32 0
}

define void @_EN4main1A4initE(%A* %this) {
  ret void
}

define void @_EN4main1B4initE(%B* %this) {
  ret void
}
