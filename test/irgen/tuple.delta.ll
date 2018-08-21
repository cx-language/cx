
define { i1, { double } } @_EN4main1fE() {
  %d = alloca { double }
  %a = alloca { i32, { i1, { double } } }
  %b = alloca { i1, { double } }
  %e = alloca double
  %p = alloca { i1, { double } }*
  store { double } { double -1.000000e+00 }, { double }* %d
  %d1 = load { double }, { double }* %d
  %1 = insertvalue { i1, { double } } { i1 false, { double } undef }, { double } %d1, 1
  %2 = insertvalue { i32, { i1, { double } } } { i32 42, { i1, { double } } undef }, { i1, { double } } %1, 1
  store { i32, { i1, { double } } } %2, { i32, { i1, { double } } }* %a
  %3 = getelementptr inbounds { i32, { i1, { double } } }, { i32, { i1, { double } } }* %a, i32 0, i32 1
  %4 = load { i1, { double } }, { i1, { double } }* %3
  store { i1, { double } } %4, { i1, { double } }* %b
  %5 = getelementptr inbounds { i1, { double } }, { i1, { double } }* %b, i32 0, i32 1
  %6 = getelementptr inbounds { double }, { double }* %5, i32 0, i32 0
  %7 = load double, double* %6
  store double %7, double* %e
  %8 = getelementptr inbounds { i32, { i1, { double } } }, { i32, { i1, { double } } }* %a, i32 0, i32 1
  store { i1, { double } }* %8, { i1, { double } }** %p
  %p2 = load { i1, { double } }*, { i1, { double } }** %p
  %9 = load { i1, { double } }, { i1, { double } }* %p2
  store { i1, { double } } %9, { i1, { double } }* %b
  %10 = load { i1, { double } }*, { i1, { double } }** %p
  %11 = getelementptr inbounds { i1, { double } }, { i1, { double } }* %10, i32 0, i32 1
  %12 = getelementptr inbounds { double }, { double }* %11, i32 0, i32 0
  %13 = load double, double* %12
  store double %13, double* %e
  store double 1.000000e+00, double* %e
  %b3 = load { i1, { double } }, { i1, { double } }* %b
  ret { i1, { double } } %b3
}
