
@0 = private unnamed_addr constant [6 x i8] c"%d %f\00", align 1

define i32 @main() {
  %f = alloca double
  store double 4.200000e+00, double* %f
  %f.load = load double, double* %f
  %1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @0, i32 0, i32 0), i32 -3, double %f.load)
  ret i32 0
}

declare i32 @printf(i8*, ...)
