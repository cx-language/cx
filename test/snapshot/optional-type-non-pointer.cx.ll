
%"Optional<int>" = type { i1, i32 }

@0 = private unnamed_addr constant [53 x i8] c"Unwrap failed at optional-type-non-pointer.cx:24:18\0A\00", align 1
@1 = private unnamed_addr constant [53 x i8] c"Unwrap failed at optional-type-non-pointer.cx:25:18\0A\00", align 1
@2 = private unnamed_addr constant [53 x i8] c"Unwrap failed at optional-type-non-pointer.cx:26:16\0A\00", align 1
@3 = private unnamed_addr constant [53 x i8] c"Unwrap failed at optional-type-non-pointer.cx:30:18\0A\00", align 1
@4 = private unnamed_addr constant [53 x i8] c"Unwrap failed at optional-type-non-pointer.cx:32:22\0A\00", align 1

define i32 @main() {
  %a = alloca %"Optional<int>", align 8
  %1 = alloca %"Optional<int>", align 8
  %b = alloca %"Optional<int>", align 8
  %c = alloca %"Optional<int>", align 8
  %2 = alloca %"Optional<int>", align 8
  %d = alloca %"Optional<int>", align 8
  %3 = alloca %"Optional<int>", align 8
  %4 = alloca %"Optional<int>", align 8
  call void @_EN3std8OptionalI3intE4initE(%"Optional<int>"* %1)
  %.load = load %"Optional<int>", %"Optional<int>"* %1, align 4
  store %"Optional<int>" %.load, %"Optional<int>"* %a, align 4
  call void @_EN3std8OptionalI3intE4initE3int(%"Optional<int>"* %b, i32 2)
  call void @_EN3std8OptionalI3intE4initE3int(%"Optional<int>"* %2, i32 3)
  %.load1 = load %"Optional<int>", %"Optional<int>"* %2, align 4
  store %"Optional<int>" %.load1, %"Optional<int>"* %c, align 4
  call void @_EN3std8OptionalI3intE4initE(%"Optional<int>"* %d)
  call void @_EN3std8OptionalI3intE4initE(%"Optional<int>"* %3)
  %5 = call i1 @_EN3stdeqI3intEEPO3intPO3int(%"Optional<int>"* %d, %"Optional<int>"* %3)
  br i1 %5, label %if.then, label %if.else

if.then:                                          ; preds = %0
  br label %if.end

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  call void @_EN3std8OptionalI3intE4initE(%"Optional<int>"* %4)
  %6 = call i1 @_EN3stdneI3intEEPO3intPO3int(%"Optional<int>"* %d, %"Optional<int>"* %4)
  br i1 %6, label %if.then2, label %if.else3

if.then2:                                         ; preds = %if.end
  br label %if.end4

if.else3:                                         ; preds = %if.end
  br label %if.end4

if.end4:                                          ; preds = %if.else3, %if.then2
  %d.load = load %"Optional<int>", %"Optional<int>"* %d, align 4
  %7 = extractvalue %"Optional<int>" %d.load, 0
  br i1 %7, label %if.then5, label %if.else6

if.then5:                                         ; preds = %if.end4
  br label %if.end7

if.else6:                                         ; preds = %if.end4
  br label %if.end7

if.end7:                                          ; preds = %if.else6, %if.then5
  %d.load8 = load %"Optional<int>", %"Optional<int>"* %d, align 4
  %8 = extractvalue %"Optional<int>" %d.load8, 0
  %9 = xor i1 %8, true
  br i1 %9, label %if.then9, label %if.else10

if.then9:                                         ; preds = %if.end7
  br label %if.end11

if.else10:                                        ; preds = %if.end7
  br label %if.end11

if.end11:                                         ; preds = %if.else10, %if.then9
  ret i32 0
}

define void @_EN3std8OptionalI3intE4initE(%"Optional<int>"* %this) {
  %hasValue = getelementptr inbounds %"Optional<int>", %"Optional<int>"* %this, i32 0, i32 0
  store i1 false, i1* %hasValue, align 1
  ret void
}

