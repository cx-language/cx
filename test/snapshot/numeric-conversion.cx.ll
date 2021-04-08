
define i32 @main() {
  %i = alloca i16, align 2
  %f = alloca float, align 4
  %u = alloca i64, align 8
  %s = alloca i32, align 4
  store i16 42, i16* %i, align 2
  %i.load = load i16, i16* %i, align 2
  %1 = uitofp i16 %i.load to float
  store float %1, float* %f, align 4
  %f.load = load float, float* %f, align 4
  %2 = fptosi float %f.load to i64
  store i64 %2, i64* %u, align 4
  store i32 1, i32* %s, align 4
  ret i32 0
}
