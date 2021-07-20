
%"ArrayIterator<int>" = type { i32*, i32* }

define i32 @main() {
  %__iterator = alloca %"ArrayIterator<int>", align 8
  %1 = alloca [3 x i32], align 4
  %e = alloca i32*, align 8
  %a = alloca [2 x i32], align 4
  %__iterator1 = alloca %"ArrayIterator<int>", align 8
  %e2 = alloca i32*, align 8
  store [3 x i32] [i32 1, i32 2, i32 3], [3 x i32]* %1, align 4
  %2 = getelementptr inbounds [3 x i32], [3 x i32]* %1, i32 0, i32 0
  %3 = getelementptr inbounds i32, i32* %2, i32 3
  %4 = insertvalue %"ArrayIterator<int>" undef, i32* %2, 0
  %5 = insertvalue %"ArrayIterator<int>" %4, i32* %3, 1
  store %"ArrayIterator<int>" %5, %"ArrayIterator<int>"* %__iterator, align 8
  br label %loop.condition

loop.condition:                                   ; preds = %loop.increment, %0
  %6 = call i1 @_EN3std13ArrayIteratorI3intE8hasValueE(%"ArrayIterator<int>"* %__iterator)
  br i1 %6, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %7 = call i32* @_EN3std13ArrayIteratorI3intE5valueE(%"ArrayIterator<int>"* %__iterator)
  store i32* %7, i32** %e, align 8
  %e.load = load i32*, i32** %e, align 8
  call void @_EN3std7printlnI3intEEP3int(i32* %e.load)
  br label %loop.increment

loop.increment:                                   ; preds = %loop.body
  call void @_EN3std13ArrayIteratorI3intE9incrementE(%"ArrayIterator<int>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  store [2 x i32] [i32 4, i32 5], [2 x i32]* %a, align 4
  %8 = getelementptr inbounds [2 x i32], [2 x i32]* %a, i32 0, i32 0
  %9 = getelementptr inbounds i32, i32* %8, i32 2
  %10 = insertvalue %"ArrayIterator<int>" undef, i32* %8, 0
  %11 = insertvalue %"ArrayIterator<int>" %10, i32* %9, 1
  store %"ArrayIterator<int>" %11, %"ArrayIterator<int>"* %__iterator1, align 8
  br label %loop.condition3

loop.condition3:                                  ; preds = %loop.increment6, %loop.end
  %12 = call i1 @_EN3std13ArrayIteratorI3intE8hasValueE(%"ArrayIterator<int>"* %__iterator1)
  br i1 %12, label %loop.body4, label %loop.end7

loop.body4:                                       ; preds = %loop.condition3
  %13 = call i32* @_EN3std13ArrayIteratorI3intE5valueE(%"ArrayIterator<int>"* %__iterator1)
  store i32* %13, i32** %e2, align 8
  %e.load5 = load i32*, i32** %e2, align 8
  call void @_EN3std7printlnI3intEEP3int(i32* %e.load5)
  br label %loop.increment6

loop.increment6:                                  ; preds = %loop.body4
  call void @_EN3std13ArrayIteratorI3intE9incrementE(%"ArrayIterator<int>"* %__iterator1)
  br label %loop.condition3

loop.end7:                                        ; preds = %loop.condition3
  ret i32 0
}

define i1 @_EN3std13ArrayIteratorI3intE8hasValueE(%"ArrayIterator<int>"* %this) {
  %current = getelementptr inbounds %"ArrayIterator<int>", %"ArrayIterator<int>"* %this, i32 0, i32 0
  %current.load = load i32*, i32** %current, align 8
  %end = getelementptr inbounds %"ArrayIterator<int>", %"ArrayIterator<int>"* %this, i32 0, i32 1
  %end.load = load i32*, i32** %end, align 8
  %1 = icmp ne i32* %current.load, %end.load
  ret i1 %1
}

define i32* @_EN3std13ArrayIteratorI3intE5valueE(%"ArrayIterator<int>"* %this) {
  %current = getelementptr inbounds %"ArrayIterator<int>", %"ArrayIterator<int>"* %this, i32 0, i32 0
  %current.load = load i32*, i32** %current, align 8
  ret i32* %current.load
}

define void @_EN3std7printlnI3intEEP3int(i32* %value) {
  call void @_EN3std5printI3intEEP3int(i32* %value)
  call void @_EN3std5printI4charEE4char(i8 10)
  ret void
}

define void @_EN3std13ArrayIteratorI3intE9incrementE(%"ArrayIterator<int>"* %this) {
  %current = getelementptr inbounds %"ArrayIterator<int>", %"ArrayIterator<int>"* %this, i32 0, i32 0
  %current.load = load i32*, i32** %current, align 8
  %1 = getelementptr inbounds i32, i32* %current.load, i32 1
  store i32* %1, i32** %current, align 8
  ret void
}

declare void @_EN3std5printI3intEEP3int(i32*)

declare void @_EN3std5printI4charEE4char(i8)
