
%"ClosedRangeIterator<int>" = type { i32, i32 }
%"ClosedRange<int>" = type { i32, i32 }

define i32 @main() {
  %__iterator = alloca %"ClosedRangeIterator<int>"
  %1 = alloca %"ClosedRange<int>"
  %i = alloca i32
  %__iterator2 = alloca %"ClosedRangeIterator<int>"
  %2 = alloca %"ClosedRange<int>"
  %i8 = alloca i32
  call void @_ENM3std11ClosedRangeI3intE4initE5start3int3end3int(%"ClosedRange<int>"* %1, i32 0, i32 5)
  %.load = load %"ClosedRange<int>", %"ClosedRange<int>"* %1
  %3 = call %"ClosedRangeIterator<int>" @_EN3std11ClosedRangeI3intE8iteratorE(%"ClosedRange<int>" %.load)
  store %"ClosedRangeIterator<int>" %3, %"ClosedRangeIterator<int>"* %__iterator
  br label %loop.condition

loop.condition:                                   ; preds = %loop.increment, %0
  %__iterator.load = load %"ClosedRangeIterator<int>", %"ClosedRangeIterator<int>"* %__iterator
  %4 = call i1 @_EN3std19ClosedRangeIteratorI3intE8hasValueE(%"ClosedRangeIterator<int>" %__iterator.load)
  br i1 %4, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %__iterator.load1 = load %"ClosedRangeIterator<int>", %"ClosedRangeIterator<int>"* %__iterator
  %5 = call i32 @_EN3std19ClosedRangeIteratorI3intE5valueE(%"ClosedRangeIterator<int>" %__iterator.load1)
  store i32 %5, i32* %i
  br label %loop.increment

loop.increment:                                   ; preds = %loop.body
  call void @_ENM3std19ClosedRangeIteratorI3intE9incrementE(%"ClosedRangeIterator<int>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  call void @_ENM3std11ClosedRangeI3intE4initE5start3int3end3int(%"ClosedRange<int>"* %2, i32 0, i32 5)
  %.load3 = load %"ClosedRange<int>", %"ClosedRange<int>"* %2
  %6 = call %"ClosedRangeIterator<int>" @_EN3std11ClosedRangeI3intE8iteratorE(%"ClosedRange<int>" %.load3)
  store %"ClosedRangeIterator<int>" %6, %"ClosedRangeIterator<int>"* %__iterator2
  br label %loop.condition4

loop.condition4:                                  ; preds = %loop.increment6, %loop.end
  %__iterator2.load = load %"ClosedRangeIterator<int>", %"ClosedRangeIterator<int>"* %__iterator2
  %7 = call i1 @_EN3std19ClosedRangeIteratorI3intE8hasValueE(%"ClosedRangeIterator<int>" %__iterator2.load)
  br i1 %7, label %loop.body5, label %loop.end7

loop.body5:                                       ; preds = %loop.condition4
  %__iterator2.load9 = load %"ClosedRangeIterator<int>", %"ClosedRangeIterator<int>"* %__iterator2
  %8 = call i32 @_EN3std19ClosedRangeIteratorI3intE5valueE(%"ClosedRangeIterator<int>" %__iterator2.load9)
  store i32 %8, i32* %i8
  br label %loop.increment6

loop.increment6:                                  ; preds = %loop.body5
  call void @_ENM3std19ClosedRangeIteratorI3intE9incrementE(%"ClosedRangeIterator<int>"* %__iterator2)
  br label %loop.condition4

loop.end7:                                        ; preds = %loop.condition4
  ret i32 0
}

define void @_ENM3std11ClosedRangeI3intE4initE5start3int3end3int(%"ClosedRange<int>"* %this, i32 %start, i32 %end) {
  %start1 = getelementptr inbounds %"ClosedRange<int>", %"ClosedRange<int>"* %this, i32 0, i32 0
  store i32 %start, i32* %start1
  %end2 = getelementptr inbounds %"ClosedRange<int>", %"ClosedRange<int>"* %this, i32 0, i32 1
  store i32 %end, i32* %end2
  ret void
}

define void @_ENM3std11ClosedRangeI3intE4initE3end3int(%"ClosedRange<int>"* %this, i32 %end) {
  call void @_ENM3std11ClosedRangeI3intE4initE5start3int3end3int(%"ClosedRange<int>"* %this, i32 0, i32 %end)
  ret void
}

define i32 @_EN3std11ClosedRangeI3intE4sizeE(%"ClosedRange<int>" %this) {
  %end = extractvalue %"ClosedRange<int>" %this, 1
  %start = extractvalue %"ClosedRange<int>" %this, 0
  %1 = sub i32 %end, %start
  %2 = add i32 %1, 1
  ret i32 %2
}

define i32 @_EN3std11ClosedRangeI3intE6lengthE(%"ClosedRange<int>" %this) {
  %1 = call i32 @_EN3std11ClosedRangeI3intE4sizeE(%"ClosedRange<int>" %this)
  ret i32 %1
}

define i32 @_EN3std11ClosedRangeI3intE5startE(%"ClosedRange<int>" %this) {
  %start = extractvalue %"ClosedRange<int>" %this, 0
  ret i32 %start
}

define i32 @_EN3std11ClosedRangeI3intE3endE(%"ClosedRange<int>" %this) {
  %end = extractvalue %"ClosedRange<int>" %this, 1
  ret i32 %end
}

define %"ClosedRangeIterator<int>" @_EN3std11ClosedRangeI3intE8iteratorE(%"ClosedRange<int>" %this) {
  %1 = alloca %"ClosedRangeIterator<int>"
  call void @_ENM3std19ClosedRangeIteratorI3intE4initE5range11ClosedRangeI3intE(%"ClosedRangeIterator<int>"* %1, %"ClosedRange<int>" %this)
  %.load = load %"ClosedRangeIterator<int>", %"ClosedRangeIterator<int>"* %1
  ret %"ClosedRangeIterator<int>" %.load
}

define void @_ENM3std19ClosedRangeIteratorI3intE4initE5range11ClosedRangeI3intE(%"ClosedRangeIterator<int>"* %this, %"ClosedRange<int>" %range) {
  %current = getelementptr inbounds %"ClosedRangeIterator<int>", %"ClosedRangeIterator<int>"* %this, i32 0, i32 0
  %1 = call i32 @_EN3std11ClosedRangeI3intE5startE(%"ClosedRange<int>" %range)
  store i32 %1, i32* %current
  %end = getelementptr inbounds %"ClosedRangeIterator<int>", %"ClosedRangeIterator<int>"* %this, i32 0, i32 1
  %2 = call i32 @_EN3std11ClosedRangeI3intE3endE(%"ClosedRange<int>" %range)
  store i32 %2, i32* %end
  ret void
}

declare void @_ENM3std19ClosedRangeIteratorI3intE4initE5range11ClosedRangeI3intE.1(%"ClosedRangeIterator<int>"*, %"ClosedRange<int>")

define i1 @_EN3std19ClosedRangeIteratorI3intE8hasValueE(%"ClosedRangeIterator<int>" %this) {
  %current = extractvalue %"ClosedRangeIterator<int>" %this, 0
  %end = extractvalue %"ClosedRangeIterator<int>" %this, 1
  %1 = icmp sle i32 %current, %end
  ret i1 %1
}

define i32 @_EN3std19ClosedRangeIteratorI3intE5valueE(%"ClosedRangeIterator<int>" %this) {
  %current = extractvalue %"ClosedRangeIterator<int>" %this, 0
  ret i32 %current
}

define void @_ENM3std19ClosedRangeIteratorI3intE9incrementE(%"ClosedRangeIterator<int>"* %this) {
  %current = getelementptr inbounds %"ClosedRangeIterator<int>", %"ClosedRangeIterator<int>"* %this, i32 0, i32 0
  %current.load = load i32, i32* %current
  %1 = add i32 %current.load, 1
  store i32 %1, i32* %current
  ret void
}
