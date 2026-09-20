
%"M<int>" = type {}
%"ArrayRef<int>" = type { ptr, i32 }

define i32 @main() {
  %b = alloca [3 x i32], align 4
  %1 = alloca %"M<int>", align 8
  store [3 x i32] [i32 1, i32 2, i32 3], ptr %b, align 4
  %2 = getelementptr inbounds [3 x i32], ptr %b, i32 0, i32 0
  %3 = insertvalue %"ArrayRef<int>" undef, ptr %2, 0
  %4 = insertvalue %"ArrayRef<int>" %3, i32 3, 1
  call void @_EN4main1MI3intE4initE8ArrayRefI3intE(ptr %1, %"ArrayRef<int>" %4)
  ret i32 0
}

define void @_EN4main1MI3intE4initE8ArrayRefI3intE(ptr %this, %"ArrayRef<int>" %a) {
  %a1 = alloca %"ArrayRef<int>", align 8
  store %"ArrayRef<int>" %a, ptr %a1, align 8
  ret void
}
