
%"ClosedRangeIterator<int>" = type { i32, i32 }
%"ClosedRange<int>" = type { i32, i32 }

define i32 @main() {
  %__iterator = alloca %"ClosedRangeIterator<int>", align 8
  %1 = alloca %"ClosedRange<int>", align 8
  %i = alloca i32, align 4
  %__iterator1 = alloca %"ClosedRangeIterator<int>", align 8
  %2 = alloca %"ClosedRange<int>", align 8
  %i2 = alloca i32, align 4
  call void @_EN3std11ClosedRangeI3intE4initE3int3int(ptr %1, i32 0, i32 5)
  %3 = call %"ClosedRangeIterator<int>" @_EN3std11ClosedRangeI3intE8iteratorE(ptr %1)
  store %"ClosedRangeIterator<int>" %3, ptr %__iterator, align 4
  br label %loop.condition

loop.condition:                                   ; preds = %loop.increment, %0
  %4 = call i1 @_EN3std19ClosedRangeIteratorI3intE8hasValueE(ptr %__iterator)
  br i1 %4, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %5 = call i32 @_EN3std19ClosedRangeIteratorI3intE5valueE(ptr %__iterator)
  store i32 %5, ptr %i, align 4
  br label %loop.increment

loop.increment:                                   ; preds = %loop.body
  call void @_EN3std19ClosedRangeIteratorI3intE9incrementE(ptr %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  call void @_EN3std11ClosedRangeI3intE4initE3int3int(ptr %2, i32 0, i32 5)
  %6 = call %"ClosedRangeIterator<int>" @_EN3std11ClosedRangeI3intE8iteratorE(ptr %2)
  store %"ClosedRangeIterator<int>" %6, ptr %__iterator1, align 4
  br label %loop.condition3

loop.condition3:                                  ; preds = %loop.increment5, %loop.end
  %7 = call i1 @_EN3std19ClosedRangeIteratorI3intE8hasValueE(ptr %__iterator1)
  br i1 %7, label %loop.body4, label %loop.end6

loop.body4:                                       ; preds = %loop.condition3
  %8 = call i32 @_EN3std19ClosedRangeIteratorI3intE5valueE(ptr %__iterator1)
  store i32 %8, ptr %i2, align 4
  br label %loop.increment5

loop.increment5:                                  ; preds = %loop.body4
  call void @_EN3std19ClosedRangeIteratorI3intE9incrementE(ptr %__iterator1)
  br label %loop.condition3

loop.end6:                                        ; preds = %loop.condition3
  ret i32 0
}

define void @_EN3std11ClosedRangeI3intE4initE3int3int(ptr %this, i32 %start, i32 %end) {
  %start1 = alloca i32, align 4
  %end2 = alloca i32, align 4
  store i32 %start, ptr %start1, align 4
  store i32 %end, ptr %end2, align 4
  %start3 = getelementptr inbounds %"ClosedRange<int>", ptr %this, i32 0, i32 0
  %start.load = load i32, ptr %start1, align 4
  store i32 %start.load, ptr %start3, align 4
  %end4 = getelementptr inbounds %"ClosedRange<int>", ptr %this, i32 0, i32 1
  %end.load = load i32, ptr %end2, align 4
  store i32 %end.load, ptr %end4, align 4
  ret void
}

define %"ClosedRangeIterator<int>" @_EN3std11ClosedRangeI3intE8iteratorE(ptr %this) {
  %1 = alloca %"ClosedRangeIterator<int>", align 8
  %this.load = load %"ClosedRange<int>", ptr %this, align 4
  call void @_EN3std19ClosedRangeIteratorI3intE4initE11ClosedRangeI3intE(ptr %1, %"ClosedRange<int>" %this.load)
  %.load = load %"ClosedRangeIterator<int>", ptr %1, align 4
  ret %"ClosedRangeIterator<int>" %.load
}

define i1 @_EN3std19ClosedRangeIteratorI3intE8hasValueE(ptr %this) {
  %current = getelementptr inbounds %"ClosedRangeIterator<int>", ptr %this, i32 0, i32 0
  %current.load = load i32, ptr %current, align 4
  %end = getelementptr inbounds %"ClosedRangeIterator<int>", ptr %this, i32 0, i32 1
  %end.load = load i32, ptr %end, align 4
  %1 = icmp sle i32 %current.load, %end.load
  ret i1 %1
}

define i32 @_EN3std19ClosedRangeIteratorI3intE5valueE(ptr %this) {
  %current = getelementptr inbounds %"ClosedRangeIterator<int>", ptr %this, i32 0, i32 0
  %current.load = load i32, ptr %current, align 4
  ret i32 %current.load
}

define void @_EN3std19ClosedRangeIteratorI3intE9incrementE(ptr %this) {
  %current = getelementptr inbounds %"ClosedRangeIterator<int>", ptr %this, i32 0, i32 0
  %current.load = load i32, ptr %current, align 4
  %1 = add i32 %current.load, 1
  store i32 %1, ptr %current, align 4
  ret void
}

define void @_EN3std19ClosedRangeIteratorI3intE4initE11ClosedRangeI3intE(ptr %this, %"ClosedRange<int>" %range) {
  %range1 = alloca %"ClosedRange<int>", align 8
  store %"ClosedRange<int>" %range, ptr %range1, align 4
  %current = getelementptr inbounds %"ClosedRangeIterator<int>", ptr %this, i32 0, i32 0
  %1 = call i32 @_EN3std11ClosedRangeI3intE5startE(ptr %range1)
  store i32 %1, ptr %current, align 4
  %end = getelementptr inbounds %"ClosedRangeIterator<int>", ptr %this, i32 0, i32 1
  %2 = call i32 @_EN3std11ClosedRangeI3intE3endE(ptr %range1)
  store i32 %2, ptr %end, align 4
  ret void
}

define i32 @_EN3std11ClosedRangeI3intE5startE(ptr %this) {
  %start = getelementptr inbounds %"ClosedRange<int>", ptr %this, i32 0, i32 0
  %start.load = load i32, ptr %start, align 4
  ret i32 %start.load
}

define i32 @_EN3std11ClosedRangeI3intE3endE(ptr %this) {
  %end = getelementptr inbounds %"ClosedRange<int>", ptr %this, i32 0, i32 1
  %end.load = load i32, ptr %end, align 4
  ret i32 %end.load
}
