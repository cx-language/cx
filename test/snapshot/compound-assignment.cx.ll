
@0 = private unnamed_addr constant [48 x i8] c"integer overflow at compound-assignment.cx:5:7\0A\00", align 1
@1 = private unnamed_addr constant [48 x i8] c"integer overflow at compound-assignment.cx:6:7\0A\00", align 1
@2 = private unnamed_addr constant [48 x i8] c"integer overflow at compound-assignment.cx:7:7\0A\00", align 1

define i32 @main() {
  %i = alloca i32, align 4
  store i32 0, ptr %i, align 4
  %i.load = load i32, ptr %i, align 4
  %1 = sext i32 %i.load to i128
  %2 = add i128 %1, 1
  %3 = trunc i128 %2 to i32
  %4 = sext i32 %3 to i128
  %5 = icmp ne i128 %2, %4
  %6 = xor i1 %5, true
  %overflow.condition = icmp eq i1 %6, false
  br i1 %overflow.condition, label %overflow.fail, label %overflow.success

overflow.fail:                                    ; preds = %0
  call void @_EN3std10assertFailEP4char(ptr @0)
  unreachable

overflow.success:                                 ; preds = %0
  store i32 %3, ptr %i, align 4
  %i.load1 = load i32, ptr %i, align 4
  %7 = sext i32 %i.load1 to i128
  %8 = sub i128 %7, 1
  %9 = trunc i128 %8 to i32
  %10 = sext i32 %9 to i128
  %11 = icmp ne i128 %8, %10
  %12 = xor i1 %11, true
  %overflow.condition2 = icmp eq i1 %12, false
  br i1 %overflow.condition2, label %overflow.fail3, label %overflow.success4

overflow.fail3:                                   ; preds = %overflow.success
  call void @_EN3std10assertFailEP4char(ptr @1)
  unreachable

overflow.success4:                                ; preds = %overflow.success
  store i32 %9, ptr %i, align 4
  %i.load5 = load i32, ptr %i, align 4
  %13 = sext i32 %i.load5 to i128
  %14 = mul i128 %13, 1
  %15 = trunc i128 %14 to i32
  %16 = sext i32 %15 to i128
  %17 = icmp ne i128 %14, %16
  %18 = xor i1 %17, true
  %overflow.condition6 = icmp eq i1 %18, false
  br i1 %overflow.condition6, label %overflow.fail7, label %overflow.success8

overflow.fail7:                                   ; preds = %overflow.success4
  call void @_EN3std10assertFailEP4char(ptr @2)
  unreachable

overflow.success8:                                ; preds = %overflow.success4
  store i32 %15, ptr %i, align 4
  %i.load9 = load i32, ptr %i, align 4
  %19 = sdiv i32 %i.load9, 1
  store i32 %19, ptr %i, align 4
  %i.load10 = load i32, ptr %i, align 4
  %20 = and i32 %i.load10, 1
  store i32 %20, ptr %i, align 4
  %i.load11 = load i32, ptr %i, align 4
  %21 = or i32 %i.load11, 1
  store i32 %21, ptr %i, align 4
  %i.load12 = load i32, ptr %i, align 4
  %22 = xor i32 %i.load12, 1
  store i32 %22, ptr %i, align 4
  %i.load13 = load i32, ptr %i, align 4
  %23 = shl i32 %i.load13, 1
  store i32 %23, ptr %i, align 4
  %i.load14 = load i32, ptr %i, align 4
  %24 = ashr i32 %i.load14, 1
  store i32 %24, ptr %i, align 4
  ret i32 0
}

declare void @_EN3std10assertFailEP4char(ptr)
