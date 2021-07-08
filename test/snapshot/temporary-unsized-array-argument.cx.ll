
%"ArrayRef<int>" = type { i32*, i32 }

define i32 @main() {
  %1 = alloca [3 x i32], align 4
  %2 = alloca [3 x i32], align 4
  store [3 x i32] [i32 1, i32 2, i32 3], [3 x i32]* %1, align 4
  %3 = getelementptr inbounds [3 x i32], [3 x i32]* %1, i32 0, i32 0
  %4 = insertvalue %"ArrayRef<int>" undef, i32* %3, 0
  %5 = insertvalue %"ArrayRef<int>" %4, i32 3, 1
  call void @_EN4main1fI3intEE8ArrayRefI3intE(%"ArrayRef<int>" %5)
  store [3 x i32] [i32 1, i32 2, i32 3], [3 x i32]* %2, align 4
  %6 = getelementptr inbounds [3 x i32], [3 x i32]* %2, i32 0, i32 0
  %7 = insertvalue %"ArrayRef<int>" undef, i32* %6, 0
  %8 = insertvalue %"ArrayRef<int>" %7, i32 3, 1
  call void @_EN4main1fI3intEE8ArrayRefI3intE(%"ArrayRef<int>" %8)
  ret i32 0
}

define void @_EN4main1fI3intEE8ArrayRefI3intE(%"ArrayRef<int>" %a) {
  %s = alloca i32, align 4
  %1 = alloca %"ArrayRef<int>", align 8
  store %"ArrayRef<int>" %a, %"ArrayRef<int>"* %1, align 8
  %2 = call i32 @_EN3std8ArrayRefI3intE4sizeE(%"ArrayRef<int>"* %1)
  store i32 %2, i32* %s, align 4
  ret void
}

define i32 @_EN3std8ArrayRefI3intE4sizeE(%"ArrayRef<int>"* %this) {
  %size = getelementptr inbounds %"ArrayRef<int>", %"ArrayRef<int>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  ret i32 %size.load
}
