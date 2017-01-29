define void @foo(i32 %a, i1 %b, i8 %c) {
  %aa = alloca i32
  store i32 %a, i32* %aa
  %bb = alloca i1
  store i1 %b, i1* %bb
  %cc = alloca i8
  store i8 %c, i8* %cc
  ret void
}
