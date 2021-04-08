
define i32 @main() {
  %foo = alloca [1 x float], align 4
  %bar = alloca [2 x i16], align 2
  store [1 x float] [float 1.000000e+00], [1 x float]* %foo, align 4
  store [2 x i16] [i16 50, i16 100], [2 x i16]* %bar, align 2
  ret i32 0
}
