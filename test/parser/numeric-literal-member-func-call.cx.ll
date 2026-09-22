
define i32 @main() {
  %foo = alloca i64, align 8
  %1 = alloca i32, align 4
  store i32 0, ptr %1, align 4
  %2 = call i64 @_EN3std3int4hashE(ptr %1)
  store i64 %2, ptr %foo, align 8
  ret i32 0
}

define i64 @_EN3std3int4hashE(ptr %this) {
  %this.load = load i32, ptr %this, align 4
  %1 = sext i32 %this.load to i64
  ret i64 %1
}
