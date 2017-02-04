@0 = private unnamed_addr constant [4 x i8] c"foo\00"

define i32 @main() {
  %s = alloca i8*
  store i8* getelementptr inbounds ([4 x i8], [4 x i8]* @0, i32 0, i32 0), i8** %s
  ret i32 0
}
