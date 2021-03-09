
declare void @foo(i8)

define i32 @main() {
  %b = alloca i64, align 8
  %c = alloca i8, align 1
  call void @foo(i8 1)
  store i64 42, i64* %b, align 4
  store i8 -42, i8* %c, align 1
  %b.load = load i64, i64* %b, align 4
  %1 = add i64 %b.load, 1
  store i64 %1, i64* %b, align 4
  ret i32 0
}
