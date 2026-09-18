
@0 = private unnamed_addr constant [61 x i8] c"Unwrap failed at var-decl-in-control-flow-condition.cx:5:11\0A\00", align 1
@1 = private unnamed_addr constant [61 x i8] c"Unwrap failed at var-decl-in-control-flow-condition.cx:9:11\0A\00", align 1
@2 = private unnamed_addr constant [62 x i8] c"Unwrap failed at var-decl-in-control-flow-condition.cx:15:11\0A\00", align 1
@3 = private unnamed_addr constant [62 x i8] c"Unwrap failed at var-decl-in-control-flow-condition.cx:19:11\0A\00", align 1
@4 = private unnamed_addr constant [62 x i8] c"Unwrap failed at var-decl-in-control-flow-condition.cx:23:11\0A\00", align 1
@5 = private unnamed_addr constant [62 x i8] c"Unwrap failed at var-decl-in-control-flow-condition.cx:28:11\0A\00", align 1
@6 = private unnamed_addr constant [62 x i8] c"Unwrap failed at var-decl-in-control-flow-condition.cx:32:11\0A\00", align 1

define void @_EN4main1fE() {
  %c = alloca ptr, align 8
  %c1 = alloca ptr, align 8
  br label %loop.condition

loop.condition:                                   ; preds = %__implicit_unwrap.success, %0
  %1 = call ptr @_EN4main1hE()
  store ptr %1, ptr %c, align 8
  %c.load = load ptr, ptr %c, align 8
  %2 = icmp ne ptr %c.load, null
  br i1 %2, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %c.load2 = load ptr, ptr %c, align 8
  %__implicit_unwrap.condition = icmp eq ptr %c.load2, null
  br i1 %__implicit_unwrap.condition, label %__implicit_unwrap.fail, label %__implicit_unwrap.success

loop.end:                                         ; preds = %loop.condition
  %3 = call ptr @_EN4main1hE()
  store ptr %3, ptr %c1, align 8
  %c.load3 = load ptr, ptr %c1, align 8
  %4 = icmp ne ptr %c.load3, null
  br i1 %4, label %if.then, label %if.else

__implicit_unwrap.fail:                           ; preds = %loop.body
  call void @_EN3std10assertFailEP4char(ptr @0)
  unreachable

__implicit_unwrap.success:                        ; preds = %loop.body
  call void @_EN4main1jEP3int(ptr %c.load2)
  br label %loop.condition

if.then:                                          ; preds = %loop.end
  %c.load4 = load ptr, ptr %c1, align 8
  %__implicit_unwrap.condition5 = icmp eq ptr %c.load4, null
  br i1 %__implicit_unwrap.condition5, label %__implicit_unwrap.fail6, label %__implicit_unwrap.success7

if.else:                                          ; preds = %loop.end
  br label %if.end

if.end:                                           ; preds = %__implicit_unwrap.success7, %if.else
  ret void

__implicit_unwrap.fail6:                          ; preds = %if.then
  call void @_EN3std10assertFailEP4char(ptr @1)
  unreachable

__implicit_unwrap.success7:                       ; preds = %if.then
  call void @_EN4main1jEP3int(ptr %c.load4)
  br label %if.end
}

define ptr @_EN4main1hE() {
  ret ptr null
}

declare void @_EN3std10assertFailEP4char(ptr)

define void @_EN4main1jEP3int(ptr %i) {
  ret void
}

