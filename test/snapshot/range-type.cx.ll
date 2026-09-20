
%"Range<int>" = type { i32, i32 }
%"RangeIterator<int>" = type { i32, i32 }

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
  %4 = add i32 %sum.load, %i.load
  store i32 %4, ptr %sum, align 4
  br label %loop.increment

loop.increment:                                   ; preds = %loop.body
  call void @_EN3std13RangeIteratorI3intE9incrementE(ptr %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  %sum.load2 = load i32, ptr %sum, align 4
  ret i32 %sum.load2
}

declare %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(ptr)

declare i1 @_EN3std13RangeIteratorI3intE8hasValueE(ptr)

declare i32 @_EN3std13RangeIteratorI3intE5valueE(ptr)

declare void @_EN3std13RangeIteratorI3intE9incrementE(ptr)

define i32 @main() {
  %1 = alloca %"Range<int>", align 8
  call void @_EN3std5RangeI3intE4initE3int3int(ptr %1, i32 0, i32 5)
  %.load = load %"Range<int>", ptr %1, align 4
  %2 = call i32 @_EN4main3fooE5RangeI3intE(%"Range<int>" %.load)
  ret i32 %2
}

declare void @_EN3std5RangeI3intE4initE3int3int(ptr, i32, i32)