define void @_EN3std8OptionalI3intE4initE3int(%"Optional<int>"* %this, i32 %value) {
  %hasValue = getelementptr inbounds %"Optional<int>", %"Optional<int>"* %this, i32 0, i32 0
  store i1 true, i1* %hasValue, align 1
  %value1 = getelementptr inbounds %"Optional<int>", %"Optional<int>"* %this, i32 0, i32 1
  store i32 %value, i32* %value1, align 4
  ret void
}

define i1 @_EN3stdeqI3intEEPO3intPO3int(%"Optional<int>"* %a, %"Optional<int>"* %b) {
  %hasValue = getelementptr inbounds %"Optional<int>", %"Optional<int>"* %a, i32 0, i32 0
  %hasValue.load = load i1, i1* %hasValue, align 1
  %hasValue1 = getelementptr inbounds %"Optional<int>", %"Optional<int>"* %b, i32 0, i32 0
  %hasValue.load2 = load i1, i1* %hasValue1, align 1
  %1 = icmp ne i1 %hasValue.load, %hasValue.load2
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  ret i1 false

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else
  %hasValue3 = getelementptr inbounds %"Optional<int>", %"Optional<int>"* %b, i32 0, i32 0
  %hasValue.load4 = load i1, i1* %hasValue3, align 1
  %2 = xor i1 %hasValue.load4, true
  br i1 %2, label %if.then5, label %if.else6

if.then5:                                         ; preds = %if.end
  ret i1 true

if.else6:                                         ; preds = %if.end
  br label %if.end7

if.end7:                                          ; preds = %if.else6
  %value = getelementptr inbounds %"Optional<int>", %"Optional<int>"* %a, i32 0, i32 1
  %value.load = load i32, i32* %value, align 4
  %value8 = getelementptr inbounds %"Optional<int>", %"Optional<int>"* %b, i32 0, i32 1
  %value.load9 = load i32, i32* %value8, align 4
  %3 = icmp eq i32 %value.load, %value.load9
  ret i1 %3
}

define i1 @_EN3stdneI3intEEPO3intPO3int(%"Optional<int>"* %a, %"Optional<int>"* %b) {
  %1 = call i1 @_EN3stdeqI3intEEPO3intPO3int(%"Optional<int>"* %a, %"Optional<int>"* %b)
  %2 = xor i1 %1, true
  ret i1 %2
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
  call void @_EN3std8OptionalI3intE4initE(%"Optional<int>"* %1)
  %.load = load %"Optional<int>", %"Optional<int>"* %1, align 4
  store %"Optional<int>" %.load, %"Optional<int>"* %a, align 4
  call void @_EN3std8OptionalI3intE4initE3int(%"Optional<int>"* %b, i32 2)
  call void @_EN3std8OptionalI3intE4initE3int(%"Optional<int>"* %2, i32 3)
  %.load1 = load %"Optional<int>", %"Optional<int>"* %2, align 4
  store %"Optional<int>" %.load1, %"Optional<int>"* %c, align 4
  call void @_EN3std8OptionalI3intE4initE(%"Optional<int>"* %d)
  call void @_EN3std8OptionalI3intE4initE(%"Optional<int>"* %3)
  %.load2 = load %"Optional<int>", %"Optional<int>"* %3, align 4
  store %"Optional<int>" %.load2, %"Optional<int>"* %c, align 4
  %c.load = load %"Optional<int>", %"Optional<int>"* %c, align 4
  %5 = extractvalue %"Optional<int>" %c.load, 0
  br i1 %5, label %if.then, label %if.else

if.then:                                          ; preds = %0
  %c.load3 = load %"Optional<int>", %"Optional<int>"* %c, align 4
  %6 = extractvalue %"Optional<int>" %c.load3, 0
  %assert.condition = icmp eq i1 %6, false
  br i1 %assert.condition, label %assert.fail, label %assert.success

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %assert.success12, %if.else
  %c.load4 = load %"Optional<int>", %"Optional<int>"* %c, align 4
  %7 = extractvalue %"Optional<int>" %c.load4, 0
  %__implicit_unwrap.condition = icmp eq i1 %7, false
  br i1 %__implicit_unwrap.condition, label %__implicit_unwrap.fail, label %__implicit_unwrap.success

assert.fail:                                      ; preds = %if.then
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([53 x i8], [53 x i8]* @0, i32 0, i32 0))
  unreachable

