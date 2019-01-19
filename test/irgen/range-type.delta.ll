
%"Range<int>" = type { i32, i32 }
%"RangeIterator<int>" = type { i32, i32 }

define i32 @_EN4main3fooE1r5RangeI3intE(%"Range<int>" %r) {
  %sum = alloca i32
  %__iterator = alloca %"RangeIterator<int>"
  %i = alloca i32
  store i32 0, i32* %sum
  %1 = call %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(%"Range<int>" %r)
  store %"RangeIterator<int>" %1, %"RangeIterator<int>"* %__iterator
  br label %loop.condition

loop.condition:                                   ; preds = %loop.increment, %0
  %__iterator.load = load %"RangeIterator<int>", %"RangeIterator<int>"* %__iterator
  %2 = call i1 @_EN3std13RangeIteratorI3intE8hasValueE(%"RangeIterator<int>" %__iterator.load)
  br i1 %2, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %__iterator.load1 = load %"RangeIterator<int>", %"RangeIterator<int>"* %__iterator
  %3 = call i32 @_EN3std13RangeIteratorI3intE5valueE(%"RangeIterator<int>" %__iterator.load1)
  store i32 %3, i32* %i
  %sum.load = load i32, i32* %sum
  %i.load = load i32, i32* %i
  %4 = add i32 %sum.load, %i.load
  store i32 %4, i32* %sum
  br label %loop.increment

loop.increment:                                   ; preds = %loop.body
  call void @_ENM3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  %sum.load2 = load i32, i32* %sum
  ret i32 %sum.load2
}

declare %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(%"Range<int>")

declare i1 @_EN3std13RangeIteratorI3intE8hasValueE(%"RangeIterator<int>")

declare i32 @_EN3std13RangeIteratorI3intE5valueE(%"RangeIterator<int>")

declare void @_ENM3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"*)

define i32 @main() {
  %1 = alloca %"Range<int>"
  call void @_ENM3std5RangeI3intE4initE5start3int3end3int(%"Range<int>"* %1, i32 0, i32 5)
  %.load = load %"Range<int>", %"Range<int>"* %1
  %2 = call i32 @_EN4main3fooE1r5RangeI3intE(%"Range<int>" %.load)
  ret i32 %2
}

declare void @_ENM3std5RangeI3intE4initE5start3int3end3int(%"Range<int>"*, i32, i32)
