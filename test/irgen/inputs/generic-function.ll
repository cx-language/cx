@0 = private unnamed_addr constant [4 x i8] c"bar\00"

define i32 @main() {
  %b = alloca [4 x i8]*
  %five = alloca i32
  call void @"foo<int>"(i32 1)
  call void @"foo<bool>"(i1 false)
  call void @"foo<bool>"(i1 true)
  %1 = call [4 x i8]* @"bar<char[4]&>"([4 x i8]* @0)
  store [4 x i8]* %1, [4 x i8]** %b
  %2 = call i32 @"qux<int>"(i32 -5)
  store i32 %2, i32* %five
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

define i32 @"qux<int>"(i32 %t) {
  %1 = icmp slt i32 %t, 0
  br i1 %1, label %then, label %else

then:                                             ; preds = %0
  %2 = sub i32 0, %t
  ret i32 %2

else:                                             ; preds = %0
  br label %endif

endif:                                            ; preds = %else
  ret i32 %t
}