assert.success:                                   ; preds = %if.then
  %8 = extractvalue %"Optional<int>" %c.load3, 1
  %9 = add i32 %8, 1
  store i32 %9, i32* %x, align 4
  %c.load5 = load %"Optional<int>", %"Optional<int>"* %c, align 4
  %10 = extractvalue %"Optional<int>" %c.load5, 0
  %assert.condition6 = icmp eq i1 %10, false
  br i1 %assert.condition6, label %assert.fail7, label %assert.success8

assert.fail7:                                     ; preds = %assert.success
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([53 x i8], [53 x i8]* @1, i32 0, i32 0))
  unreachable

assert.success8:                                  ; preds = %assert.success
  %11 = extractvalue %"Optional<int>" %c.load5, 1
  %12 = add i32 1, %11
  %c.load9 = load %"Optional<int>", %"Optional<int>"* %c, align 4
  %13 = extractvalue %"Optional<int>" %c.load9, 0
  %assert.condition10 = icmp eq i1 %13, false
  br i1 %assert.condition10, label %assert.fail11, label %assert.success12

assert.fail11:                                    ; preds = %assert.success8
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([53 x i8], [53 x i8]* @2, i32 0, i32 0))
  unreachable

assert.success12:                                 ; preds = %assert.success8
  %14 = extractvalue %"Optional<int>" %c.load9, 1
  %15 = sub i32 0, %14
  call void @_EN3std8OptionalI3intE4initE3int(%"Optional<int>"* %4, i32 %15)
  %.load13 = load %"Optional<int>", %"Optional<int>"* %4, align 4
  store %"Optional<int>" %.load13, %"Optional<int>"* %c, align 4
  br label %if.end

__implicit_unwrap.fail:                           ; preds = %if.end
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([53 x i8], [53 x i8]* @3, i32 0, i32 0))
  unreachable

__implicit_unwrap.success:                        ; preds = %if.end
  %16 = extractvalue %"Optional<int>" %c.load4, 1
  call void @_EN4main12takesNonNullE3int(i32 %16)
  %c.load14 = load %"Optional<int>", %"Optional<int>"* %c, align 4
  %17 = extractvalue %"Optional<int>" %c.load14, 0
  br i1 %17, label %if.then15, label %if.else18

if.then15:                                        ; preds = %__implicit_unwrap.success
  %c.load16 = load %"Optional<int>", %"Optional<int>"* %c, align 4
  %18 = extractvalue %"Optional<int>" %c.load16, 0
  %__implicit_unwrap.condition17 = icmp eq i1 %18, false
  br i1 %__implicit_unwrap.condition17, label %__implicit_unwrap.fail20, label %__implicit_unwrap.success21

if.else18:                                        ; preds = %__implicit_unwrap.success
  br label %if.end19

if.end19:                                         ; preds = %__implicit_unwrap.success21, %if.else18
  ret void

__implicit_unwrap.fail20:                         ; preds = %if.then15
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([53 x i8], [53 x i8]* @4, i32 0, i32 0))
  unreachable

__implicit_unwrap.success21:                      ; preds = %if.then15
  %19 = extractvalue %"Optional<int>" %c.load16, 1
  call void @_EN4main12takesNonNullE3int(i32 %19)
  br label %if.end19
}

declare void @_EN3std10assertFailEP4char(i8*)

define void @_EN4main12takesNonNullE3int(i32 %i) {
  ret void
}

define void @_EN4main1gE() {
  %p = alloca i8**, align 8
  store i8** null, i8*** %p, align 8
  %p.load = load i8**, i8*** %p, align 8
  %1 = icmp eq i8** %p.load, null
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  br label %if.end

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  ret void
}
