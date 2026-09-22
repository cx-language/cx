
%"Array<int, 3>" = type { [3 x i32] }

@0 = private unnamed_addr constant [43 x i8] c"integer overflow at array-generic.cx:5:21\0A\00", align 1
@1 = private unnamed_addr constant [43 x i8] c"integer overflow at array-generic.cx:5:28\0A\00", align 1

define i32 @main() {
  %a = alloca %"Array<int, 3>", align 8
  call void @_EN3std5ArrayI3intN3_E4initEA3_3int(ptr %a, [3 x i32] [i32 10, i32 20, i32 30])
  %1 = call i32 @_EN3std5ArrayI3intN3_E4sizeE(ptr %a)
  %2 = call ptr @_EN3std5ArrayI3intN3_EixE3int(ptr %a, i32 0)
  %.load = load i32, ptr %2, align 4
  %3 = sext i32 %1 to i64
  %4 = sext i32 %.load to i64
  %5 = add i64 %3, %4
  %6 = trunc i64 %5 to i32
  %7 = sext i32 %6 to i64
  %8 = icmp ne i64 %5, %7
  %9 = xor i1 %8, true
  %overflow.condition = icmp eq i1 %9, false
  br i1 %overflow.condition, label %overflow.fail, label %overflow.success

overflow.fail:                                    ; preds = %0
  call void @_EN3std10assertFailEP4char(ptr @0)
  unreachable

overflow.success:                                 ; preds = %0
  %10 = call ptr @_EN3std5ArrayI3intN3_EixE3int(ptr %a, i32 2)
  %.load1 = load i32, ptr %10, align 4
  %11 = sext i32 %6 to i64
  %12 = sext i32 %.load1 to i64
  %13 = add i64 %11, %12
  %14 = trunc i64 %13 to i32
  %15 = sext i32 %14 to i64
  %16 = icmp ne i64 %13, %15
  %17 = xor i1 %16, true
  %overflow.condition2 = icmp eq i1 %17, false
  br i1 %overflow.condition2, label %overflow.fail3, label %overflow.success4

overflow.fail3:                                   ; preds = %overflow.success
  call void @_EN3std10assertFailEP4char(ptr @1)
  unreachable

overflow.success4:                                ; preds = %overflow.success
  ret i32 %14
}

define void @_EN3std5ArrayI3intN3_E4initEA3_3int(ptr %this, [3 x i32] %elements) {
  %elements1 = alloca [3 x i32], align 4
  store [3 x i32] %elements, ptr %elements1, align 4
  %elements2 = getelementptr inbounds %"Array<int, 3>", ptr %this, i32 0, i32 0
  %elements.load = load [3 x i32], ptr %elements1, align 4
  store [3 x i32] %elements.load, ptr %elements2, align 4
  ret void
}

define i32 @_EN3std5ArrayI3intN3_E4sizeE(ptr %this) {
  ret i32 3
}

define ptr @_EN3std5ArrayI3intN3_EixE3int(ptr %this, i32 %index) {
  %index1 = alloca i32, align 4
  store i32 %index, ptr %index1, align 4
  %elements = getelementptr inbounds %"Array<int, 3>", ptr %this, i32 0, i32 0
  %index.load = load i32, ptr %index1, align 4
  %1 = getelementptr inbounds [3 x i32], ptr %elements, i32 0, i32 %index.load
  ret ptr %1
}

declare void @_EN3std10assertFailEP4char(ptr)
