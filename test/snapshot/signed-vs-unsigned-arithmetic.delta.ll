
define void @_EN4main3fooE3int4uint(i32 %i, i32 %u) {
  %a = alloca i1, align 1
  %b = alloca i32, align 4
  %c = alloca i32, align 4
  %1 = icmp slt i32 %i, %i
  store i1 %1, i1* %a, align 1
  %2 = icmp ult i32 %u, %u
  store i1 %2, i1* %a, align 1
  %3 = icmp sgt i32 %i, %i
  store i1 %3, i1* %a, align 1
  %4 = icmp ugt i32 %u, %u
  store i1 %4, i1* %a, align 1
  %5 = icmp sle i32 %i, %i
  store i1 %5, i1* %a, align 1
  %6 = icmp ule i32 %u, %u
  store i1 %6, i1* %a, align 1
  %7 = icmp sge i32 %i, %i
  store i1 %7, i1* %a, align 1
  %8 = icmp uge i32 %u, %u
  store i1 %8, i1* %a, align 1
  %9 = sdiv i32 %i, %i
  store i32 %9, i32* %b, align 4
  %10 = udiv i32 %u, %u
  store i32 %10, i32* %c, align 4
  ret void
}
