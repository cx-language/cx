
@0 = private unnamed_addr constant [61 x i8] c"Unwrap failed at var-decl-in-control-flow-condition.cx:5:11\0A\00", align 1
@1 = private unnamed_addr constant [61 x i8] c"Unwrap failed at var-decl-in-control-flow-condition.cx:9:11\0A\00", align 1

define void @_EN4main1fE() {
  %c = alloca i32*, align 8
  %c1 = alloca i32*, align 8
  br label %loop.condition

loop.condition:                                   ; preds = %__implicit_unwrap.success, %0
  %1 = call i32* @_EN4main1hE()
  store i32* %1, i32** %c, align 8
  %c.load = load i32*, i32** %c, align 8
  %2 = icmp ne i32* %c.load, null
  br i1 %2, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %c.load2 = load i32*, i32** %c, align 8
  %__implicit_unwrap.condition = icmp eq i32* %c.load2, null
  br i1 %__implicit_unwrap.condition, label %__implicit_unwrap.fail, label %__implicit_unwrap.success

loop.end:                                         ; preds = %loop.condition
  %3 = call i32* @_EN4main1hE()
  store i32* %3, i32** %c1, align 8
  %c.load3 = load i32*, i32** %c1, align 8
  %4 = icmp ne i32* %c.load3, null
  br i1 %4, label %if.then, label %if.else

__implicit_unwrap.fail:                           ; preds = %loop.body
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([61 x i8], [61 x i8]* @0, i32 0, i32 0))
  unreachable

__implicit_unwrap.success:                        ; preds = %loop.body
  call void @_EN4main1jEP3int(i32* %c.load2)
  br label %loop.condition

if.then:                                          ; preds = %loop.end
  %c.load4 = load i32*, i32** %c1, align 8
  %__implicit_unwrap.condition5 = icmp eq i32* %c.load4, null
  br i1 %__implicit_unwrap.condition5, label %__implicit_unwrap.fail6, label %__implicit_unwrap.success7

if.else:                                          ; preds = %loop.end
  br label %if.end

if.end:                                           ; preds = %__implicit_unwrap.success7, %if.else
  ret void

__implicit_unwrap.fail6:                          ; preds = %if.then
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([61 x i8], [61 x i8]* @1, i32 0, i32 0))
  unreachable

__implicit_unwrap.success7:                       ; preds = %if.then
  call void @_EN4main1jEP3int(i32* %c.load4)
  br label %if.end
}

define i32* @_EN4main1hE() {
  ret i32* null
}

declare void @_EN3std10assertFailEP4char(i8*)

define void @_EN4main1jEP3int(i32* %i) {
  ret void
}
