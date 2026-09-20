
define void @_EN4main1fEP7float647float647float64(ptr %foo, double %bar, double %baz) {
  %foo1 = alloca ptr, align 8
  %bar2 = alloca double, align 8
  %baz3 = alloca double, align 8
  store ptr %foo, ptr %foo1, align 8
  store double %bar, ptr %bar2, align 8
  store double %baz, ptr %baz3, align 8
  %foo.load = load ptr, ptr %foo1, align 8
  %foo.load4 = load ptr, ptr %foo1, align 8
  %foo.load.load = load double, ptr %foo.load4, align 8
  %bar.load = load double, ptr %bar2, align 8
  %baz.load = load double, ptr %baz3, align 8
  %1 = fmul double %bar.load, %baz.load
  %2 = fsub double 1.000000e+00, %1
  %3 = fmul double %foo.load.load, %2
  store double %3, ptr %foo.load, align 8
  ret void
}
