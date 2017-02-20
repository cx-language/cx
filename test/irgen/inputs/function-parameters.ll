define i32 @foo(i32 %a, i1 %b) {
  %aa = alloca i32
  %bb = alloca i1
  store i32 %a, i32* %aa
  store i1 %b, i1* %bb
  ret i32 0
}

define i32 @main() {
  %value = alloca i32
  %result = alloca i32
  store i32 42, i32* %value
  %value1 = load i32, i32* %value
  %1 = call i32 @foo(i32 %value1, i1 true)
  store i32 %1, i32* %result
  ret i32 0
}
