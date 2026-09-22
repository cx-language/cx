
%0 = type { { i32 } }
%"Optional<int>" = type { i32, %0 }

@0 = private unnamed_addr constant [56 x i8] c"integer overflow at optional-type-non-pointer.cx:26:19\0A\00", align 1
@1 = private unnamed_addr constant [56 x i8] c"integer overflow at optional-type-non-pointer.cx:27:15\0A\00", align 1
@2 = private unnamed_addr constant [53 x i8] c"Unwrap failed at optional-type-non-pointer.cx:33:18\0A\00", align 1

define i32 @main() {
  %a = alloca %"Optional<int>", align 8
  %enum = alloca %"Optional<int>", align 8
  %b = alloca %"Optional<int>", align 8
  %enum1 = alloca %"Optional<int>", align 8
  %c = alloca %"Optional<int>", align 8
  %enum2 = alloca %"Optional<int>", align 8
  %d = alloca %"Optional<int>", align 8
  %enum3 = alloca %"Optional<int>", align 8
  %tag = getelementptr inbounds %"Optional<int>", ptr %enum, i32 0, i32 0
  store i32 0, ptr %tag, align 4
  %enum.load = load %"Optional<int>", ptr %enum, align 4
  store %"Optional<int>" %enum.load, ptr %a, align 4
  %tag4 = getelementptr inbounds %"Optional<int>", ptr %enum1, i32 0, i32 0
  store i32 1, ptr %tag4, align 4
  %associatedValue = getelementptr inbounds %"Optional<int>", ptr %enum1, i32 0, i32 1
  store { i32 } { i32 2 }, ptr %associatedValue, align 4
  %enum.load5 = load %"Optional<int>", ptr %enum1, align 4
  store %"Optional<int>" %enum.load5, ptr %b, align 4
  %tag6 = getelementptr inbounds %"Optional<int>", ptr %enum2, i32 0, i32 0
  store i32 1, ptr %tag6, align 4
  %associatedValue7 = getelementptr inbounds %"Optional<int>", ptr %enum2, i32 0, i32 1
  store { i32 } { i32 3 }, ptr %associatedValue7, align 4
  %enum.load8 = load %"Optional<int>", ptr %enum2, align 4
  store %"Optional<int>" %enum.load8, ptr %c, align 4
  %tag9 = getelementptr inbounds %"Optional<int>", ptr %enum3, i32 0, i32 0
  store i32 0, ptr %tag9, align 4
  %enum.load10 = load %"Optional<int>", ptr %enum3, align 4
  store %"Optional<int>" %enum.load10, ptr %d, align 4
  %d.load = load %"Optional<int>", ptr %d, align 4
  %1 = extractvalue %"Optional<int>" %d.load, 0
  %2 = icmp eq i32 %1, 1
  %3 = xor i1 %2, true
  br i1 %3, label %if.then, label %if.else

if.then:                                          ; preds = %0
  br label %if.end

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %d.load11 = load %"Optional<int>", ptr %d, align 4
  %4 = extractvalue %"Optional<int>" %d.load11, 0
  %5 = icmp eq i32 %4, 1
  br i1 %5, label %if.then12, label %if.else13

if.then12:                                        ; preds = %if.end
  br label %if.end14

if.else13:                                        ; preds = %if.end
  br label %if.end14

if.end14:                                         ; preds = %if.else13, %if.then12
  %d.load15 = load %"Optional<int>", ptr %d, align 4
  %6 = extractvalue %"Optional<int>" %d.load15, 0
  %7 = icmp eq i32 %6, 1
  %8 = xor i1 %7, true
  br i1 %8, label %if.then16, label %if.else17

if.then16:                                        ; preds = %if.end14
  br label %if.end18

if.else17:                                        ; preds = %if.end14
  br label %if.end18

if.end18:                                         ; preds = %if.else17, %if.then16
  %d.load19 = load %"Optional<int>", ptr %d, align 4
  %9 = extractvalue %"Optional<int>" %d.load19, 0
  %10 = icmp eq i32 %9, 1
  br i1 %10, label %if.then20, label %if.else21

if.then20:                                        ; preds = %if.end18
  br label %if.end22

if.else21:                                        ; preds = %if.end18
  br label %if.end22

if.end22:                                         ; preds = %if.else21, %if.then20
  %d.load23 = load %"Optional<int>", ptr %d, align 4
  %11 = extractvalue %"Optional<int>" %d.load23, 0
  %12 = icmp eq i32 %11, 1
  br i1 %12, label %if.then24, label %if.else25

if.then24:                                        ; preds = %if.end22
  br label %if.end26

if.else25:                                        ; preds = %if.end22
  br label %if.end26

if.end26:                                         ; preds = %if.else25, %if.then24
  %d.load27 = load %"Optional<int>", ptr %d, align 4
  %13 = extractvalue %"Optional<int>" %d.load27, 0
  %14 = icmp eq i32 %13, 1
  %15 = xor i1 %14, true
  br i1 %15, label %if.then28, label %if.else29

if.then28:                                        ; preds = %if.end26
  br label %if.end30

if.else29:                                        ; preds = %if.end26
  br label %if.end30

if.end30:                                         ; preds = %if.else29, %if.then28
  ret i32 0
}

