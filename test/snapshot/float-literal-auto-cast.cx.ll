
define i32 @main() {
  %f = alloca float, align 4
  %g = alloca float, align 4
  store float 4.200000e+01, float* %f, align 4
  store float -4.200000e+01, float* %g, align 4
  ret i32 0
}
