
@0 = private unnamed_addr constant [59 x i8] c"integer overflow at dereference-vs-multiplication.cx:11:5\0A\00", align 1

define i32 @_EN4main1gE() {
  ret i32 1
}

define void @_EN4main1fEP3int3int(ptr %a, i32 %b) {
  %a1 = alloca ptr, align 8
  %b2 = alloca i32, align 4
  %m = alloca i32, align 4
  %n = alloca i32, align 4
  store ptr %a, ptr %a1, align 8
  store i32 %b, ptr %b2, align 4
  %1 = call i32 @_EN4main1gE()
  store i32 %1, ptr %m, align 4
  %a.load = load ptr, ptr %a1, align 8
  %b.load = load i32, ptr %b2, align 4
  store i32 %b.load, ptr %a.load, align 4
  %2 = call i32 @_EN4main1gE()
  %b.load3 = load i32, ptr %b2, align 4
  %3 = sext i32 %2 to i128
  %4 = sext i32 %b.load3 to i128
  %5 = mul i128 %3, %4
  %6 = trunc i128 %5 to i32
  %7 = sext i32 %6 to i128
  %8 = icmp ne i128 %5, %7
  %9 = xor i1 %8, true
  %overflow.condition = icmp eq i1 %9, false
  br i1 %overflow.condition, label %overflow.fail, label %overflow.success

overflow.fail:                                    ; preds = %0
  call void @_EN3std10assertFailEP4char(ptr @0)
  unreachable

overflow.success:                                 ; preds = %0
  store i32 %6, ptr %n, align 4
  ret void
}

declare void @_EN3std10assertFailEP4char(ptr)
