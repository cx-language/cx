declare void @foo(i8)

define i32 @main() {
  %b = alloca i64
  call void @foo(i8 1)
  store i64 42, i64* %b
  %1 = load i64, i64* %b
  %2 = sub i64 %1, 1
  store i64 %2, i64* %b
  %b1 = load i64, i64* %b
  %3 = add i64 %b1, 1
  store i64 %3, i64* %b
  ret i32 0
}
