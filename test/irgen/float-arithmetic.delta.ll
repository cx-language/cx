
define i32 @main() {
  %a = alloca double
  %b = alloca double
  store double 1.000000e+00, double* %a
  %a1 = load double, double* %a
  %1 = fsub double -0.000000e+00, %a1
  %2 = fmul double 2.000000e+00, %1
  store double %2, double* %b
  ret i32 0
}
