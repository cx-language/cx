
@0 = private unnamed_addr constant [50 x i8] c"integer overflow at auto-deref-arithmetic.cx:6:7\0A\00", align 1
@1 = private unnamed_addr constant [51 x i8] c"integer overflow at auto-deref-arithmetic.cx:7:11\0A\00", align 1

define i32 @main() {
  %a = alloca i32, align 4
  %p = alloca ptr, align 8
  store i32 0, ptr %a, align 4
  store ptr %a, ptr %p, align 8
  %a.load = load i32, ptr %a, align 4
  %p.load = load ptr, ptr %p, align 8
  %p.load.load = load i32, ptr %p.load, align 4
  %1 = sext i32 %a.load to i64
  %2 = sext i32 %p.load.load to i64
  %3 = add i64 %1, %2
  %4 = trunc i64 %3 to i32
  %5 = sext i32 %4 to i64
  %6 = icmp ne i64 %3, %5
  %7 = xor i1 %6, true
  %overflow.condition = icmp eq i1 %7, false
  br i1 %overflow.condition, label %overflow.fail, label %overflow.success

overflow.fail:                                    ; preds = %0
  call void @_EN3std10assertFailEP4char(ptr @0)
  unreachable

overflow.success:                                 ; preds = %0
  store i32 %4, ptr %a, align 4
  %a.load1 = load i32, ptr %a, align 4
  %p.load2 = load ptr, ptr %p, align 8
  %p.load.load3 = load i32, ptr %p.load2, align 4
  %8 = sext i32 %a.load1 to i64
  %9 = sext i32 %p.load.load3 to i64
  %10 = mul i64 %8, %9
  %11 = trunc i64 %10 to i32
  %12 = sext i32 %11 to i64
  %13 = icmp ne i64 %10, %12
  %14 = xor i1 %13, true
  %overflow.condition4 = icmp eq i1 %14, false
  br i1 %overflow.condition4, label %overflow.fail5, label %overflow.success6

overflow.fail5:                                   ; preds = %overflow.success
  call void @_EN3std10assertFailEP4char(ptr @1)
  unreachable

overflow.success6:                                ; preds = %overflow.success
  store i32 %11, ptr %a, align 4
  ret i32 0
}

declare void @_EN3std10assertFailEP4char(ptr)
