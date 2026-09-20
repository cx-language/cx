
%__closure0 = type { ptr, i32, i32 }
%S = type { i32 }
%__closure2 = type { ptr, i32 }
%__closure1 = type { ptr, i32, ptr }

@0 = private unnamed_addr constant [46 x i8] c"integer overflow at lambda-capturing.cx:5:26\0A\00", align 1
@1 = private unnamed_addr constant [46 x i8] c"integer overflow at lambda-capturing.cx:5:30\0A\00", align 1
@2 = private unnamed_addr constant [47 x i8] c"integer overflow at lambda-capturing.cx:23:26\0A\00", align 1
@3 = private unnamed_addr constant [47 x i8] c"integer overflow at lambda-capturing.cx:13:30\0A\00", align 1
@4 = private unnamed_addr constant [47 x i8] c"integer overflow at lambda-capturing.cx:13:34\0A\00", align 1

define void @_EN4main13capturesParamE3int(i32 %p) {
  %p1 = alloca i32, align 4
  %d = alloca i32, align 4
  %b = alloca %__closure0, align 8
  store i32 %p, ptr %p1, align 4
  store i32 3, ptr %d, align 4
  %p.load = load i32, ptr %p1, align 4
  %1 = insertvalue %__closure0 { ptr @_EN4main9__lambda0E3int, i32 undef, i32 undef }, i32 %p.load, 1
  %d.load = load i32, ptr %d, align 4
  %2 = insertvalue %__closure0 %1, i32 %d.load, 2
  store %__closure0 %2, ptr %b, align 8
  %b.load = load %__closure0, ptr %b, align 8
  %3 = extractvalue %__closure0 %b.load, 0
  %4 = extractvalue %__closure0 %b.load, 1
  %5 = extractvalue %__closure0 %b.load, 2
  %6 = call i32 %3(i32 %4, i32 %5, i32 1)
  ret void
}

define i32 @_EN4main9__lambda0E3int(i32 %__capture_p, i32 %__capture_d, i32 %c) {
  %__capture_p1 = alloca i32, align 4
  %__capture_d2 = alloca i32, align 4
  %c3 = alloca i32, align 4
  store i32 %__capture_p, ptr %__capture_p1, align 4
  store i32 %__capture_d, ptr %__capture_d2, align 4
  store i32 %c, ptr %c3, align 4
  %c.load = load i32, ptr %c3, align 4
  %__capture_p.load = load i32, ptr %__capture_p1, align 4
  %1 = sext i32 %c.load to i128
  %2 = sext i32 %__capture_p.load to i128
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
  %__capture_d.load = load i32, ptr %__capture_d2, align 4
  %8 = sext i32 %4 to i128
  %9 = sext i32 %__capture_d.load to i128
  %10 = add i128 %8, %9
  %11 = trunc i128 %10 to i32
  %12 = sext i32 %11 to i128
  %13 = icmp ne i128 %10, %12
  %14 = xor i1 %13, true
  %overflow.condition4 = icmp eq i1 %14, false
  br i1 %overflow.condition4, label %overflow.fail5, label %overflow.success6

overflow.fail5:                                   ; preds = %overflow.success
  call void @_EN3std10assertFailEP4char(ptr @1)
  unreachable

overflow.success6:                                ; preds = %overflow.success
  ret i32 %11
}

declare void @_EN3std10assertFailEP4char(ptr)

define i32 @main() {
  %s = alloca %S, align 8
  %a = alloca i32, align 4
  %b = alloca %__closure2, align 8
  call void @_EN4main13capturesParamE3int(i32 100)
  call void @_EN4main1S4initE3int(ptr %s, i32 5)
  %1 = call i32 @_EN4main1S3getE3int(ptr %s, i32 1)
  store i32 1, ptr %a, align 4
  %a.load = load i32, ptr %a, align 4
  %2 = insertvalue %__closure2 { ptr @_EN4main9__lambda2E3int, i32 undef }, i32 %a.load, 1
  store %__closure2 %2, ptr %b, align 8
  %b.load = load %__closure2, ptr %b, align 8
  %3 = extractvalue %__closure2 %b.load, 0
  %4 = extractvalue %__closure2 %b.load, 1
  %5 = call i32 %3(i32 %4, i32 2)
  ret i32 0
}

