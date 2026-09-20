
define [2 x i32] @_EN4main1fE3int3int(i32 %foo, i32 %bar) {
  %foo1 = alloca i32, align 4
  %bar2 = alloca i32, align 4
  store i32 %foo, ptr %foo1, align 4
  store i32 %bar, ptr %bar2, align 4
  %foo.load = load i32, ptr %foo1, align 4
  %bar.load = load i32, ptr %bar2, align 4
  %1 = add i32 %foo.load, %bar.load
  %2 = insertvalue [2 x i32] undef, i32 %1, 0
  %foo.load3 = load i32, ptr %foo1, align 4
  %bar.load4 = load i32, ptr %bar2, align 4
  %3 = sub i32 %foo.load3, %bar.load4
  %4 = insertvalue [2 x i32] %2, i32 %3, 1
  ret [2 x i32] %4
}

define [2 x i32] @_EN4main1gE3int3int(i32 %foo, i32 %bar) {
  %foo1 = alloca i32, align 4
  %bar2 = alloca i32, align 4
  %c = alloca [2 x i32], align 4
  store i32 %foo, ptr %foo1, align 4
  store i32 %bar, ptr %bar2, align 4
  %foo.load = load i32, ptr %foo1, align 4
  %bar.load = load i32, ptr %bar2, align 4
  %1 = add i32 %foo.load, %bar.load
  %2 = insertvalue [2 x i32] undef, i32 %1, 0
  %foo.load3 = load i32, ptr %foo1, align 4
  %bar.load4 = load i32, ptr %bar2, align 4
  %3 = sub i32 %foo.load3, %bar.load4
  %4 = insertvalue [2 x i32] %2, i32 %3, 1
  store [2 x i32] %4, ptr %c, align 4
  %c.load = load [2 x i32], ptr %c, align 4
  ret [2 x i32] %c.load
}
