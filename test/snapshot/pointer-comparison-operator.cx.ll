
define void @_EN4main3fooEOP3intP3int(ptr %a, ptr %b) {
  %a1 = alloca ptr, align 8
  %b2 = alloca ptr, align 8
  store ptr %a, ptr %a1, align 8
  store ptr %b, ptr %b2, align 8
  %a.load = load ptr, ptr %a1, align 8
  %b.load = load ptr, ptr %b2, align 8
  %1 = icmp eq ptr %a.load, %b.load
  %a.load3 = load ptr, ptr %a1, align 8
  %b.load4 = load ptr, ptr %b2, align 8
  %2 = icmp ne ptr %a.load3, %b.load4
  ret void
}
