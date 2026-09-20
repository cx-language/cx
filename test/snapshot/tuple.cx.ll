
define { i1, { float } } @_EN4main1fE() {
  %d = alloca { float }, align 8
  %a = alloca { i32, { i1, { float } } }, align 8
  %b = alloca { i1, { float } }, align 8
  %e = alloca float, align 4
  %p = alloca ptr, align 8
  store { float } { float -1.000000e+00 }, ptr %d, align 4
  %d.load = load { float }, ptr %d, align 4
  %1 = insertvalue { i1, { float } } { i1 false, { float } undef }, { float } %d.load, 1
  %2 = insertvalue { i32, { i1, { float } } } { i32 42, { i1, { float } } undef }, { i1, { float } } %1, 1
  store { i32, { i1, { float } } } %2, ptr %a, align 4
  %b1 = getelementptr inbounds { i32, { i1, { float } } }, ptr %a, i32 0, i32 1
  %b.load = load { i1, { float } }, ptr %b1, align 4
  store { i1, { float } } %b.load, ptr %b, align 4
  %d2 = getelementptr inbounds { i1, { float } }, ptr %b, i32 0, i32 1
  %e3 = getelementptr inbounds { float }, ptr %d2, i32 0, i32 0
  %e.load = load float, ptr %e3, align 4
  store float %e.load, ptr %e, align 4
  %b4 = getelementptr inbounds { i32, { i1, { float } } }, ptr %a, i32 0, i32 1
  store ptr %b4, ptr %p, align 8
  %p.load = load ptr, ptr %p, align 8
  %p.load.load = load { i1, { float } }, ptr %p.load, align 4
  store { i1, { float } } %p.load.load, ptr %b, align 4
  %p.load5 = load ptr, ptr %p, align 8
  %d6 = getelementptr inbounds { i1, { float } }, ptr %p.load5, i32 0, i32 1
  %e7 = getelementptr inbounds { float }, ptr %d6, i32 0, i32 0
  %e.load8 = load float, ptr %e7, align 4
  store float %e.load8, ptr %e, align 4
  store float 1.000000e+00, ptr %e, align 4
  %b.load9 = load { i1, { float } }, ptr %b, align 4
  ret { i1, { float } } %b.load9
}

define { i1, float } @_EN4main1gE() {
  ret { i1, float } { i1 true, float 1.000000e+00 }
}
