
define i32 @main() {
  %buffer = alloca [4 x i8]*, align 8
  %buffer.load = load [4 x i8]*, [4 x i8]** %buffer, align 8
  %1 = getelementptr inbounds [4 x i8], [4 x i8]* %buffer.load, i32 0, i32 1
  store i8 -1, i8* %1, align 1
  ret i32 0
}
