
define i32 @main() {
  %a = alloca [3 x i8*], align 8
  %b = alloca [2 x i32*], align 8
  %c = alloca [1 x float**], align 8
  ret i32 0
}
