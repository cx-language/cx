
define void @_EN4main3fooEP3int(ptr %ref_i) {
  %ref_i1 = alloca ptr, align 8
  store ptr %ref_i, ptr %ref_i1, align 8
  ret void
}

define i32 @main() {
  %bar = alloca i32, align 4
  store i32 42, ptr %bar, align 4
  call void @_EN4main3fooEP3int(ptr %bar)
  ret i32 0
}
