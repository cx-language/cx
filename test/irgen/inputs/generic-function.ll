@0 = private unnamed_addr constant [4 x i8] c"bar\00"

define i32 @main() {
  call void @"foo<int>"(i32 1)
  call void @"foo<bool>"(i1 false)
  call void @"foo<bool>"(i1 true)
  %1 = call [4 x i8]* @"bar<char[4]&>"([4 x i8]* @0)
  ret i32 0
}

define void @"foo<int>"(i32 %t) {
  ret void
}

define void @"foo<bool>"(i1 %t) {
  ret void
}

define [4 x i8]* @"bar<char[4]&>"([4 x i8]* %t) {
  ret [4 x i8]* %t
}
