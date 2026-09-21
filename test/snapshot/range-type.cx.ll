
%"Range<int>" = type { i32, i32 }
%"RangeIterator<int>" = type { i32, i32 }

@0 = private unnamed_addr constant [40 x i8] c"integer overflow at range-type.cx:6:13\0A\00", align 1

define i32 @_EN4main3fooE5RangeI3intE(%"Range<int>" %r) {
  %r1 = alloca %"Range<int>", align 8
  %sum = alloca i32, align 4
  %__iterator = alloca %"RangeIterator<int>", align 8
  %i = alloca i32, align 4
  store %"Range<int>" %r, ptr %r1, align 4
  store i32 0, ptr %sum, align 4
  %1 = call %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(ptr %r1)
  store %"RangeIterator<int>" %1, ptr %__iterator, align 4
  br label %loop.condition

loop.condition:                                   ; preds = %loop.increment, %0
  %2 = call i1 @_EN3std13RangeIteratorI3intE8hasValueE(ptr %__iterator)
  br i1 %2, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %3 = call i32 @_EN3std13RangeIteratorI3intE5valueE(ptr %__iterator)
  store i32 %3, ptr %i, align 4
  %sum.load = load i32, ptr %sum, align 4
  %i.load = load i32, ptr %i, align 4
  %4 = sext i32 %sum.load to i64
  %5 = sext i32 %i.load to i64
  %6 = add i64 %4, %5
  %7 = trunc i64 %6 to i32
  %8 = sext i32 %7 to i64
  %9 = icmp ne i64 %6, %8
  %10 = xor i1 %9, true
  %overflow.condition = icmp eq i1 %10, false
  br i1 %overflow.condition, label %overflow.fail, label %overflow.success

loop.increment:                                   ; preds = %overflow.success
  call void @_EN3std13RangeIteratorI3intE9incrementE(ptr %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  %sum.load2 = load i32, ptr %sum, align 4
  ret i32 %sum.load2

overflow.fail:                                    ; preds = %loop.body
  call void @_EN3std10assertFailEP4char(ptr @0)
  unreachable

overflow.success:                                 ; preds = %loop.body
  store i32 %7, ptr %sum, align 4
  br label %loop.increment
}

declare %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(ptr)

declare i1 @_EN3std13RangeIteratorI3intE8hasValueE(ptr)

declare i32 @_EN3std13RangeIteratorI3intE5valueE(ptr)

declare void @_EN3std13RangeIteratorI3intE9incrementE(ptr)

declare void @_EN3std10assertFailEP4char(ptr)

define i32 @main() {
  %1 = alloca %"Range<int>", align 8
  call void @_EN3std5RangeI3intE4initE3int3int(ptr %1, i32 0, i32 5)
  %.load = load %"Range<int>", ptr %1, align 4
  %2 = call i32 @_EN4main3fooE5RangeI3intE(%"Range<int>" %.load)
  ret i32 %2
}

declare void @_EN3std5RangeI3intE4initE3int3int(ptr, i32, i32)
