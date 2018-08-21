
@0 = private unnamed_addr constant [71 x i8] c"Assertion failed at /Users/emlai/src/delta/test/irgen/assert.delta:6:5\00"

declare i1 @b()

define i32 @main() {
  %1 = call i1 @b()
  %assert.condition = icmp eq i1 %1, false
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %0
  %2 = call i32 @puts(i8* getelementptr inbounds ([71 x i8], [71 x i8]* @0, i32 0, i32 0))
  call void @llvm.trap()
  unreachable

assert.success:                                   ; preds = %0
  ret i32 0
}

declare i32 @puts(i8*)

; Function Attrs: noreturn nounwind
declare void @llvm.trap() #0

attributes #0 = { noreturn nounwind }