define void @_EN4main1fE() {
  %a = alloca %"Optional<int>", align 8
  %enum = alloca %"Optional<int>", align 8
  %b = alloca %"Optional<int>", align 8
  %enum1 = alloca %"Optional<int>", align 8
  %c = alloca %"Optional<int>", align 8
  %enum2 = alloca %"Optional<int>", align 8
  %d = alloca %"Optional<int>", align 8
  %enum3 = alloca %"Optional<int>", align 8
  %enum4 = alloca %"Optional<int>", align 8
  %x = alloca i32, align 4
  %enum5 = alloca %"Optional<int>", align 8
  %1 = alloca %"Optional<int>", align 8
  %tag = getelementptr inbounds %"Optional<int>", ptr %enum, i32 0, i32 0
  store i32 0, ptr %tag, align 4
  %enum.load = load %"Optional<int>", ptr %enum, align 4
  store %"Optional<int>" %enum.load, ptr %a, align 4
  %tag6 = getelementptr inbounds %"Optional<int>", ptr %enum1, i32 0, i32 0
  store i32 1, ptr %tag6, align 4
  %associatedValue = getelementptr inbounds %"Optional<int>", ptr %enum1, i32 0, i32 1
  store { i32 } { i32 2 }, ptr %associatedValue, align 4
  %enum.load7 = load %"Optional<int>", ptr %enum1, align 4
  store %"Optional<int>" %enum.load7, ptr %b, align 4
  %tag8 = getelementptr inbounds %"Optional<int>", ptr %enum2, i32 0, i32 0
  store i32 1, ptr %tag8, align 4
  %associatedValue9 = getelementptr inbounds %"Optional<int>", ptr %enum2, i32 0, i32 1
  store { i32 } { i32 3 }, ptr %associatedValue9, align 4
  %enum.load10 = load %"Optional<int>", ptr %enum2, align 4
  store %"Optional<int>" %enum.load10, ptr %c, align 4
  %tag11 = getelementptr inbounds %"Optional<int>", ptr %enum3, i32 0, i32 0
  store i32 0, ptr %tag11, align 4
  %enum.load12 = load %"Optional<int>", ptr %enum3, align 4
  store %"Optional<int>" %enum.load12, ptr %d, align 4
  %tag13 = getelementptr inbounds %"Optional<int>", ptr %enum4, i32 0, i32 0
  store i32 0, ptr %tag13, align 4
  %enum.load14 = load %"Optional<int>", ptr %enum4, align 4
  store %"Optional<int>" %enum.load14, ptr %c, align 4
  %c.load = load %"Optional<int>", ptr %c, align 4
  %2 = extractvalue %"Optional<int>" %c.load, 0
  %3 = icmp eq i32 %2, 1
  br i1 %3, label %if.then, label %if.else

if.then:                                          ; preds = %0
  %4 = getelementptr inbounds %"Optional<int>", ptr %c, i32 0, i32 1
  %.load = load i32, ptr %4, align 4
  %5 = sext i32 %.load to i64
  %6 = add i64 %5, 1
  %7 = trunc i64 %6 to i32
  %8 = sext i32 %7 to i64
  %9 = icmp ne i64 %6, %8
  %10 = xor i1 %9, true
  %overflow.condition = icmp eq i1 %10, false
  br i1 %overflow.condition, label %overflow.fail, label %overflow.success

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %overflow.success19, %if.else
  %c.load15 = load %"Optional<int>", ptr %c, align 4
  %11 = extractvalue %"Optional<int>" %c.load15, 0
  %12 = icmp eq i32 %11, 1
  br i1 %12, label %if.then24, label %if.else26

overflow.fail:                                    ; preds = %if.then
  call void @_EN3std10assertFailEP4char(ptr @0)
  unreachable

overflow.success:                                 ; preds = %if.then
  store i32 %7, ptr %x, align 4
  %13 = getelementptr inbounds %"Optional<int>", ptr %c, i32 0, i32 1
  %.load16 = load i32, ptr %13, align 4
  %14 = sext i32 %.load16 to i64
  %15 = add i64 1, %14
  %16 = trunc i64 %15 to i32
  %17 = sext i32 %16 to i64
  %18 = icmp ne i64 %15, %17
  %19 = xor i1 %18, true
  %overflow.condition17 = icmp eq i1 %19, false
  br i1 %overflow.condition17, label %overflow.fail18, label %overflow.success19

overflow.fail18:                                  ; preds = %overflow.success
  call void @_EN3std10assertFailEP4char(ptr @1)
  unreachable

overflow.success19:                               ; preds = %overflow.success
  %tag20 = getelementptr inbounds %"Optional<int>", ptr %enum5, i32 0, i32 0
  store i32 1, ptr %tag20, align 4
  %20 = getelementptr inbounds %"Optional<int>", ptr %c, i32 0, i32 1
  %.load21 = load i32, ptr %20, align 4
  %21 = sub i32 0, %.load21
  %22 = insertvalue { i32 } undef, i32 %21, 0
  %associatedValue22 = getelementptr inbounds %"Optional<int>", ptr %enum5, i32 0, i32 1
  store { i32 } %22, ptr %associatedValue22, align 4
  %enum.load23 = load %"Optional<int>", ptr %enum5, align 4
  store %"Optional<int>" %enum.load23, ptr %c, align 4
  br label %if.end

if.then24:                                        ; preds = %if.end
  %23 = getelementptr inbounds %"Optional<int>", ptr %c, i32 0, i32 1
  %.load25 = load i32, ptr %23, align 4
  %24 = add i32 %.load25, 1
  store i32 %24, ptr %23, align 4
  br label %if.end27

if.else26:                                        ; preds = %if.end
  br label %if.end27

if.end27:                                         ; preds = %if.else26, %if.then24
  %c.load28 = load %"Optional<int>", ptr %c, align 4
  %25 = extractvalue %"Optional<int>" %c.load28, 0
  %26 = icmp eq i32 %25, 1
  %__implicit_unwrap.condition = icmp eq i1 %26, false
  br i1 %__implicit_unwrap.condition, label %__implicit_unwrap.fail, label %__implicit_unwrap.success

__implicit_unwrap.fail:                           ; preds = %if.end27
  call void @_EN3std10assertFailEP4char(ptr @2)
  unreachable

__implicit_unwrap.success:                        ; preds = %if.end27
  store %"Optional<int>" %c.load28, ptr %1, align 4
  %27 = getelementptr inbounds %"Optional<int>", ptr %1, i32 0, i32 1
  %.load29 = load i32, ptr %27, align 4
  call void @_EN4main12takesNonNullE3int(i32 %.load29)
  %c.load30 = load %"Optional<int>", ptr %c, align 4
  %28 = extractvalue %"Optional<int>" %c.load30, 0
  %29 = icmp eq i32 %28, 1
  br i1 %29, label %if.then31, label %if.else33

if.then31:                                        ; preds = %__implicit_unwrap.success
  %30 = getelementptr inbounds %"Optional<int>", ptr %c, i32 0, i32 1
  %.load32 = load i32, ptr %30, align 4
  call void @_EN4main12takesNonNullE3int(i32 %.load32)
  br label %if.end34

if.else33:                                        ; preds = %__implicit_unwrap.success
  br label %if.end34

if.end34:                                         ; preds = %if.else33, %if.then31
  ret void
}

declare void @_EN3std10assertFailEP4char(ptr)

define void @_EN4main12takesNonNullE3int(i32 %i) {
  %i1 = alloca i32, align 4
  store i32 %i, ptr %i1, align 4
  ret void
}

define void @_EN4main1gE() {
  %p = alloca ptr, align 8
  store ptr null, ptr %p, align 8
  %p.load = load ptr, ptr %p, align 8
  %1 = icmp eq ptr %p.load, null
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  br label %if.end

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  ret void
}
