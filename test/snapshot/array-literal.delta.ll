
define i32 @main() {
  %a = alloca [3 x i32], align 4
  store [3 x i32] [i32 1, i32 2, i32 3], [3 x i32]* %a, align 4
  %1 = getelementptr inbounds [3 x i32], [3 x i32]* %a, i32 0, i32 1
  %.load = load i32, i32* %1, align 4
  ret i32 %.load
}
