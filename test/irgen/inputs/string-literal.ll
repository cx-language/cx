@0 = private unnamed_addr constant [4 x i8] c"foo\00"

define i32 @main() {
  %s = alloca { i8*, i32 }
  store { i8*, i32 } { i8* getelementptr inbounds ([4 x i8], [4 x i8]* @0, i32 0, i32 0), i32 3 }, { i8*, i32 }* %s
  ret i32 0
}
