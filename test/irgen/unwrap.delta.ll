
@0 = private unnamed_addr constant [38 x i8] c"Assertion failed at unwrap.delta:6:58\00"
@1 = private unnamed_addr constant [37 x i8] c"Assertion failed at unwrap.delta:8:8\00"

declare i8* @f()

define i32 @main() {
  %byte = alloca i8*
  %ptr = alloca [1 x i8]*
  %1 = call i8* @f()
  %assert.condition = icmp eq i8* %1, null
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %0
  %2 = call i32 @puts(i8* getelementptr inbounds ([38 x i8], [38 x i8]* @0, i32 0, i32 0))
  call void @llvm.trap()
  unreachable

assert.success:                                   ; preds = %0
  store i8* %1, i8** %byte
  %3 = call i8* @f()
  %4 = bitcast i8* %3 to [1 x i8]*
  store [1 x i8]* %4, [1 x i8]** %ptr
  %ptr1 = load [1 x i8]*, [1 x i8]** %ptr
  %assert.condition2 = icmp eq [1 x i8]* %ptr1, null
  br i1 %assert.condition2, label %assert.fail3, label %assert.success4

assert.fail3:                                     ; preds = %assert.success
  %5 = call i32 @puts(i8* getelementptr inbounds ([37 x i8], [37 x i8]* @1, i32 0, i32 0))
  call void @llvm.trap()
  unreachable

assert.success4:                                  ; preds = %assert.success
  %6 = getelementptr [1 x i8], [1 x i8]* %ptr1, i32 0, i32 0
  store i8 1, i8* %6
  ret i32 0
}

declare i32 @puts(i8*)

; Function Attrs: noreturn nounwind
declare void @llvm.trap() #0

attributes #0 = { noreturn nounwind }
