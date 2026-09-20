
define i32 @main() {
  %p = alloca ptr, align 8
  %cp = alloca ptr, align 8
  %v = alloca ptr, align 8
  %b = alloca ptr, align 8
  %i = alloca i32, align 4
  %p.load = load ptr, ptr %p, align 8
  store ptr %p.load, ptr %v, align 8
  %p.load1 = load ptr, ptr %p, align 8
  call void @_EN4main3barEP4void(ptr %p.load1)
  %v.load = load ptr, ptr %v, align 8
  call void @_EN4main3barEP4void(ptr %v.load)
  %v.load2 = load ptr, ptr %v, align 8
  store ptr %v.load2, ptr %b, align 8
  %b.load = load ptr, ptr %b, align 8
  store ptr %b.load, ptr %v, align 8
  %cp.load = load ptr, ptr %cp, align 8
  call void @_EN4main3bazEOP4void(ptr %cp.load)
  store i32 0, ptr %i, align 4
  call void @_EN4main3bazEOP4void(ptr %i)
  ret i32 0
}

define void @_EN4main3barEP4void(ptr %p) {
  %p1 = alloca ptr, align 8
  store ptr %p, ptr %p1, align 8
  ret void
}

define void @_EN4main3bazEOP4void(ptr %p) {
  %p1 = alloca ptr, align 8
  store ptr %p, ptr %p1, align 8
  ret void
}
