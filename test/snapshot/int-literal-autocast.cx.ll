
@0 = private unnamed_addr constant [50 x i8] c"integer overflow at int-literal-autocast.cx:9:11\0A\00", align 1

declare void @foo(i8)

define i32 @main() {
  %b = alloca i64, align 8
  %c = alloca i8, align 1
  call void @foo(i8 1)
  store i64 42, ptr %b, align 4
  store i8 -42, ptr %c, align 1
  %b.load = load i64, ptr %b, align 4
  %1 = zext i64 %b.load to i128
  %2 = add i128 %1, 1
  %3 = trunc i128 %2 to i64
  %4 = zext i64 %3 to i128
  %5 = icmp ne i128 %2, %4
  %6 = xor i1 %5, true
  %overflow.condition = icmp eq i1 %6, false
  br i1 %overflow.condition, label %overflow.fail, label %overflow.success

overflow.fail:                                    ; preds = %0
  call void @_EN3std10assertFailEP4char(ptr @0)
  unreachable

overflow.success:                                 ; preds = %0
  store i64 %3, ptr %b, align 4
  ret i32 0
}

declare void @_EN3std10assertFailEP4char(ptr)
