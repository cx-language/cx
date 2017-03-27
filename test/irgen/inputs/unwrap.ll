declare i8* @malloc(i64)

define i32 @main() {
  %byte = alloca i8*
  %ptr = alloca [1 x i8]*
  %1 = call i8* @malloc(i64 1)
  store i8* %1, i8** %byte
  %2 = call i8* @malloc(i64 1)
  %3 = bitcast i8* %2 to [1 x i8]*
  store [1 x i8]* %3, [1 x i8]** %ptr
  %ptr1 = load [1 x i8]*, [1 x i8]** %ptr
  %4 = getelementptr [1 x i8], [1 x i8]* %ptr1, i32 0, i32 0
  store i8 1, i8* %4
  ret i32 0
}
