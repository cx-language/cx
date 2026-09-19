
%"Optional<int>" = type { i1, i32 }

@0 = private unnamed_addr constant [53 x i8] c"Unwrap failed at optional-type-non-pointer.cx:33:18\0A\00", align 1

define i32 @main() {
  %a = alloca %"Optional<int>", align 8
  %1 = alloca %"Optional<int>", align 8
  %b = alloca %"Optional<int>", align 8
  %c = alloca %"Optional<int>", align 8
  %2 = alloca %"Optional<int>", align 8
  %d = alloca %"Optional<int>", align 8
  call void @_EN3std8OptionalI3intE4initE(ptr %1)
  %.load = load %"Optional<int>", ptr %1, align 4
  store %"Optional<int>" %.load, ptr %a, align 4
  call void @_EN3std8OptionalI3intE4initE3int(ptr %b, i32 2)
  call void @_EN3std8OptionalI3intE4initE3int(ptr %2, i32 3)
  %.load1 = load %"Optional<int>", ptr %2, align 4
  store %"Optional<int>" %.load1, ptr %c, align 4
  call void @_EN3std8OptionalI3intE4initE(ptr %d)
  %d.load = load %"Optional<int>", ptr %d, align 4
  %3 = extractvalue %"Optional<int>" %d.load, 0
  %4 = xor i1 %3, true
  br i1 %4, label %if.then, label %if.else

if.then:                                          ; preds = %0
  br label %if.end

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %d.load2 = load %"Optional<int>", ptr %d, align 4
  %5 = extractvalue %"Optional<int>" %d.load2, 0
  br i1 %5, label %if.then3, label %if.else4

if.then3:                                         ; preds = %if.end
  br label %if.end5

if.else4:                                         ; preds = %if.end
  br label %if.end5

if.end5:                                          ; preds = %if.else4, %if.then3
  %d.load6 = load %"Optional<int>", ptr %d, align 4
  %6 = extractvalue %"Optional<int>" %d.load6, 0
  %7 = xor i1 %6, true
  br i1 %7, label %if.then7, label %if.else8

if.then7:                                         ; preds = %if.end5
  br label %if.end9

if.else8:                                         ; preds = %if.end5
  br label %if.end9

if.end9:                                          ; preds = %if.else8, %if.then7
  %d.load10 = load %"Optional<int>", ptr %d, align 4
  %8 = extractvalue %"Optional<int>" %d.load10, 0
  br i1 %8, label %if.then11, label %if.else12

if.then11:                                        ; preds = %if.end9
  br label %if.end13

if.else12:                                        ; preds = %if.end9
  br label %if.end13

if.end13:                                         ; preds = %if.else12, %if.then11
  %d.load14 = load %"Optional<int>", ptr %d, align 4
  %9 = extractvalue %"Optional<int>" %d.load14, 0
  br i1 %9, label %if.then15, label %if.else16

if.then15:                                        ; preds = %if.end13
  br label %if.end17

if.else16:                                        ; preds = %if.end13
  br label %if.end17

if.end17:                                         ; preds = %if.else16, %if.then15
  %d.load18 = load %"Optional<int>", ptr %d, align 4
  %10 = extractvalue %"Optional<int>" %d.load18, 0
  %11 = xor i1 %10, true
  br i1 %11, label %if.then19, label %if.else20

if.then19:                                        ; preds = %if.end17
  br label %if.end21

if.else20:                                        ; preds = %if.end17
  br label %if.end21

if.end21:                                         ; preds = %if.else20, %if.then19
  ret i32 0
}

define void @_EN3std8OptionalI3intE4initE(ptr %this) {
  %hasValue = getelementptr inbounds %"Optional<int>", ptr %this, i32 0, i32 0
  store i1 false, ptr %hasValue, align 1
  ret void
}

define void @_EN3std8OptionalI3intE4initE3int(ptr %this, i32 %value) {
  %hasValue = getelementptr inbounds %"Optional<int>", ptr %this, i32 0, i32 0
  store i1 true, ptr %hasValue, align 1
  %value1 = getelementptr inbounds %"Optional<int>", ptr %this, i32 0, i32 1
  store i32 %value, ptr %value1, align 4
  ret void
}

