
%"Optional<int>" = type { i32, <{ { i32 } }> }

@0 = private unnamed_addr constant [53 x i8] c"Unwrap failed at optional-type-non-pointer.cx:33:18\0A\00", align 1

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
  %5 = add i32 %.load, 1
  store i32 %5, ptr %x, align 4
  %6 = getelementptr inbounds %"Optional<int>", ptr %c, i32 0, i32 1
  %.load15 = load i32, ptr %6, align 4
  %7 = add i32 1, %.load15
  %tag16 = getelementptr inbounds %"Optional<int>", ptr %enum5, i32 0, i32 0
  store i32 1, ptr %tag16, align 4
  %8 = getelementptr inbounds %"Optional<int>", ptr %c, i32 0, i32 1
  %.load17 = load i32, ptr %8, align 4
  %9 = sub i32 0, %.load17
  %10 = insertvalue { i32 } undef, i32 %9, 0
  %associatedValue18 = getelementptr inbounds %"Optional<int>", ptr %enum5, i32 0, i32 1
  store { i32 } %10, ptr %associatedValue18, align 4
  %enum.load19 = load %"Optional<int>", ptr %enum5, align 4
  store %"Optional<int>" %enum.load19, ptr %c, align 4
  br label %if.end

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %c.load20 = load %"Optional<int>", ptr %c, align 4
  %11 = extractvalue %"Optional<int>" %c.load20, 0
  %12 = icmp eq i32 %11, 1
  br i1 %12, label %if.then21, label %if.else23

if.then21:                                        ; preds = %if.end
  %13 = getelementptr inbounds %"Optional<int>", ptr %c, i32 0, i32 1
  %.load22 = load i32, ptr %13, align 4
  %14 = add i32 %.load22, 1
  store i32 %14, ptr %13, align 4
  br label %if.end24

if.else23:                                        ; preds = %if.end
  br label %if.end24

if.end24:                                         ; preds = %if.else23, %if.then21
  %c.load25 = load %"Optional<int>", ptr %c, align 4
  %15 = extractvalue %"Optional<int>" %c.load25, 0
  %16 = icmp eq i32 %15, 1
  %__implicit_unwrap.condition = icmp eq i1 %16, false
  br i1 %__implicit_unwrap.condition, label %__implicit_unwrap.fail, label %__implicit_unwrap.success

__implicit_unwrap.fail:                           ; preds = %if.end24
  call void @_EN3std10assertFailEP4char(ptr @0)
  unreachable

__implicit_unwrap.success:                        ; preds = %if.end24
  store %"Optional<int>" %c.load25, ptr %1, align 4
  %17 = getelementptr inbounds %"Optional<int>", ptr %1, i32 0, i32 1
  %.load26 = load i32, ptr %17, align 4
  call void @_EN4main12takesNonNullE3int(i32 %.load26)
  %c.load27 = load %"Optional<int>", ptr %c, align 4
  %18 = extractvalue %"Optional<int>" %c.load27, 0
  %19 = icmp eq i32 %18, 1
  br i1 %19, label %if.then28, label %if.else30

if.then28:                                        ; preds = %__implicit_unwrap.success
  %20 = getelementptr inbounds %"Optional<int>", ptr %c, i32 0, i32 1
  %.load29 = load i32, ptr %20, align 4
  call void @_EN4main12takesNonNullE3int(i32 %.load29)
  br label %if.end31

if.else30:                                        ; preds = %__implicit_unwrap.success
  br label %if.end31

if.end31:                                         ; preds = %if.else30, %if.then28
  ret void
}

declare void @_EN3std10assertFailEP4char(ptr)

define void @_EN4main12takesNonNullE3int(i32 %i) {
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
