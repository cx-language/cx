
define void @_EN4main3fooEP5SliceI3intE(ptr %a) {
  %a1 = alloca ptr, align 8
  store ptr %a, ptr %a1, align 8
  ret void
}
