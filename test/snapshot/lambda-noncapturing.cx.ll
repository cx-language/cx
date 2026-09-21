
@0 = private unnamed_addr constant [49 x i8] c"integer overflow at lambda-noncapturing.cx:8:29\0A\00", align 1
@1 = private unnamed_addr constant [49 x i8] c"integer overflow at lambda-noncapturing.cx:9:31\0A\00", align 1
@2 = private unnamed_addr constant [50 x i8] c"integer overflow at lambda-noncapturing.cx:12:22\0A\00", align 1
@3 = private unnamed_addr constant [50 x i8] c"integer overflow at lambda-noncapturing.cx:14:22\0A\00", align 1
@4 = private unnamed_addr constant [50 x i8] c"integer overflow at lambda-noncapturing.cx:18:19\0A\00", align 1

define i32 @main() {
  call void @_EN4main3fooI3intEEF3int3int_3int(ptr @_EN4main9__lambda0E3int3int)
  call void @_EN4main3fooI3intEEF3int3int_3int(ptr @_EN4main9__lambda1E3int3int)
  call void @_EN4main3fooI3intEEF3int3int_3int(ptr @_EN4main9__lambda2E3int3int)
  call void @_EN4main3fooI3intEEF3int3int_3int(ptr @_EN4main9__lambda3E3int3int)
  call void @_EN4main3fooI4voidEEF3int3int_4void(ptr @_EN4main9__lambda4E3int3int)
  ret i32 0
}

define void @_EN4main3fooI3intEEF3int3int_3int(ptr %a) {
  %a1 = alloca ptr, align 8
  store ptr %a, ptr %a1, align 8
  %a.load = load ptr, ptr %a1, align 8
  %1 = call i32 %a.load(i32 1, i32 2)
  ret void
}

define i32 @_EN4main9__lambda0E3int3int(i32 %a, i32 %b) {
  %a1 = alloca i32, align 4
  %b2 = alloca i32, align 4
  store i32 %a, ptr %a1, align 4
  store i32 %b, ptr %b2, align 4
  %a.load = load i32, ptr %a1, align 4
  %b.load = load i32, ptr %b2, align 4
  %1 = sext i32 %a.load to i64
  %2 = sext i32 %b.load to i64
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
  ret i32 %4
}

define i32 @_EN4main9__lambda1E3int3int(i32 %a, i32 %b) {
  %a1 = alloca i32, align 4
  %b2 = alloca i32, align 4
  store i32 %a, ptr %a1, align 4
  store i32 %b, ptr %b2, align 4
  %a.load = load i32, ptr %a1, align 4
  %b.load = load i32, ptr %b2, align 4
  %1 = sext i32 %a.load to i64
  %2 = sext i32 %b.load to i64
  %3 = add i64 %1, %2
  %4 = trunc i64 %3 to i32
  %5 = sext i32 %4 to i64
  %6 = icmp ne i64 %3, %5
  %7 = xor i1 %6, true
  %overflow.condition = icmp eq i1 %7, false
  br i1 %overflow.condition, label %overflow.fail, label %overflow.success

overflow.fail:                                    ; preds = %0
  call void @_EN3std10assertFailEP4char(ptr @1)
  unreachable

overflow.success:                                 ; preds = %0
  ret i32 %4
}

define i32 @_EN4main9__lambda2E3int3int(i32 %a, i32 %b) {
  %a1 = alloca i32, align 4
  %b2 = alloca i32, align 4
  store i32 %a, ptr %a1, align 4
  store i32 %b, ptr %b2, align 4
  %a.load = load i32, ptr %a1, align 4
  %b.load = load i32, ptr %b2, align 4
  %1 = icmp sgt i32 %a.load, %b.load
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  %a.load3 = load i32, ptr %a1, align 4
  %b.load4 = load i32, ptr %b2, align 4
  %2 = sext i32 %a.load3 to i64
  %3 = sext i32 %b.load4 to i64
  %4 = add i64 %2, %3
  %5 = trunc i64 %4 to i32
  %6 = sext i32 %5 to i64
  %7 = icmp ne i64 %4, %6
  %8 = xor i1 %7, true
  %overflow.condition = icmp eq i1 %8, false
  br i1 %overflow.condition, label %overflow.fail, label %overflow.success

if.else:                                          ; preds = %0
  %a.load5 = load i32, ptr %a1, align 4
  %b.load6 = load i32, ptr %b2, align 4
  %9 = sext i32 %a.load5 to i64
  %10 = sext i32 %b.load6 to i64
  %11 = sub i64 %9, %10
  %12 = trunc i64 %11 to i32
  %13 = sext i32 %12 to i64
  %14 = icmp ne i64 %11, %13
  %15 = xor i1 %14, true
  %overflow.condition7 = icmp eq i1 %15, false
  br i1 %overflow.condition7, label %overflow.fail8, label %overflow.success9

if.end:                                           ; No predecessors!
  unreachable

overflow.fail:                                    ; preds = %if.then
  call void @_EN3std10assertFailEP4char(ptr @2)
  unreachable

overflow.success:                                 ; preds = %if.then
  ret i32 %5

overflow.fail8:                                   ; preds = %if.else
  call void @_EN3std10assertFailEP4char(ptr @3)
  unreachable

overflow.success9:                                ; preds = %if.else
  ret i32 %12
}

define i32 @_EN4main9__lambda3E3int3int(i32 %a, i32 %b) {
  %a1 = alloca i32, align 4
  %b2 = alloca i32, align 4
  %x = alloca i32, align 4
  store i32 %a, ptr %a1, align 4
  store i32 %b, ptr %b2, align 4
  %a.load = load i32, ptr %a1, align 4
  %b.load = load i32, ptr %b2, align 4
  %1 = sext i32 %a.load to i64
  %2 = sext i32 %b.load to i64
  %3 = add i64 %1, %2
  %4 = trunc i64 %3 to i32
  %5 = sext i32 %4 to i64
  %6 = icmp ne i64 %3, %5
  %7 = xor i1 %6, true
  %overflow.condition = icmp eq i1 %7, false
  br i1 %overflow.condition, label %overflow.fail, label %overflow.success

overflow.fail:                                    ; preds = %0
  call void @_EN3std10assertFailEP4char(ptr @4)
  unreachable

overflow.success:                                 ; preds = %0
  store i32 %4, ptr %x, align 4
  %x.load = load i32, ptr %x, align 4
  ret i32 %x.load
}

define void @_EN4main3fooI4voidEEF3int3int_4void(ptr %a) {
  %a1 = alloca ptr, align 8
  store ptr %a, ptr %a1, align 8
  %a.load = load ptr, ptr %a1, align 8
  call void %a.load(i32 1, i32 2)
  ret void
}

define void @_EN4main9__lambda4E3int3int(i32 %a, i32 %b) {
  %a1 = alloca i32, align 4
  %b2 = alloca i32, align 4
  store i32 %a, ptr %a1, align 4
  store i32 %b, ptr %b2, align 4
  %a.load = load i32, ptr %a1, align 4
  %b.load = load i32, ptr %b2, align 4
  %1 = icmp eq i32 %a.load, %b.load
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  ret void

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else
  ret void
}

declare void @_EN3std10assertFailEP4char(ptr)