define void @_EN4main1S4initE3int(ptr %this, i32 %d) {
  %d1 = alloca i32, align 4
  store i32 %d, ptr %d1, align 4
  %d2 = getelementptr inbounds %S, ptr %this, i32 0, i32 0
  %d.load = load i32, ptr %d1, align 4
  store i32 %d.load, ptr %d2, align 4
  ret void
}

define i32 @_EN4main1S3getE3int(ptr %this, i32 %c) {
  %c1 = alloca i32, align 4
  %b = alloca %__closure1, align 8
  store i32 %c, ptr %c1, align 4
  %c.load = load i32, ptr %c1, align 4
  %1 = insertvalue %__closure1 { ptr @_EN4main9__lambda1E3int, i32 undef, ptr undef }, i32 %c.load, 1
  %2 = insertvalue %__closure1 %1, ptr %this, 2
  store %__closure1 %2, ptr %b, align 8
  %b.load = load %__closure1, ptr %b, align 8
  %3 = extractvalue %__closure1 %b.load, 0
  %4 = extractvalue %__closure1 %b.load, 1
  %5 = extractvalue %__closure1 %b.load, 2
  %6 = call i32 %3(i32 %4, ptr %5, i32 1)
  ret i32 %6
}

define i32 @_EN4main9__lambda2E3int(i32 %__capture_a, i32 %c) {
  %__capture_a1 = alloca i32, align 4
  %c2 = alloca i32, align 4
  store i32 %__capture_a, ptr %__capture_a1, align 4
  store i32 %c, ptr %c2, align 4
  %c.load = load i32, ptr %c2, align 4
  %__capture_a.load = load i32, ptr %__capture_a1, align 4
  %1 = sext i32 %c.load to i128
  %2 = sext i32 %__capture_a.load to i128
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
  ret i32 %4
}

define i32 @_EN4main9__lambda1E3int(i32 %__capture_c, ptr %__capture_this, i32 %x) {
  %__capture_c1 = alloca i32, align 4
  %x2 = alloca i32, align 4
  store i32 %__capture_c, ptr %__capture_c1, align 4
  store i32 %x, ptr %x2, align 4
  %x.load = load i32, ptr %x2, align 4
  %__capture_c.load = load i32, ptr %__capture_c1, align 4
  %1 = sext i32 %x.load to i128
  %2 = sext i32 %__capture_c.load to i128
  %3 = add i128 %1, %2
  %4 = trunc i128 %3 to i32
  %5 = sext i32 %4 to i128
  %6 = icmp ne i128 %3, %5
  %7 = xor i1 %6, true
  %overflow.condition = icmp eq i1 %7, false
  br i1 %overflow.condition, label %overflow.fail, label %overflow.success

overflow.fail:                                    ; preds = %0
  call void @_EN3std10assertFailEP4char(ptr @3)
  unreachable

overflow.success:                                 ; preds = %0
  %d = getelementptr inbounds %S, ptr %__capture_this, i32 0, i32 0
  %d.load = load i32, ptr %d, align 4
  %8 = sext i32 %4 to i128
  %9 = sext i32 %d.load to i128
  %10 = add i128 %8, %9
  %11 = trunc i128 %10 to i32
  %12 = sext i32 %11 to i128
  %13 = icmp ne i128 %10, %12
  %14 = xor i1 %13, true
  %overflow.condition3 = icmp eq i1 %14, false
  br i1 %overflow.condition3, label %overflow.fail4, label %overflow.success5

overflow.fail4:                                   ; preds = %overflow.success
  call void @_EN3std10assertFailEP4char(ptr @4)
  unreachable

overflow.success5:                                ; preds = %overflow.success
  ret i32 %11
}
