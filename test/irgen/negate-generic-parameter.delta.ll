
define i32 @main() {
  %1 = call double @_EN4main1fI7float64EE1a7float64(double 0.000000e+00)
  ret i32 0
}

define double @_EN4main1fI7float64EE1a7float64(double %a) {
  %1 = fsub double -0.000000e+00, %a
  ret double %1
}
