@0 = private unnamed_addr constant [4 x i8] c"foo\00"

define i32 @main() {
  %s = alloca [4 x i8]*
  store [4 x i8]* @0, [4 x i8]** %s
  ret i32 0
}
