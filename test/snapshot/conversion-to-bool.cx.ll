
define void @_EN4main1fEP4char(ptr %pb) {
  %pb1 = alloca ptr, align 8
  %b = alloca i1, align 1
  store ptr %pb, ptr %pb1, align 8
  store i1 true, ptr %b, align 1
  %pb.load = load ptr, ptr %pb1, align 8
  %pb.load.load = load i8, ptr %pb.load, align 1
  %1 = icmp ne i8 %pb.load.load, 0
  store i1 %1, ptr %b, align 1
  %b.load = load i1, ptr %b, align 1
  ret void
}
