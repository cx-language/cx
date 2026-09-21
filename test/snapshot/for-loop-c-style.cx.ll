
@0 = private unnamed_addr constant [46 x i8] c"integer overflow at for-loop-c-style.cx:6:13\0A\00", align 1

define i32 @main() {
  %sum = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 0, ptr %sum, align 4
  store i32 3, ptr %i, align 4
  br label %loop.condition

loop.condition:                                   ; preds = %loop.increment, %0
  %i.load = load i32, ptr %i, align 4
  %1 = icmp slt i32 %i.load, 9
  br i1 %1, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %sum.load = load i32, ptr %sum, align 4
  %i.load1 = load i32, ptr %i, align 4
  %2 = sext i32 %sum.load to i64
  %3 = sext i32 %i.load1 to i64
  %4 = add i64 %2, %3
  %5 = trunc i64 %4 to i32
  %6 = sext i32 %5 to i64
  %7 = icmp ne i64 %4, %6
  %8 = xor i1 %7, true
  %overflow.condition = icmp eq i1 %8, false
  br i1 %overflow.condition, label %overflow.fail, label %overflow.success

loop.increment:                                   ; preds = %overflow.success
  %i.load2 = load i32, ptr %i, align 4
  %9 = add i32 %i.load2, 1
  store i32 %9, ptr %i, align 4
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  ret i32 0

overflow.fail:                                    ; preds = %loop.body
  call void @_EN3std10assertFailEP4char(ptr @0)
  unreachable

overflow.success:                                 ; preds = %loop.body
  store i32 %5, ptr %sum, align 4
  br label %loop.increment
}

declare void @_EN3std10assertFailEP4char(ptr)
