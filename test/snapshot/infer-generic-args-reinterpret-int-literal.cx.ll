
define i32 @main() {
  %u = alloca i32, align 4
  %u.load = load i32, ptr %u, align 4
  call void @_EN4main1fI4uintEE4uint4uint(i32 0, i32 %u.load)
  ret i32 0
}

define void @_EN4main1fI4uintEE4uint4uint(i32 %a, i32 %b) {
  %a1 = alloca i32, align 4
  %b2 = alloca i32, align 4
  store i32 %a, ptr %a1, align 4
  store i32 %b, ptr %b2, align 4
  ret void
}
