
define i32 @main() {
  %a = alloca float
  %b = alloca float
  store float 1.000000e+00, float* %a
  %a.load = load float, float* %a
  %1 = fsub float -0.000000e+00, %a.load
  %2 = fmul float 2.000000e+00, %1
  store float %2, float* %b
  ret i32 0
}
