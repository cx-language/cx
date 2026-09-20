
define void @_EN4main3fooEPP4charP4char(ptr %p, ptr %x) {
  %p1 = alloca ptr, align 8
  %x2 = alloca ptr, align 8
  %pp = alloca ptr, align 8
  store ptr %p, ptr %p1, align 8
  store ptr %x, ptr %x2, align 8
  %p.load = load ptr, ptr %p1, align 8
  store ptr %p.load, ptr %pp, align 8
  %pp.load = load ptr, ptr %pp, align 8
  %x.load = load ptr, ptr %x2, align 8
  store ptr %x.load, ptr %pp.load, align 8
  %pp.load3 = load ptr, ptr %pp, align 8
  %pp.load.load = load ptr, ptr %pp.load3, align 8
  store i8 120, ptr %pp.load.load, align 1
  ret void
}
