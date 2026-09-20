
@0 = private unnamed_addr constant [62 x i8] c"integer overflow at function-pointer-local-variable.cx:14:27\0A\00", align 1

define void @_EN4main3fooE() {
  ret void
}

define i32 @_EN4main2f2E3int4bool(i32 %a, i1 %b) {
  %a1 = alloca i32, align 4
  %b2 = alloca i1, align 1
  store i32 %a, ptr %a1, align 4
  store i1 %b, ptr %b2, align 1
  %a.load = load i32, ptr %a1, align 4
  ret i32 %a.load
}

define void @_EN4main1gEF_4void(ptr %p) {
  %p1 = alloca ptr, align 8
  %p2 = alloca ptr, align 8
  store ptr %p, ptr %p1, align 8
  %p.load = load ptr, ptr %p1, align 8
  store ptr %p.load, ptr %p2, align 8
  %p2.load = load ptr, ptr %p2, align 8
  call void %p2.load()
  ret void
}

define void @_EN4main2g2EF3int4bool_3int(ptr %p) {
  %p1 = alloca ptr, align 8
  %p2 = alloca ptr, align 8
  %a = alloca i32, align 4
  store ptr %p, ptr %p1, align 8
  %p.load = load ptr, ptr %p1, align 8
  store ptr %p.load, ptr %p2, align 8
  %p2.load = load ptr, ptr %p2, align 8
  %1 = call i32 %p2.load(i32 42, i1 false)
  %2 = sext i32 %1 to i128
  %3 = add i128 %2, 1
  %4 = trunc i128 %3 to i32
  %5 = sext i32 %4 to i128
  %6 = icmp ne i128 %3, %5
  %7 = xor i1 %6, true
  %overflow.condition = icmp eq i1 %7, false
  br i1 %overflow.condition, label %overflow.fail, label %overflow.success

overflow.fail:                                    ; preds = %0
  call void @_EN3std10assertFailEP4char(ptr @0)
  unreachable

overflow.success:                                 ; preds = %0
  store i32 %4, ptr %a, align 4
  ret void
}

declare void @_EN3std10assertFailEP4char(ptr)

define i32 @main() {
  %lf = alloca ptr, align 8
  %lf2 = alloca ptr, align 8
  store ptr @_EN4main3fooE, ptr %lf, align 8
  %lf.load = load ptr, ptr %lf, align 8
  call void @_EN4main1gEF_4void(ptr %lf.load)
  store ptr @_EN4main2f2E3int4bool, ptr %lf2, align 8
  %lf2.load = load ptr, ptr %lf2, align 8
  call void @_EN4main2g2EF3int4bool_3int(ptr %lf2.load)
  ret i32 0
}
