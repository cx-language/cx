
%"RangeIterator<int>" = type { i32, i32 }
%"Range<int>" = type { i32, i32 }

define i32 @main() {
  %p = alloca i32
  %__iterator = alloca %"RangeIterator<int>"
  %1 = alloca %"Range<int>"
  %i = alloca i32
  store i32 9, i32* %p
  call void @_ENM3std5RangeI3intE4initE5start3int3end3int(%"Range<int>"* %1, i32 0, i32 3)
  %.load = load %"Range<int>", %"Range<int>"* %1
  %2 = call %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(%"Range<int>" %.load)
  store %"RangeIterator<int>" %2, %"RangeIterator<int>"* %__iterator
  br label %loop.condition

loop.condition:                                   ; preds = %loop.increment, %0
  %__iterator.load = load %"RangeIterator<int>", %"RangeIterator<int>"* %__iterator
  %3 = call i1 @_EN3std13RangeIteratorI3intE8hasValueE(%"RangeIterator<int>" %__iterator.load)
  br i1 %3, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %__iterator.load1 = load %"RangeIterator<int>", %"RangeIterator<int>"* %__iterator
  %4 = call i32 @_EN3std13RangeIteratorI3intE5valueE(%"RangeIterator<int>" %__iterator.load1)
  store i32 %4, i32* %i
  %p.load = load i32, i32* %p
  %i.load = load i32, i32* %i
  %5 = sub i32 %p.load, %i.load
  store i32 %5, i32* %p
  br label %loop.increment

loop.increment:                                   ; preds = %loop.body
  call void @_ENM3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  %p.load2 = load i32, i32* %p
  ret i32 %p.load2
}

declare %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(%"Range<int>")

declare void @_ENM3std5RangeI3intE4initE5start3int3end3int(%"Range<int>"*, i32, i32)

declare i1 @_EN3std13RangeIteratorI3intE8hasValueE(%"RangeIterator<int>")

declare i32 @_EN3std13RangeIteratorI3intE5valueE(%"RangeIterator<int>")

declare void @_ENM3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"*)
