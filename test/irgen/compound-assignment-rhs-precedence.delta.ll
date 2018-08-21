
define void @_EN4main1fE3fooPM7float643bar7float643baz7float64(double* %foo, double %bar, double %baz) {
  %1 = load double, double* %foo
  %2 = fmul double %bar, %baz
  %3 = fsub double 1.000000e+00, %2
  %4 = fmul double %1, %3
  store double %4, double* %foo
  ret void
}
