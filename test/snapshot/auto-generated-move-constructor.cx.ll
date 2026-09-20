
%X = type { i32 }

define void @_EN4main3fooEP1X1X(ptr %p, %X %x) {
  %p1 = alloca ptr, align 8
  %x2 = alloca %X, align 8
  store ptr %p, ptr %p1, align 8
  store %X %x, ptr %x2, align 4
  %p.load = load ptr, ptr %p1, align 8
  %x.load = load %X, ptr %x2, align 4
  store %X %x.load, ptr %p.load, align 4
  ret void
}

define void @_EN4main1X6deinitE(ptr %this) {
  ret void
}
