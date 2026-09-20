
%S2 = type { double, double, double, double }
%S = type { [4 x double] }

define void @_EN4main19returnsLargeStruct2E(ptr sret(%S2) %sret.arg) {
  %1 = alloca %S2, align 8
  call void @_EN4main2S24initE7float647float647float647float64(ptr %1, double 0.000000e+00, double 1.000000e+00, double 2.000000e+00, double 3.000000e+00)
  %.load = load %S2, ptr %1, align 8
  store %S2 %.load, ptr %sret.arg, align 8
  ret void
}

define void @_EN4main2S24initE7float647float647float647float64(ptr %this, double %x, double %y, double %z, double %w) {
  %x1 = alloca double, align 8
  %y2 = alloca double, align 8
  %z3 = alloca double, align 8
  %w4 = alloca double, align 8
  store double %x, ptr %x1, align 8
  store double %y, ptr %y2, align 8
  store double %z, ptr %z3, align 8
  store double %w, ptr %w4, align 8
  %x5 = getelementptr inbounds %S2, ptr %this, i32 0, i32 0
  %x.load = load double, ptr %x1, align 8
  store double %x.load, ptr %x5, align 8
  %y6 = getelementptr inbounds %S2, ptr %this, i32 0, i32 1
  %y.load = load double, ptr %y2, align 8
  store double %y.load, ptr %y6, align 8
  %z7 = getelementptr inbounds %S2, ptr %this, i32 0, i32 2
  %z.load = load double, ptr %z3, align 8
  store double %z.load, ptr %z7, align 8
  %w8 = getelementptr inbounds %S2, ptr %this, i32 0, i32 3
  %w.load = load double, ptr %w4, align 8
  store double %w.load, ptr %w8, align 8
  ret void
}

define i32 @main() {
  %s = alloca %S, align 8
  %s2 = alloca %S2, align 8
  %sret.alloca = alloca %S, align 8
  call void @returnsLargeStruct(ptr %sret.alloca)
  %sret.load = load %S, ptr %sret.alloca, align 8
  store %S %sret.load, ptr %s, align 8
  %sret.alloca1 = alloca %S2, align 8
  call void @_EN4main19returnsLargeStruct2E(ptr %sret.alloca1)
  %sret.load2 = load %S2, ptr %sret.alloca1, align 8
  store %S2 %sret.load2, ptr %s2, align 8
  ret i32 0
}

declare void @returnsLargeStruct(ptr sret(%S))
