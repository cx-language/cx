
define { i1, { float } } @_EN4main1fE() {
  %d = alloca { float }, align 8
  %a = alloca { i32, { i1, { float } } }, align 8
  %b = alloca { i1, { float } }, align 8
  %e = alloca float, align 4
  %p = alloca { i1, { float } }*, align 8
  store { float } { float -1.000000e+00 }, { float }* %d, align 4
  %d.load = load { float }, { float }* %d, align 4
  %1 = insertvalue { i1, { float } } { i1 false, { float } undef }, { float } %d.load, 1
  %2 = insertvalue { i32, { i1, { float } } } { i32 42, { i1, { float } } undef }, { i1, { float } } %1, 1
  store { i32, { i1, { float } } } %2, { i32, { i1, { float } } }* %a, align 4
  %b1 = getelementptr inbounds { i32, { i1, { float } } }, { i32, { i1, { float } } }* %a, i32 0, i32 1
  %b.load = load { i1, { float } }, { i1, { float } }* %b1, align 4
  store { i1, { float } } %b.load, { i1, { float } }* %b, align 4
  %d2 = getelementptr inbounds { i1, { float } }, { i1, { float } }* %b, i32 0, i32 1
  %e3 = getelementptr inbounds { float }, { float }* %d2, i32 0, i32 0
  %e.load = load float, float* %e3, align 4
  store float %e.load, float* %e, align 4
  %b4 = getelementptr inbounds { i32, { i1, { float } } }, { i32, { i1, { float } } }* %a, i32 0, i32 1
  store { i1, { float } }* %b4, { i1, { float } }** %p, align 8
  %p.load = load { i1, { float } }*, { i1, { float } }** %p, align 8
  %p.load.load = load { i1, { float } }, { i1, { float } }* %p.load, align 4
  store { i1, { float } } %p.load.load, { i1, { float } }* %b, align 4
  %p.load5 = load { i1, { float } }*, { i1, { float } }** %p, align 8
  %d6 = getelementptr inbounds { i1, { float } }, { i1, { float } }* %p.load5, i32 0, i32 1
  %e7 = getelementptr inbounds { float }, { float }* %d6, i32 0, i32 0
  %e.load8 = load float, float* %e7, align 4
  store float %e.load8, float* %e, align 4
  store float 1.000000e+00, float* %e, align 4
  %b.load9 = load { i1, { float } }, { i1, { float } }* %b, align 4
  ret { i1, { float } } %b.load9
}

define { i1, float } @_EN4main1gE() {
  ret { i1, float } { i1 true, i32 1 }
}
