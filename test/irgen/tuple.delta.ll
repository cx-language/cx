
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
  %b1 = getelementptr inbounds { i32, { i1, { double } } }, { i32, { i1, { double } } }* %a, i32 0, i32 1
  %b1.load = load { i1, { double } }, { i1, { double } }* %b1
  store { i1, { double } } %b1.load, { i1, { double } }* %b
  %d2 = getelementptr inbounds { i1, { double } }, { i1, { double } }* %b, i32 0, i32 1
  %e3 = getelementptr inbounds { double }, { double }* %d2, i32 0, i32 0
  %e3.load = load double, double* %e3
  store double %e3.load, double* %e
  %b4 = getelementptr inbounds { i32, { i1, { double } } }, { i32, { i1, { double } } }* %a, i32 0, i32 1
  store { i1, { double } }* %b4, { i1, { double } }** %p
  %p.load = load { i1, { double } }*, { i1, { double } }** %p
  %p.load.load = load { i1, { double } }, { i1, { double } }* %p.load
  store { i1, { double } } %p.load.load, { i1, { double } }* %b
  %p.load5 = load { i1, { double } }*, { i1, { double } }** %p
  %d6 = getelementptr inbounds { i1, { double } }, { i1, { double } }* %p.load5, i32 0, i32 1
  %e7 = getelementptr inbounds { double }, { double }* %d6, i32 0, i32 0
  %e7.load = load double, double* %e7
  store double %e7.load, double* %e
  store double 1.000000e+00, double* %e
  %b.load = load { i1, { double } }, { i1, { double } }* %b
  ret { i1, { double } } %b.load
}
