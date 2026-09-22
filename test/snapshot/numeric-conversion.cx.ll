
define i32 @main() {
  %i = alloca i16, align 2
  %f = alloca float, align 4
  %u = alloca i64, align 8
  %s = alloca i32, align 4
  store i16 42, ptr %i, align 2
  %i.load = load i16, ptr %i, align 2
  %1 = uitofp i16 %i.load to float
  store float %1, ptr %f, align 4
  %f.load = load float, ptr %f, align 4
  %2 = fptosi float %f.load to i64
  store i64 %2, ptr %u, align 8
  store i32 1, ptr %s, align 4
  ret i32 0
}
