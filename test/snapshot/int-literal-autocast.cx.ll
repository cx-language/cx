
@0 = private unnamed_addr constant [50 x i8] c"integer overflow at int-literal-autocast.cx:9:11\0A\00", align 1

declare void @foo(i8)

define i32 @main() {
  %b = alloca i64, align 8
  %c = alloca i8, align 1
  call void @foo(i8 1)
  store i64 42, ptr %b, align 4
  store i8 -42, ptr %c, align 1
  %b.load = load i64, ptr %b, align 4
  %1 = add i64 %b.load, 1
  %2 = icmp ult i64 %1, %b.load
  %3 = xor i1 %2, true
  %overflow.condition = icmp eq i1 %3, false
  br i1 %overflow.condition, label %overflow.fail, label %overflow.success

overflow.fail:                                    ; preds = %0
  call void @_EN3std10assertFailEP4char(ptr @0)
  unreachable

overflow.success:                                 ; preds = %0
  store i64 %1, ptr %b, align 4
  ret i32 0
}

declare void @_EN3std10assertFailEP4char(ptr)
