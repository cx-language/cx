@0 = private unnamed_addr constant [8 x i8] c"foo bar\00"

define i32 @main() {
  %1 = call i32 @puts(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @0, i32 0, i32 0))
  ret i32 0
}

declare i32 @puts(i8*)
