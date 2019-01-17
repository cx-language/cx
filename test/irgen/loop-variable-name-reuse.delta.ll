
%"ClosedRangeIterator<int>" = type { i32, i32 }
%"ClosedRange<int>" = type { i32, i32 }

define i32 @main() {
  %__iterator4 = alloca %"ClosedRangeIterator<int>"
  %1 = alloca %"ClosedRange<int>"
  %i = alloca i32
  %__iterator5 = alloca %"ClosedRangeIterator<int>"
  %2 = alloca %"ClosedRange<int>"
  %i8 = alloca i32
  call void @_ENM3std11ClosedRangeI3intE4initE5start3int3end3int(%"ClosedRange<int>"* %1, i32 0, i32 5)
  %3 = load %"ClosedRange<int>", %"ClosedRange<int>"* %1
  %4 = call %"ClosedRangeIterator<int>" @_EN3std11ClosedRangeI3intE8iteratorE(%"ClosedRange<int>" %3)
  store %"ClosedRangeIterator<int>" %4, %"ClosedRangeIterator<int>"* %__iterator4
  br label %while

while:                                            ; preds = %loop.increment, %0
  %__iterator41 = load %"ClosedRangeIterator<int>", %"ClosedRangeIterator<int>"* %__iterator4
  %5 = call i1 @_EN3std19ClosedRangeIteratorI3intE8hasValueE(%"ClosedRangeIterator<int>" %__iterator41)
  br i1 %5, label %body, label %endwhile

body:                                             ; preds = %while
  %__iterator42 = load %"ClosedRangeIterator<int>", %"ClosedRangeIterator<int>"* %__iterator4
  %6 = call i32 @_EN3std19ClosedRangeIteratorI3intE5valueE(%"ClosedRangeIterator<int>" %__iterator42)
  store i32 %6, i32* %i
  br label %loop.increment

loop.increment:                                   ; preds = %body
  call void @_ENM3std19ClosedRangeIteratorI3intE9incrementE(%"ClosedRangeIterator<int>"* %__iterator4)
  br label %while

endwhile:                                         ; preds = %while
  call void @_ENM3std11ClosedRangeI3intE4initE5start3int3end3int(%"ClosedRange<int>"* %2, i32 0, i32 5)
  %7 = load %"ClosedRange<int>", %"ClosedRange<int>"* %2
  %8 = call %"ClosedRangeIterator<int>" @_EN3std11ClosedRangeI3intE8iteratorE(%"ClosedRange<int>" %7)
  store %"ClosedRangeIterator<int>" %8, %"ClosedRangeIterator<int>"* %__iterator5
  br label %while3

while3:                                           ; preds = %loop.increment5, %endwhile
  %__iterator57 = load %"ClosedRangeIterator<int>", %"ClosedRangeIterator<int>"* %__iterator5
  %9 = call i1 @_EN3std19ClosedRangeIteratorI3intE8hasValueE(%"ClosedRangeIterator<int>" %__iterator57)
  br i1 %9, label %body4, label %endwhile6

body4:                                            ; preds = %while3
  %__iterator59 = load %"ClosedRangeIterator<int>", %"ClosedRangeIterator<int>"* %__iterator5
  %10 = call i32 @_EN3std19ClosedRangeIteratorI3intE5valueE(%"ClosedRangeIterator<int>" %__iterator59)
  store i32 %10, i32* %i8
  br label %loop.increment5

loop.increment5:                                  ; preds = %body4
  call void @_ENM3std19ClosedRangeIteratorI3intE9incrementE(%"ClosedRangeIterator<int>"* %__iterator5)
  br label %while3

endwhile6:                                        ; preds = %while3
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
  %2 = load %"ClosedRangeIterator<int>", %"ClosedRangeIterator<int>"* %1
  ret %"ClosedRangeIterator<int>" %2
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
  %1 = load i32, i32* %current
  %2 = add i32 %1, 1
  store i32 %2, i32* %current
  ret void
}
