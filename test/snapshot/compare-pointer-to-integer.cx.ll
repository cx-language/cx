
define void @_EN4main1fEP3int3int(ptr %foo, i32 %bar) {
  %foo1 = alloca ptr, align 8
  %bar2 = alloca i32, align 4
  store ptr %foo, ptr %foo1, align 8
  store i32 %bar, ptr %bar2, align 4
  %foo.load = load ptr, ptr %foo1, align 8
  %bar.load = load i32, ptr %bar2, align 4
  %foo.load.load = load i32, ptr %foo.load, align 4
  %1 = icmp slt i32 %foo.load.load, %bar.load
  ret void
}
