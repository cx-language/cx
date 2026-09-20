
define void @_EN4main3barE() {
  ret void
}

define void @_EN4main3fooE3int(i32 %bar) {
  %bar1 = alloca i32, align 4
  store i32 %bar, ptr %bar1, align 4
  call void @_EN4main3barE()
  ret void
}
