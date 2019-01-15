
@0 = private unnamed_addr constant [37 x i8] c"Assertion failed at assert.delta:6:5\00"

declare i1 @b()

define i32 @main() {
  %1 = call i1 @b()
  %assert.condition = icmp eq i1 %1, false
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %0
  %2 = call i32 @puts(i8* getelementptr inbounds ([37 x i8], [37 x i8]* @0, i32 0, i32 0))
  call void @abort()
  unreachable

assert.success:                                   ; preds = %0
  ret i32 0
}

declare i32 @puts(i8*)

declare void @abort()
