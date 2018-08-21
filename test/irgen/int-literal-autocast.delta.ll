
declare void @foo(i8)

define i32 @main() {
  %b = alloca i64
  %c = alloca i8
  call void @foo(i8 1)
  store i64 42, i64* %b
  store i8 -42, i8* %c
  %b1 = load i64, i64* %b
  %1 = add i64 %b1, 1
  store i64 %1, i64* %b
  ret i32 0
}
