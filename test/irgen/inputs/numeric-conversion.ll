define void @foo(i16 %i) {
  %f = alloca float
  %u = alloca i64
  %1 = uitofp i16 %i to float
  store float %1, float* %f
  %f1 = load float, float* %f
  %2 = fptosi float %f1 to i64
  store i64 %2, i64* %u
  ret void
}
