
%"ArrayRef<int>" = type { i32*, i32 }

define i32 @main() {
  %1 = alloca [3 x i32]
  %2 = alloca [3 x i32]
  %3 = alloca [3 x i32]
  store [3 x i32] [i32 1, i32 2, i32 3], [3 x i32]* %1
  %4 = getelementptr [3 x i32], [3 x i32]* %1, i32 0, i32 0
  %5 = insertvalue %"ArrayRef<int>" undef, i32* %4, 0
  %6 = insertvalue %"ArrayRef<int>" %5, i32 3, 1
  call void @_EN4main1fI3intEE1aPAR_3int(%"ArrayRef<int>" %6)
  store [3 x i32] [i32 1, i32 2, i32 3], [3 x i32]* %2
  %7 = getelementptr [3 x i32], [3 x i32]* %2, i32 0, i32 0
  %8 = insertvalue %"ArrayRef<int>" undef, i32* %7, 0
  %9 = insertvalue %"ArrayRef<int>" %8, i32 3, 1
  call void @_EN4main1fI3intEE1aPAR_3int(%"ArrayRef<int>" %9)
  store [3 x i32] [i32 1, i32 2, i32 3], [3 x i32]* %3
  %10 = getelementptr [3 x i32], [3 x i32]* %3, i32 0, i32 0
  %11 = insertvalue %"ArrayRef<int>" undef, i32* %10, 0
  %12 = insertvalue %"ArrayRef<int>" %11, i32 3, 1
  call void @_EN4main1fI3intEE1aPAR_3int(%"ArrayRef<int>" %12)
  ret i32 0
}

define void @_EN4main1fI3intEE1aPAR_3int(%"ArrayRef<int>" %a) {
  %s = alloca i32
  %size = extractvalue %"ArrayRef<int>" %a, 1
  store i32 %size, i32* %s
  ret void
}
