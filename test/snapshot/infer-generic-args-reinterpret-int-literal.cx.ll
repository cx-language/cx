
define i32 @main() {
  %u = alloca i32, align 4
  %u.load = load i32, i32* %u, align 4
  call void @_EN4main1fI4uintEE4uint4uint(i32 0, i32 %u.load)
  ret i32 0
}

define void @_EN4main1fI4uintEE4uint4uint(i32 %a, i32 %b) {
  ret void
}
