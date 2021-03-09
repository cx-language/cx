
define void @_EN4main3fooE() {
  ret void
}

define i32 @_EN4main2f2E3int4bool(i32 %a, i1 %b) {
  ret i32 %a
}

define void @_EN4main1gEF_4void(void ()* %p) {
  %p2 = alloca void ()*, align 8
  store void ()* %p, void ()** %p2, align 8
  %p2.load = load void ()*, void ()** %p2, align 8
  call void %p2.load()
  ret void
}

define void @_EN4main2g2EF3int4bool_3int(i32 (i32, i1)* %p) {
  %p2 = alloca i32 (i32, i1)*, align 8
  %a = alloca i32, align 4
  store i32 (i32, i1)* %p, i32 (i32, i1)** %p2, align 8
  %p2.load = load i32 (i32, i1)*, i32 (i32, i1)** %p2, align 8
  %1 = call i32 %p2.load(i32 42, i1 false)
  %2 = add i32 %1, 1
  store i32 %2, i32* %a, align 4
  ret void
}

define i32 @main() {
  %lf = alloca void ()*, align 8
  %lf2 = alloca i32 (i32, i1)*, align 8
  store void ()* @_EN4main3fooE, void ()** %lf, align 8
  %lf.load = load void ()*, void ()** %lf, align 8
  call void @_EN4main1gEF_4void(void ()* %lf.load)
  store i32 (i32, i1)* @_EN4main2f2E3int4bool, i32 (i32, i1)** %lf2, align 8
  %lf2.load = load i32 (i32, i1)*, i32 (i32, i1)** %lf2, align 8
  call void @_EN4main2g2EF3int4bool_3int(i32 (i32, i1)* %lf2.load)
  ret i32 0
}
