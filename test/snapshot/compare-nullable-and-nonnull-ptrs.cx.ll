
@0 = private unnamed_addr constant [60 x i8] c"Unwrap failed at compare-nullable-and-nonnull-ptrs.cx:4:16\0A\00", align 1

define void @_EN4main1fEP3intOP3int(i32* %foo, i32* %bar) {
  %__implicit_unwrap.condition = icmp eq i32* %bar, null
  br i1 %__implicit_unwrap.condition, label %__implicit_unwrap.fail, label %__implicit_unwrap.success

__implicit_unwrap.fail:                           ; preds = %0
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([60 x i8], [60 x i8]* @0, i32 0, i32 0))
  unreachable

__implicit_unwrap.success:                        ; preds = %0
  %1 = icmp eq i32* %foo, %bar
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %__implicit_unwrap.success
  br label %if.end

if.else:                                          ; preds = %__implicit_unwrap.success
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  ret void
}

declare void @_EN3std10assertFailEP4char(i8*)
