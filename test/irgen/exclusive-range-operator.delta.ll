
%"RangeIterator<int>" = type { i32, i32 }
%"Range<int>" = type { i32, i32 }

define i32 @main() {
  %p = alloca i32
  %__iterator = alloca %"RangeIterator<int>"
  %1 = alloca %"Range<int>"
  %i = alloca i32
  store i32 9, i32* %p
  call void @_ENM3std5RangeI3intE4initE5start3int3end3int(%"Range<int>"* %1, i32 0, i32 3)
  %2 = load %"Range<int>", %"Range<int>"* %1
  %3 = call %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(%"Range<int>" %2)
  store %"RangeIterator<int>" %3, %"RangeIterator<int>"* %__iterator
  br label %loop.condition

loop.condition:                                   ; preds = %loop.increment, %0
  %__iterator1 = load %"RangeIterator<int>", %"RangeIterator<int>"* %__iterator
  %4 = call i1 @_EN3std13RangeIteratorI3intE8hasValueE(%"RangeIterator<int>" %__iterator1)
  br i1 %4, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %__iterator2 = load %"RangeIterator<int>", %"RangeIterator<int>"* %__iterator
  %5 = call i32 @_EN3std13RangeIteratorI3intE5valueE(%"RangeIterator<int>" %__iterator2)
  store i32 %5, i32* %i
  %6 = load i32, i32* %p
  %i3 = load i32, i32* %i
  %7 = sub i32 %6, %i3
  store i32 %7, i32* %p
  br label %loop.increment

loop.increment:                                   ; preds = %loop.body
  call void @_ENM3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  %p4 = load i32, i32* %p
  ret i32 %p4
}

declare %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(%"Range<int>")

declare void @_ENM3std5RangeI3intE4initE5start3int3end3int(%"Range<int>"*, i32, i32)

declare i1 @_EN3std13RangeIteratorI3intE8hasValueE(%"RangeIterator<int>")

declare i32 @_EN3std13RangeIteratorI3intE5valueE(%"RangeIterator<int>")

declare void @_ENM3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"*)
