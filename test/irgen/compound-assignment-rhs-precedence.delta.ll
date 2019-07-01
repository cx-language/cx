
define void @_EN4main1fE3fooP7float643bar7float643baz7float64(double* %foo, double %bar, double %baz) {
  %foo.load = load double, double* %foo
  %1 = fmul double %bar, %baz
  %2 = fsub double 1.000000e+00, %1
  %3 = fmul double %foo.load, %2
  store double %3, double* %foo
  ret void
}
