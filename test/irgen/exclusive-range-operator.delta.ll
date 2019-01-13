
%"RangeIterator<int>" = type { i32, i32 }
%"Range<int>" = type { i32, i32 }

define i32 @main() {
  %p = alloca i32
  %__iterator6 = alloca %"RangeIterator<int>"
  %1 = alloca %"Range<int>"
  %i = alloca i32
  store i32 9, i32* %p
  call void @_ENM3std5RangeI3intE4initE5start3int3end3int(%"Range<int>"* %1, i32 0, i32 3)
  %2 = load %"Range<int>", %"Range<int>"* %1
  %3 = call %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(%"Range<int>" %2)
  store %"RangeIterator<int>" %3, %"RangeIterator<int>"* %__iterator6
  br label %while

while:                                            ; preds = %body, %0
  %__iterator61 = load %"RangeIterator<int>", %"RangeIterator<int>"* %__iterator6
  %4 = call i1 @_EN3std13RangeIteratorI3intE8hasValueE(%"RangeIterator<int>" %__iterator61)
  br i1 %4, label %body, label %endwhile

body:                                             ; preds = %while
  %__iterator62 = load %"RangeIterator<int>", %"RangeIterator<int>"* %__iterator6
  %5 = call i32 @_EN3std13RangeIteratorI3intE5valueE(%"RangeIterator<int>" %__iterator62)
  store i32 %5, i32* %i
  %6 = load i32, i32* %p
  %i3 = load i32, i32* %i
  %7 = sub i32 %6, %i3
  store i32 %7, i32* %p
  call void @_ENM3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"* %__iterator6)
  br label %while

endwhile:                                         ; preds = %while
  %p4 = load i32, i32* %p
  ret i32 %p4
}

declare %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(%"Range<int>")

declare void @_ENM3std5RangeI3intE4initE5start3int3end3int(%"Range<int>"*, i32, i32)

declare i1 @_EN3std13RangeIteratorI3intE8hasValueE(%"RangeIterator<int>")

declare i32 @_EN3std13RangeIteratorI3intE5valueE(%"RangeIterator<int>")

declare void @_ENM3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"*)