define void @_EN4main1fE() {
  %a = alloca %"Optional<int>", align 8
  %1 = alloca %"Optional<int>", align 8
  %b = alloca %"Optional<int>", align 8
  %c = alloca %"Optional<int>", align 8
  %2 = alloca %"Optional<int>", align 8
  %d = alloca %"Optional<int>", align 8
  %3 = alloca %"Optional<int>", align 8
  %x = alloca i32, align 4
  %4 = alloca %"Optional<int>", align 8
  call void @_EN3std8OptionalI3intE4initE(ptr %1)
  %.load = load %"Optional<int>", ptr %1, align 4
  store %"Optional<int>" %.load, ptr %a, align 4
  call void @_EN3std8OptionalI3intE4initE3int(ptr %b, i32 2)
  call void @_EN3std8OptionalI3intE4initE3int(ptr %2, i32 3)
  %.load1 = load %"Optional<int>", ptr %2, align 4
  store %"Optional<int>" %.load1, ptr %c, align 4
  call void @_EN3std8OptionalI3intE4initE(ptr %d)
  call void @_EN3std8OptionalI3intE4initE(ptr %3)
  %.load2 = load %"Optional<int>", ptr %3, align 4
  store %"Optional<int>" %.load2, ptr %c, align 4
  %c.load = load %"Optional<int>", ptr %c, align 4
  %5 = extractvalue %"Optional<int>" %c.load, 0
  br i1 %5, label %if.then, label %if.else

if.then:                                          ; preds = %0
  %6 = getelementptr inbounds %"Optional<int>", ptr %c, i32 0, i32 1
  %.load3 = load i32, ptr %6, align 4
  %7 = add i32 %.load3, 1
  store i32 %7, ptr %x, align 4
  %8 = getelementptr inbounds %"Optional<int>", ptr %c, i32 0, i32 1
  %.load4 = load i32, ptr %8, align 4
  %9 = add i32 1, %.load4
  %10 = getelementptr inbounds %"Optional<int>", ptr %c, i32 0, i32 1
  %.load5 = load i32, ptr %10, align 4
  %11 = sub i32 0, %.load5
  call void @_EN3std8OptionalI3intE4initE3int(ptr %4, i32 %11)
  %.load6 = load %"Optional<int>", ptr %4, align 4
  store %"Optional<int>" %.load6, ptr %c, align 4
  br label %if.end

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %c.load7 = load %"Optional<int>", ptr %c, align 4
  %12 = extractvalue %"Optional<int>" %c.load7, 0
  br i1 %12, label %if.then8, label %if.else10

if.then8:                                         ; preds = %if.end
  %13 = getelementptr inbounds %"Optional<int>", ptr %c, i32 0, i32 1
  %.load9 = load i32, ptr %13, align 4
  %14 = add i32 %.load9, 1
  store i32 %14, ptr %13, align 4
  br label %if.end11

if.else10:                                        ; preds = %if.end
  br label %if.end11

if.end11:                                         ; preds = %if.else10, %if.then8
  %c.load12 = load %"Optional<int>", ptr %c, align 4
  %15 = extractvalue %"Optional<int>" %c.load12, 0
  %__implicit_unwrap.condition = icmp eq i1 %15, false
  br i1 %__implicit_unwrap.condition, label %__implicit_unwrap.fail, label %__implicit_unwrap.success

__implicit_unwrap.fail:                           ; preds = %if.end11
  call void @_EN3std10assertFailEP4char(ptr @0)
  unreachable

__implicit_unwrap.success:                        ; preds = %if.end11
  %16 = extractvalue %"Optional<int>" %c.load12, 1
  call void @_EN4main12takesNonNullE3int(i32 %16)
  %c.load13 = load %"Optional<int>", ptr %c, align 4
  %17 = extractvalue %"Optional<int>" %c.load13, 0
  br i1 %17, label %if.then14, label %if.else16

if.then14:                                        ; preds = %__implicit_unwrap.success
  %18 = getelementptr inbounds %"Optional<int>", ptr %c, i32 0, i32 1
  %.load15 = load i32, ptr %18, align 4
  call void @_EN4main12takesNonNullE3int(i32 %.load15)
  br label %if.end17

if.else16:                                        ; preds = %__implicit_unwrap.success
  br label %if.end17

if.end17:                                         ; preds = %if.else16, %if.then14
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
