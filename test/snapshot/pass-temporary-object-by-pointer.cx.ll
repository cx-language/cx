
%A = type { i32 }
%B = type { i32 }

define void @_EN4main1fEP1A(ptr %a) {
  %a1 = alloca ptr, align 8
  store ptr %a, ptr %a1, align 8
  ret void
}

define void @_EN4main1fEP1B(ptr %b) {
  %b1 = alloca ptr, align 8
  store ptr %b, ptr %b1, align 8
  ret void
}

define void @_EN4main1fEP3int(ptr %i) {
  %i1 = alloca ptr, align 8
  store ptr %i, ptr %i1, align 8
  ret void
}

define i32 @main() {
  %a = alloca %A, align 8
  %b = alloca %B, align 8
  %1 = alloca %A, align 8
  %2 = alloca %B, align 8
  %3 = alloca i32, align 4
  call void @_EN4main1A4initE(ptr %1)
  call void @_EN4main1fEP1A(ptr %1)
  call void @_EN4main1B4initE(ptr %2)
  call void @_EN4main1fEP1B(ptr %2)
  store i32 0, ptr %3, align 4
  call void @_EN4main1fEP3int(ptr %3)
  ret i32 0
}

define void @_EN4main1A4initE(ptr %this) {
  ret void
}

define void @_EN4main1B4initE(ptr %this) {
  ret void
}
