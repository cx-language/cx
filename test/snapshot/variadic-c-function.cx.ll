
@0 = private unnamed_addr constant [6 x i8] c"%d %f\00", align 1

define i32 @main() {
  %f = alloca float, align 4
  store float 4.200000e+00, ptr %f, align 4
  %f.load = load float, ptr %f, align 4
  %1 = call i32 (ptr, ...) @printf(ptr @0, i32 -3, float %f.load)
  ret i32 0
}

declare i32 @printf(ptr, ...)
