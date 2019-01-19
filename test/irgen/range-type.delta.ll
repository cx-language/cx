
%"Range<int>" = type { i32, i32 }
%"RangeIterator<int>" = type { i32, i32 }

define i32 @_EN4main3fooE1r5RangeI3intE(%"Range<int>" %r) {
  %sum = alloca i32
  %__iterator5 = alloca %"RangeIterator<int>"
  %i = alloca i32
  store i32 0, i32* %sum
  %1 = call %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(%"Range<int>" %r)
  store %"RangeIterator<int>" %1, %"RangeIterator<int>"* %__iterator5
  br label %loop.condition

loop.condition:                                   ; preds = %loop.increment, %0
  %__iterator51 = load %"RangeIterator<int>", %"RangeIterator<int>"* %__iterator5
  %2 = call i1 @_EN3std13RangeIteratorI3intE8hasValueE(%"RangeIterator<int>" %__iterator51)
  br i1 %2, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %__iterator52 = load %"RangeIterator<int>", %"RangeIterator<int>"* %__iterator5
  %3 = call i32 @_EN3std13RangeIteratorI3intE5valueE(%"RangeIterator<int>" %__iterator52)
  store i32 %3, i32* %i
  %4 = load i32, i32* %sum
  %i3 = load i32, i32* %i
  %5 = add i32 %4, %i3
  store i32 %5, i32* %sum
  br label %loop.increment

loop.increment:                                   ; preds = %loop.body
  call void @_ENM3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"* %__iterator5)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  %sum4 = load i32, i32* %sum
  ret i32 %sum4
}

declare %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(%"Range<int>")

declare i1 @_EN3std13RangeIteratorI3intE8hasValueE(%"RangeIterator<int>")

declare i32 @_EN3std13RangeIteratorI3intE5valueE(%"RangeIterator<int>")

declare void @_ENM3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"*)

define i32 @main() {
  %1 = alloca %"Range<int>"
  call void @_ENM3std5RangeI3intE4initE5start3int3end3int(%"Range<int>"* %1, i32 0, i32 5)
  %2 = load %"Range<int>", %"Range<int>"* %1
  %3 = call i32 @_EN4main3fooE1r5RangeI3intE(%"Range<int>" %2)
  ret i32 %3
}

declare void @_ENM3std5RangeI3intE4initE5start3int3end3int(%"Range<int>"*, i32, i32)
