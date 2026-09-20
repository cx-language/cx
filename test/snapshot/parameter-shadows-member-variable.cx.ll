
%S = type { i32 }

@0 = private unnamed_addr constant [63 x i8] c"integer overflow at parameter-shadows-member-variable.cx:7:17\0A\00", align 1

define i32 @main() {
  %1 = alloca %S, align 8
  call void @_EN4main1S4initE(ptr %1)
  call void @_EN4main1S3fooE3int(ptr %1, i32 30)
  ret i32 0
}

define void @_EN4main1S4initE(ptr %this) {
  ret void
}

define void @_EN4main1S3fooE3int(ptr %this, i32 %bar) {
  %bar1 = alloca i32, align 4
  store i32 %bar, ptr %bar1, align 4
  %bar.load = load i32, ptr %bar1, align 4
  %1 = sext i32 %bar.load to i128
  %2 = add i128 %1, 42
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
  ret void
}

declare void @_EN3std10assertFailEP4char(ptr)
