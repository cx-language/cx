
%"M<int>" = type {}
%"Slice<int>" = type { ptr, i32 }

define i32 @main() {
  %b = alloca [3 x i32], align 4
  %1 = alloca %"M<int>", align 8
  store [3 x i32] [i32 1, i32 2, i32 3], ptr %b, align 4
  %2 = getelementptr inbounds [3 x i32], ptr %b, i32 0, i32 0
  %3 = insertvalue %"Slice<int>" undef, ptr %2, 0
  %4 = insertvalue %"Slice<int>" %3, i32 3, 1
  call void @_EN4main1MI3intE4initE5SliceI3intE(ptr %1, %"Slice<int>" %4)
  ret i32 0
}

define void @_EN4main1MI3intE4initE5SliceI3intE(ptr %this, %"Slice<int>" %a) {
  %a1 = alloca %"Slice<int>", align 8
  store %"Slice<int>" %a, ptr %a1, align 8
  ret void
}
