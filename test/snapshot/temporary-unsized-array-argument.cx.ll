
%"Slice<int>" = type { ptr, i32 }

define i32 @main() {
  %1 = alloca [3 x i32], align 4
  %2 = alloca [3 x i32], align 4
  store [3 x i32] [i32 1, i32 2, i32 3], ptr %1, align 4
  %3 = getelementptr inbounds [3 x i32], ptr %1, i32 0, i32 0
  %4 = insertvalue %"Slice<int>" undef, ptr %3, 0
  %5 = insertvalue %"Slice<int>" %4, i32 3, 1
  call void @_EN4main1fI3intEE5SliceI3intE(%"Slice<int>" %5)
  store [3 x i32] [i32 1, i32 2, i32 3], ptr %2, align 4
  %6 = getelementptr inbounds [3 x i32], ptr %2, i32 0, i32 0
  %7 = insertvalue %"Slice<int>" undef, ptr %6, 0
  %8 = insertvalue %"Slice<int>" %7, i32 3, 1
  call void @_EN4main1fI3intEE5SliceI3intE(%"Slice<int>" %8)
  ret i32 0
}

define void @_EN4main1fI3intEE5SliceI3intE(%"Slice<int>" %a) {
  %a1 = alloca %"Slice<int>", align 8
  %s = alloca i32, align 4
  store %"Slice<int>" %a, ptr %a1, align 8
  %1 = call i32 @_EN3std5SliceI3intE4sizeE(ptr %a1)
  store i32 %1, ptr %s, align 4
  ret void
}

define i32 @_EN3std5SliceI3intE4sizeE(ptr %this) {
  %size = getelementptr inbounds %"Slice<int>", ptr %this, i32 0, i32 1
  %size.load = load i32, ptr %size, align 4
  ret i32 %size.load
}
