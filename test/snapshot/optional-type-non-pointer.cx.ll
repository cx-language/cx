
%"Optional<int>" = type { i1, i32 }

@0 = private unnamed_addr constant [53 x i8] c"Unwrap failed at optional-type-non-pointer.cx:24:18\0A\00", align 1
@1 = private unnamed_addr constant [53 x i8] c"Unwrap failed at optional-type-non-pointer.cx:25:18\0A\00", align 1
@2 = private unnamed_addr constant [53 x i8] c"Unwrap failed at optional-type-non-pointer.cx:26:16\0A\00", align 1

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

if.end:                                           ; preds = %assert.success11, %if.else
  ret void

assert.fail:                                      ; preds = %if.then
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([53 x i8], [53 x i8]* @0, i32 0, i32 0))
  unreachable

assert.success:                                   ; preds = %if.then
  %7 = extractvalue %"Optional<int>" %c.load3, 1
  %8 = add i32 %7, 1
  store i32 %8, i32* %x, align 4
  %c.load4 = load %"Optional<int>", %"Optional<int>"* %c, align 4
  %9 = extractvalue %"Optional<int>" %c.load4, 0
  %assert.condition5 = icmp eq i1 %9, false
  br i1 %assert.condition5, label %assert.fail6, label %assert.success7

assert.fail6:                                     ; preds = %assert.success
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([53 x i8], [53 x i8]* @1, i32 0, i32 0))
  unreachable

assert.success7:                                  ; preds = %assert.success
  %10 = extractvalue %"Optional<int>" %c.load4, 1
  %11 = add i32 1, %10
  %c.load8 = load %"Optional<int>", %"Optional<int>"* %c, align 4
  %12 = extractvalue %"Optional<int>" %c.load8, 0
  %assert.condition9 = icmp eq i1 %12, false
  br i1 %assert.condition9, label %assert.fail10, label %assert.success11

assert.fail10:                                    ; preds = %assert.success7
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([53 x i8], [53 x i8]* @2, i32 0, i32 0))
  unreachable

assert.success11:                                 ; preds = %assert.success7
  %13 = extractvalue %"Optional<int>" %c.load8, 1
  %14 = sub i32 0, %13
  call void @_EN3std8OptionalI3intE4initE3int(%"Optional<int>"* %4, i32 %14)
  %.load12 = load %"Optional<int>", %"Optional<int>"* %4, align 4
  store %"Optional<int>" %.load12, %"Optional<int>"* %c, align 4
  br label %if.end
}

declare void @_EN3std10assertFailEP4char(i8*)

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
