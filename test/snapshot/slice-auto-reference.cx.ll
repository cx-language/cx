
%"Slice<int>" = type { ptr, i32 }

define void @_EN4main3fooE5SliceI3intE(%"Slice<int>" %a) {
  %a1 = alloca %"Slice<int>", align 8
  store %"Slice<int>" %a, ptr %a1, align 8
  ret void
}

define i32 @main() {
  %a = alloca [3 x i32], align 4
  store [3 x i32] [i32 1, i32 2, i32 3], ptr %a, align 4
  %1 = getelementptr inbounds [3 x i32], ptr %a, i32 0, i32 0
  %2 = insertvalue %"Slice<int>" undef, ptr %1, 0
  %3 = insertvalue %"Slice<int>" %2, i32 3, 1
  call void @_EN4main3fooE5SliceI3intE(%"Slice<int>" %3)
  call void @_EN4main3bazEPA3_3int(ptr %a)
  ret i32 0
}

define void @_EN4main3bazEPA3_3int(ptr %b) {
  %b1 = alloca ptr, align 8
  store ptr %b, ptr %b1, align 8
  ret void
}
