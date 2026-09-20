
@0 = private unnamed_addr constant [65 x i8] c"integer overflow at array-literal-non-constant-elements.cx:4:17\0A\00", align 1
@1 = private unnamed_addr constant [65 x i8] c"integer overflow at array-literal-non-constant-elements.cx:4:28\0A\00", align 1
@2 = private unnamed_addr constant [65 x i8] c"integer overflow at array-literal-non-constant-elements.cx:8:18\0A\00", align 1
@3 = private unnamed_addr constant [65 x i8] c"integer overflow at array-literal-non-constant-elements.cx:8:29\0A\00", align 1

define [2 x i32] @_EN4main1fE3int3int(i32 %foo, i32 %bar) {
  %foo1 = alloca i32, align 4
  %bar2 = alloca i32, align 4
  store i32 %foo, ptr %foo1, align 4
  store i32 %bar, ptr %bar2, align 4
  %foo.load = load i32, ptr %foo1, align 4
  %bar.load = load i32, ptr %bar2, align 4
  %1 = sext i32 %foo.load to i128
  %2 = sext i32 %bar.load to i128
  %3 = add i128 %1, %2
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
  %8 = insertvalue [2 x i32] undef, i32 %4, 0
  %foo.load3 = load i32, ptr %foo1, align 4
  %bar.load4 = load i32, ptr %bar2, align 4
  %9 = sext i32 %foo.load3 to i128
  %10 = sext i32 %bar.load4 to i128
  %11 = sub i128 %9, %10
  %12 = trunc i128 %11 to i32
  %13 = sext i32 %12 to i128
  %14 = icmp ne i128 %11, %13
  %15 = xor i1 %14, true
  %overflow.condition5 = icmp eq i1 %15, false
  br i1 %overflow.condition5, label %overflow.fail6, label %overflow.success7

overflow.fail6:                                   ; preds = %overflow.success
  call void @_EN3std10assertFailEP4char(ptr @1)
  unreachable

overflow.success7:                                ; preds = %overflow.success
  %16 = insertvalue [2 x i32] %8, i32 %12, 1
  ret [2 x i32] %16
}

declare void @_EN3std10assertFailEP4char(ptr)

define [2 x i32] @_EN4main1gE3int3int(i32 %foo, i32 %bar) {
  %foo1 = alloca i32, align 4
  %bar2 = alloca i32, align 4
  %c = alloca [2 x i32], align 4
  store i32 %foo, ptr %foo1, align 4
  store i32 %bar, ptr %bar2, align 4
  %foo.load = load i32, ptr %foo1, align 4
  %bar.load = load i32, ptr %bar2, align 4
  %1 = sext i32 %foo.load to i128
  %2 = sext i32 %bar.load to i128
  %3 = add i128 %1, %2
  %4 = trunc i128 %3 to i32
  %5 = sext i32 %4 to i128
  %6 = icmp ne i128 %3, %5
  %7 = xor i1 %6, true
  %overflow.condition = icmp eq i1 %7, false
  br i1 %overflow.condition, label %overflow.fail, label %overflow.success

overflow.fail:                                    ; preds = %0
  call void @_EN3std10assertFailEP4char(ptr @2)
  unreachable

overflow.success:                                 ; preds = %0
  %8 = insertvalue [2 x i32] undef, i32 %4, 0
  %foo.load3 = load i32, ptr %foo1, align 4
  %bar.load4 = load i32, ptr %bar2, align 4
  %9 = sext i32 %foo.load3 to i128
  %10 = sext i32 %bar.load4 to i128
  %11 = sub i128 %9, %10
  %12 = trunc i128 %11 to i32
  %13 = sext i32 %12 to i128
  %14 = icmp ne i128 %11, %13
  %15 = xor i1 %14, true
  %overflow.condition5 = icmp eq i1 %15, false
  br i1 %overflow.condition5, label %overflow.fail6, label %overflow.success7

overflow.fail6:                                   ; preds = %overflow.success
  call void @_EN3std10assertFailEP4char(ptr @3)
  unreachable

overflow.success7:                                ; preds = %overflow.success
  %16 = insertvalue [2 x i32] %8, i32 %12, 1
  store [2 x i32] %16, ptr %c, align 4
  %c.load = load [2 x i32], ptr %c, align 4
  ret [2 x i32] %c.load
}
