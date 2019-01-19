
define { i1, { double } } @_EN4main1fE() {
  %d = alloca { double }
  %a = alloca { i32, { i1, { double } } }
  %b = alloca { i1, { double } }
  %e = alloca double
  %p = alloca { i1, { double } }*
  store { double } { double -1.000000e+00 }, { double }* %d
  %d.load = load { double }, { double }* %d
  %1 = insertvalue { i1, { double } } { i1 false, { double } undef }, { double } %d.load, 1
  %2 = insertvalue { i32, { i1, { double } } } { i32 42, { i1, { double } } undef }, { i1, { double } } %1, 1
  store { i32, { i1, { double } } } %2, { i32, { i1, { double } } }* %a
  %3 = getelementptr inbounds { i32, { i1, { double } } }, { i32, { i1, { double } } }* %a, i32 0, i32 1
  %.load = load { i1, { double } }, { i1, { double } }* %3
  store { i1, { double } } %.load, { i1, { double } }* %b
  %4 = getelementptr inbounds { i1, { double } }, { i1, { double } }* %b, i32 0, i32 1
  %5 = getelementptr inbounds { double }, { double }* %4, i32 0, i32 0
  %.load1 = load double, double* %5
  store double %.load1, double* %e
  %6 = getelementptr inbounds { i32, { i1, { double } } }, { i32, { i1, { double } } }* %a, i32 0, i32 1
  store { i1, { double } }* %6, { i1, { double } }** %p
  %p.load = load { i1, { double } }*, { i1, { double } }** %p
  %p.load.load = load { i1, { double } }, { i1, { double } }* %p.load
  store { i1, { double } } %p.load.load, { i1, { double } }* %b
  %p.load2 = load { i1, { double } }*, { i1, { double } }** %p
  %7 = getelementptr inbounds { i1, { double } }, { i1, { double } }* %p.load2, i32 0, i32 1
  %8 = getelementptr inbounds { double }, { double }* %7, i32 0, i32 0
  %.load3 = load double, double* %8
  store double %.load3, double* %e
  store double 1.000000e+00, double* %e
  %b.load = load { i1, { double } }, { i1, { double } }* %b
  ret { i1, { double } } %b.load
}
