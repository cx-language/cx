
define i32 @main() {
  %u = alloca i32
  %u1 = load i32, i32* %u
  call void @_EN4main1fI4uintEE1a4uint1b4uint(i32 0, i32 %u1)
  ret i32 0
}

define void @_EN4main1fI4uintEE1a4uint1b4uint(i32 %a, i32 %b) {
  ret void
}
