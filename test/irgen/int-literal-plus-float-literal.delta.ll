
define i32 @main() {
  %a = alloca double
  %b = alloca double
  store double 8.000000e+00, double* %a
  store double 8.000000e+00, double* %b
  ret i32 0
}
