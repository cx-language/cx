
define i32 @main() {
  %foo = alloca i64, align 8
  %1 = alloca i32, align 4
  store i32 0, i32* %1, align 4
  %2 = call i64 @_EN3std3int4hashE(i32* %1)
  store i64 %2, i64* %foo, align 4
  ret i32 0
}

define i64 @_EN3std3int4hashE(i32* %this) {
  %this.load = load i32, i32* %this, align 4
  %1 = sext i32 %this.load to i64
  ret i64 %1
}
