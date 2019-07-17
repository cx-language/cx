
define i32 @main() {
  %foo = alloca i64
  %1 = alloca i32
  store i32 0, i32* %1
  %2 = call i64 @_EN3std3int4hashE(i32* %1)
  store i64 %2, i64* %foo
  ret i32 0
}

define i64 @_EN3std3int4hashE(i32* %this) {
  %this.load = load i32, i32* %this
  %1 = sext i32 %this.load to i64
  ret i64 %1
}
