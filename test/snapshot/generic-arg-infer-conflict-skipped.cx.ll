
define i32 @main() {
  %1 = call i32 @_EN4main3fooI3int4boolEE3int4bool(i32 1, i1 false)
  ret i32 0
}

define i32 @_EN4main3fooI3int4boolEE3int4bool(i32 %a, i1 %b) {
  %a1 = alloca i32, align 4
  %b2 = alloca i1, align 1
  store i32 %a, ptr %a1, align 4
  store i1 %b, ptr %b2, align 1
  ret i32 0
}
