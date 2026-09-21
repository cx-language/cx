
@foo = private global [3 x i32] [i32 0, i32 1, i32 2]

define i32 @main() {
  %qux = alloca [1 x i32], align 4
  store [1 x i32] [i32 42], ptr %qux, align 4
  store i32 3, ptr @foo, align 4
  %1 = call i64 @_EN3std3int4hashE(ptr getelementptr inbounds ([3 x i32], ptr @foo, i32 0, i32 1))
  ret i32 0
}

define i64 @_EN3std3int4hashE(ptr %this) {
  %this.load = load i32, ptr %this, align 4
  %1 = sext i32 %this.load to i64
  ret i64 %1
}
