
define void @_EN4main1fEP3int(ptr %a) {
  %a1 = alloca ptr, align 8
  %p = alloca ptr, align 8
  store ptr %a, ptr %a1, align 8
  %a.load = load ptr, ptr %a1, align 8
  store ptr %a.load, ptr %p, align 8
  %p.load = load ptr, ptr %p, align 8
  %p.load.load = load i32, ptr %p.load, align 4
  %1 = add i32 %p.load.load, 1
  store i32 %1, ptr %p.load, align 4
  %a.load2 = load ptr, ptr %a1, align 8
  %a.load.load = load i32, ptr %a.load2, align 4
  %2 = add i32 %a.load.load, -1
  store i32 %2, ptr %a.load2, align 4
  ret void
}