define void @_EN4main1gE() {
  %c1 = alloca ptr, align 8
  %c2 = alloca ptr, align 8
  %c3 = alloca ptr, align 8
  %c4 = alloca ptr, align 8
  %c5 = alloca ptr, align 8
  br label %loop.condition

loop.condition:                                   ; preds = %__implicit_unwrap.success, %0
  %1 = call ptr @_EN4main1hE()
  store ptr %1, ptr %c1, align 8
  %c1.load = load ptr, ptr %c1, align 8
  %2 = icmp ne ptr %c1.load, null
  br i1 %2, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %c1.load1 = load ptr, ptr %c1, align 8
  %__implicit_unwrap.condition = icmp eq ptr %c1.load1, null
  br i1 %__implicit_unwrap.condition, label %__implicit_unwrap.fail, label %__implicit_unwrap.success

loop.end:                                         ; preds = %loop.condition
  %3 = call ptr @_EN4main1hE()
  store ptr %3, ptr %c2, align 8
  %c2.load = load ptr, ptr %c2, align 8
  %4 = icmp ne ptr %c2.load, null
  br i1 %4, label %if.then, label %if.else

__implicit_unwrap.fail:                           ; preds = %loop.body
  call void @_EN3std10assertFailEP4char(ptr @2)
  unreachable

__implicit_unwrap.success:                        ; preds = %loop.body
  call void @_EN4main1jEP3int(ptr %c1.load1)
  br label %loop.condition

if.then:                                          ; preds = %loop.end
  %c2.load2 = load ptr, ptr %c2, align 8
  %__implicit_unwrap.condition3 = icmp eq ptr %c2.load2, null
  br i1 %__implicit_unwrap.condition3, label %__implicit_unwrap.fail4, label %__implicit_unwrap.success5

if.else:                                          ; preds = %loop.end
  br label %if.end

if.end:                                           ; preds = %__implicit_unwrap.success5, %if.else
  br label %loop.condition6

__implicit_unwrap.fail4:                          ; preds = %if.then
  call void @_EN3std10assertFailEP4char(ptr @3)
  unreachable

__implicit_unwrap.success5:                       ; preds = %if.then
  call void @_EN4main1jEP3int(ptr %c2.load2)
  br label %if.end

loop.condition6:                                  ; preds = %__implicit_unwrap.success12, %if.end
  %5 = call ptr @_EN4main1hE()
  store ptr %5, ptr %c3, align 8
  %c3.load = load ptr, ptr %c3, align 8
  %6 = icmp ne ptr %c3.load, null
  br i1 %6, label %loop.body7, label %loop.end10

loop.body7:                                       ; preds = %loop.condition6
  %c3.load8 = load ptr, ptr %c3, align 8
  %__implicit_unwrap.condition9 = icmp eq ptr %c3.load8, null
  br i1 %__implicit_unwrap.condition9, label %__implicit_unwrap.fail11, label %__implicit_unwrap.success12

loop.end10:                                       ; preds = %loop.condition6
  %7 = call ptr @_EN4main1hE()
  store ptr %7, ptr %c4, align 8
  %c4.load = load ptr, ptr %c4, align 8
  %8 = icmp ne ptr %c4.load, null
  br i1 %8, label %if.then13, label %if.else16

__implicit_unwrap.fail11:                         ; preds = %loop.body7
  call void @_EN3std10assertFailEP4char(ptr @4)
  unreachable

__implicit_unwrap.success12:                      ; preds = %loop.body7
  call void @_EN4main1jEP3int(ptr %c3.load8)
  br label %loop.condition6

if.then13:                                        ; preds = %loop.end10
  %c4.load14 = load ptr, ptr %c4, align 8
  %__implicit_unwrap.condition15 = icmp eq ptr %c4.load14, null
  br i1 %__implicit_unwrap.condition15, label %__implicit_unwrap.fail18, label %__implicit_unwrap.success19

if.else16:                                        ; preds = %loop.end10
  br label %if.end17

if.end17:                                         ; preds = %__implicit_unwrap.success19, %if.else16
  %9 = call ptr @_EN4main1hE()
  store ptr %9, ptr %c5, align 8
  %c5.load = load ptr, ptr %c5, align 8
  %10 = icmp ne ptr %c5.load, null
  br i1 %10, label %if.then20, label %if.else23

__implicit_unwrap.fail18:                         ; preds = %if.then13
  call void @_EN3std10assertFailEP4char(ptr @5)
  unreachable

__implicit_unwrap.success19:                      ; preds = %if.then13
  call void @_EN4main1jEP3int(ptr %c4.load14)
  br label %if.end17

if.then20:                                        ; preds = %if.end17
  %c5.load21 = load ptr, ptr %c5, align 8
  %__implicit_unwrap.condition22 = icmp eq ptr %c5.load21, null
  br i1 %__implicit_unwrap.condition22, label %__implicit_unwrap.fail25, label %__implicit_unwrap.success26

if.else23:                                        ; preds = %if.end17
  br label %if.end24

if.end24:                                         ; preds = %__implicit_unwrap.success26, %if.else23
  ret void

__implicit_unwrap.fail25:                         ; preds = %if.then20
  call void @_EN3std10assertFailEP4char(ptr @6)
  unreachable

__implicit_unwrap.success26:                      ; preds = %if.then20
  call void @_EN4main1jEP3int(ptr %c5.load21)
  br label %if.end24
}
