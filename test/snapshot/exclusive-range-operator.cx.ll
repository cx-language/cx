
%"RangeIterator<int>" = type { i32, i32 }
%"Range<int>" = type { i32, i32 }

@0 = private unnamed_addr constant [54 x i8] c"integer overflow at exclusive-range-operator.cx:7:11\0A\00", align 1

define i32 @main() {
  %p = alloca i32, align 4
  %__iterator = alloca %"RangeIterator<int>", align 8
  %1 = alloca %"Range<int>", align 8
  %i = alloca i32, align 4
  store i32 9, ptr %p, align 4
  call void @_EN3std5RangeI3intE4initE3int3int(ptr %1, i32 0, i32 3)
  %2 = call %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(ptr %1)
  store %"RangeIterator<int>" %2, ptr %__iterator, align 4
  br label %loop.condition

loop.condition:                                   ; preds = %loop.increment, %0
  %3 = call i1 @_EN3std13RangeIteratorI3intE8hasValueE(ptr %__iterator)
  br i1 %3, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %4 = call i32 @_EN3std13RangeIteratorI3intE5valueE(ptr %__iterator)
  store i32 %4, ptr %i, align 4
  %p.load = load i32, ptr %p, align 4
  %i.load = load i32, ptr %i, align 4
  %5 = sext i32 %p.load to i128
  %6 = sext i32 %i.load to i128
  %7 = sub i128 %5, %6
  %8 = trunc i128 %7 to i32
  %9 = sext i32 %8 to i128
  %10 = icmp ne i128 %7, %9
  %11 = xor i1 %10, true
  %overflow.condition = icmp eq i1 %11, false
  br i1 %overflow.condition, label %overflow.fail, label %overflow.success

loop.increment:                                   ; preds = %overflow.success
  call void @_EN3std13RangeIteratorI3intE9incrementE(ptr %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  %p.load1 = load i32, ptr %p, align 4
  ret i32 %p.load1

overflow.fail:                                    ; preds = %loop.body
  call void @_EN3std10assertFailEP4char(ptr @0)
  unreachable

overflow.success:                                 ; preds = %loop.body
  store i32 %8, ptr %p, align 4
  br label %loop.increment
}

declare void @_EN3std5RangeI3intE4initE3int3int(ptr, i32, i32)

declare %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(ptr)

declare i1 @_EN3std13RangeIteratorI3intE8hasValueE(ptr)

declare i32 @_EN3std13RangeIteratorI3intE5valueE(ptr)

declare void @_EN3std13RangeIteratorI3intE9incrementE(ptr)

declare void @_EN3std10assertFailEP4char(ptr)
