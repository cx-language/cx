
%"ArrayRef<int>" = type { i32*, i32 }
%"Array<int>" = type { i32*, i32, i32 }
%"RangeIterator<uint>" = type { i32, i32 }
%"Range<uint>" = type { i32, i32 }
%"ArrayIterator<int>" = type { i32*, i32* }
%"MutableArrayIterator<int>" = type { i32*, i32* }
%"EnumeratedIterator<int>" = type { %"ArrayIterator<int>", i32 }
%"EnumeratedIteratorEntry<int>" = type { i32*, i32 }

@0 = private unnamed_addr constant [45 x i8] c"Array index %u is out of bounds, size is %u\0A\00"
@1 = private unnamed_addr constant [31 x i8] c"Called first() on empty Array\0A\00"
@2 = private unnamed_addr constant [30 x i8] c"Called last() on empty Array\0A\00"
@3 = private unnamed_addr constant [37 x i8] c"Called removeFirst() on empty Array\0A\00"
@4 = private unnamed_addr constant [36 x i8] c"Called removeLast() on empty Array\0A\00"
@5 = private unnamed_addr constant [71 x i8] c"Assertion failed at /Users/emlai/src/delta/stdlib/allocate.delta:36:67\00"

define i32 @main() {
  %__temporaryArray0 = alloca [3 x i32]
  %__temporaryArray1 = alloca [3 x i32]
  %__temporaryArray2 = alloca [3 x i32]
  store [3 x i32] [i32 1, i32 2, i32 3], [3 x i32]* %__temporaryArray0
  %1 = getelementptr [3 x i32], [3 x i32]* %__temporaryArray0, i32 0, i32 0
  %2 = insertvalue %"ArrayRef<int>" undef, i32* %1, 0
  %3 = insertvalue %"ArrayRef<int>" %2, i32 3, 1
  call void @_EN4main1fI3intEE1aPAR_3int(%"ArrayRef<int>" %3)
  store [3 x i32] [i32 1, i32 2, i32 3], [3 x i32]* %__temporaryArray1
  %4 = getelementptr [3 x i32], [3 x i32]* %__temporaryArray1, i32 0, i32 0
  %5 = insertvalue %"ArrayRef<int>" undef, i32* %4, 0
  %6 = insertvalue %"ArrayRef<int>" %5, i32 3, 1
  call void @_EN4main1fI3intEE1aPAR_3int(%"ArrayRef<int>" %6)
  store [3 x i32] [i32 1, i32 2, i32 3], [3 x i32]* %__temporaryArray2
  %7 = getelementptr [3 x i32], [3 x i32]* %__temporaryArray2, i32 0, i32 0
  %8 = insertvalue %"ArrayRef<int>" undef, i32* %7, 0
  %9 = insertvalue %"ArrayRef<int>" %8, i32 3, 1
  call void @_EN4main1fI3intEE1aPAR_3int(%"ArrayRef<int>" %9)
  ret i32 0
}

define void @_ENM3std8ArrayRefI3intE4initE(%"ArrayRef<int>"* %this) {
  %size = getelementptr inbounds %"ArrayRef<int>", %"ArrayRef<int>"* %this, i32 0, i32 1
  store i32 0, i32* %size
  ret void
}

define void @_ENM3std5ArrayI3intE4initE(%"Array<int>"* %this) {
  %size = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 1
  store i32 0, i32* %size
  %capacity = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 2
  store i32 0, i32* %capacity
  ret void
}

define void @_ENM3std5ArrayI3intE4initE8capacity4uint(%"Array<int>"* %this, i32 %capacity) {
  call void @_ENM3std5ArrayI3intE4initE(%"Array<int>"* %this)
  call void @_ENM3std5ArrayI3intE7reserveE15minimumCapacity4uint(%"Array<int>"* %this, i32 %capacity)
  ret void
}

define void @_ENM3std5ArrayI3intE7reserveE15minimumCapacity4uint(%"Array<int>"* %this, i32 %minimumCapacity) {
  %newBufferPointer = alloca i32*
  %__iterator19 = alloca %"RangeIterator<uint>"
  %1 = alloca %"Range<uint>"
  %index = alloca i32
  %source = alloca i32*
  %target = alloca i32*
  %capacity = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 2
  %capacity1 = load i32, i32* %capacity
  %2 = icmp ugt i32 %minimumCapacity, %capacity1
  br i1 %2, label %then, label %else

then:                                             ; preds = %0
  %3 = call i32* @_EN3std13allocateArrayI3intEE4size4uint(i32 %minimumCapacity)
  store i32* %3, i32** %newBufferPointer
  %size = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 1
  %size2 = load i32, i32* %size
  call void @_ENM3std5RangeI4uintE4initE5start4uint3end4uint(%"Range<uint>"* %1, i32 0, i32 %size2)
  %4 = load %"Range<uint>", %"Range<uint>"* %1
  %5 = call %"RangeIterator<uint>" @_EN3std5RangeI4uintE8iteratorE(%"Range<uint>" %4)
  store %"RangeIterator<uint>" %5, %"RangeIterator<uint>"* %__iterator19
  br label %while

else:                                             ; preds = %0
  br label %endif

endif:                                            ; preds = %else, %endif15
  ret void

while:                                            ; preds = %body, %then
  %__iterator193 = load %"RangeIterator<uint>", %"RangeIterator<uint>"* %__iterator19
  %6 = call i1 @_EN3std13RangeIteratorI4uintE8hasValueE(%"RangeIterator<uint>" %__iterator193)
  br i1 %6, label %body, label %endwhile

body:                                             ; preds = %while
  %__iterator194 = load %"RangeIterator<uint>", %"RangeIterator<uint>"* %__iterator19
  %7 = call i32 @_EN3std13RangeIteratorI4uintE5valueE(%"RangeIterator<uint>" %__iterator194)
  store i32 %7, i32* %index
  %bufferPointer = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 0
  %bufferPointer5 = load i32*, i32** %bufferPointer
  %index6 = load i32, i32* %index
  %8 = getelementptr i32, i32* %bufferPointer5, i32 %index6
  store i32* %8, i32** %source
  %newBufferPointer7 = load i32*, i32** %newBufferPointer
  %index8 = load i32, i32* %index
  %9 = getelementptr i32, i32* %newBufferPointer7, i32 %index8
  store i32* %9, i32** %target
  %target9 = load i32*, i32** %target
  %source10 = load i32*, i32** %source
  %10 = load i32, i32* %source10
  store i32 %10, i32* %target9
  call void @_ENM3std13RangeIteratorI4uintE9incrementE(%"RangeIterator<uint>"* %__iterator19)
  br label %while

endwhile:                                         ; preds = %while
  %capacity11 = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 2
  %capacity12 = load i32, i32* %capacity11
  %11 = icmp ne i32 %capacity12, 0
  br i1 %11, label %then13, label %else14

then13:                                           ; preds = %endwhile
  %bufferPointer16 = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 0
  %bufferPointer17 = load i32*, i32** %bufferPointer16
  call void @_EN3std10deallocateI3intEE10allocationPM3int(i32* %bufferPointer17)
  br label %endif15

else14:                                           ; preds = %endwhile
  br label %endif15

endif15:                                          ; preds = %else14, %then13
  %bufferPointer18 = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 0
  %newBufferPointer19 = load i32*, i32** %newBufferPointer
  store i32* %newBufferPointer19, i32** %bufferPointer18
  %capacity20 = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 2
  store i32 %minimumCapacity, i32* %capacity20
  br label %endif
}

define void @_ENM3std5ArrayI3intE4initE8elementsPAR_3int(%"Array<int>"* %this, %"ArrayRef<int>" %elements) {
  %__iterator18 = alloca %"RangeIterator<uint>"
  %1 = alloca %"Range<uint>"
  %index = alloca i32
  %size = extractvalue %"ArrayRef<int>" %elements, 1
  call void @_ENM3std5ArrayI3intE4initE8capacity4uint(%"Array<int>"* %this, i32 %size)
  %size1 = extractvalue %"ArrayRef<int>" %elements, 1
  call void @_ENM3std5RangeI4uintE4initE5start4uint3end4uint(%"Range<uint>"* %1, i32 0, i32 %size1)
  %2 = load %"Range<uint>", %"Range<uint>"* %1
  %3 = call %"RangeIterator<uint>" @_EN3std5RangeI4uintE8iteratorE(%"Range<uint>" %2)
  store %"RangeIterator<uint>" %3, %"RangeIterator<uint>"* %__iterator18
  br label %while

while:                                            ; preds = %body, %0
  %__iterator182 = load %"RangeIterator<uint>", %"RangeIterator<uint>"* %__iterator18
  %4 = call i1 @_EN3std13RangeIteratorI4uintE8hasValueE(%"RangeIterator<uint>" %__iterator182)
  br i1 %4, label %body, label %endwhile

body:                                             ; preds = %while
  %__iterator183 = load %"RangeIterator<uint>", %"RangeIterator<uint>"* %__iterator18
  %5 = call i32 @_EN3std13RangeIteratorI4uintE5valueE(%"RangeIterator<uint>" %__iterator183)
  store i32 %5, i32* %index
  %6 = extractvalue %"ArrayRef<int>" %elements, 0
  %index4 = load i32, i32* %index
  %7 = getelementptr i32, i32* %6, i32 %index4
  %8 = load i32, i32* %7
  call void @_ENM3std5ArrayI3intE4pushE10newElement3int(%"Array<int>"* %this, i32 %8)
  call void @_ENM3std13RangeIteratorI4uintE9incrementE(%"RangeIterator<uint>"* %__iterator18)
  br label %while

endwhile:                                         ; preds = %while
  ret void
}

declare %"RangeIterator<uint>" @_EN3std5RangeI4uintE8iteratorE(%"Range<uint>")

declare void @_ENM3std5RangeI4uintE4initE5start4uint3end4uint(%"Range<uint>"*, i32, i32)

declare i1 @_EN3std13RangeIteratorI4uintE8hasValueE(%"RangeIterator<uint>")

declare i32 @_EN3std13RangeIteratorI4uintE5valueE(%"RangeIterator<uint>")

define void @_ENM3std5ArrayI3intE4pushE10newElement3int(%"Array<int>"* %this, i32 %newElement) {
  %size = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 1
  %size1 = load i32, i32* %size
  %capacity = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 2
  %capacity2 = load i32, i32* %capacity
  %1 = icmp eq i32 %size1, %capacity2
  br i1 %1, label %then, label %else

then:                                             ; preds = %0
  call void @_ENM3std5ArrayI3intE4growE(%"Array<int>"* %this)
  br label %endif

else:                                             ; preds = %0
  br label %endif

endif:                                            ; preds = %else, %then
  %bufferPointer = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 0
  %bufferPointer3 = load i32*, i32** %bufferPointer
  %size4 = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 1
  %size5 = load i32, i32* %size4
  %2 = getelementptr i32, i32* %bufferPointer3, i32 %size5
  store i32 %newElement, i32* %2
  %size6 = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 1
  %3 = load i32, i32* %size6
  %4 = add i32 %3, 1
  store i32 %4, i32* %size6
  ret void
}

declare void @_ENM3std13RangeIteratorI4uintE9incrementE(%"RangeIterator<uint>"*)

define void @_ENM3std5ArrayI3intE6deinitE(%"Array<int>"* %this) {
  %capacity = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 2
  %capacity1 = load i32, i32* %capacity
  %1 = icmp ne i32 %capacity1, 0
  br i1 %1, label %then, label %else

then:                                             ; preds = %0
  %bufferPointer = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 0
  %bufferPointer2 = load i32*, i32** %bufferPointer
  call void @_EN3std10deallocateI3intEE10allocationPM3int(i32* %bufferPointer2)
  br label %endif

else:                                             ; preds = %0
  br label %endif

endif:                                            ; preds = %else, %then
  ret void
}

define void @_EN3std10deallocateI3intEE10allocationPM3int(i32* %allocation) {
  %1 = bitcast i32* %allocation to i8*
  call void @free(i8* %1)
  ret void
}

define i32 @_EN3std5ArrayI3intE4sizeE(%"Array<int>"* %this) {
  %size = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 1
  %size1 = load i32, i32* %size
  ret i32 %size1
}

define i1 @_EN3std5ArrayI3intE7isEmptyE(%"Array<int>"* %this) {
  %size = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 1
  %size1 = load i32, i32* %size
  %1 = icmp eq i32 %size1, 0
  ret i1 %1
}

define i32 @_EN3std5ArrayI3intE8capacityE(%"Array<int>"* %this) {
  %capacity = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 2
  %capacity1 = load i32, i32* %capacity
  ret i32 %capacity1
}

define i32* @_EN3std5ArrayI3intEixE5index4uint(%"Array<int>"* %this, i32 %index) {
  %size = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 1
  %size1 = load i32, i32* %size
  %1 = icmp uge i32 %index, %size1
  br i1 %1, label %then, label %else

then:                                             ; preds = %0
  call void @_EN3std5ArrayI3intE16indexOutOfBoundsE5index4uint(%"Array<int>"* %this, i32 %index)
  br label %endif

else:                                             ; preds = %0
  br label %endif

endif:                                            ; preds = %else, %then
  %bufferPointer = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 0
  %bufferPointer2 = load i32*, i32** %bufferPointer
  %2 = getelementptr i32, i32* %bufferPointer2, i32 %index
  ret i32* %2
}

define void @_EN3std5ArrayI3intE16indexOutOfBoundsE5index4uint(%"Array<int>"* %this, i32 %index) {
  %1 = call i32 @_EN3std5ArrayI3intE4sizeE(%"Array<int>"* %this)
  %2 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([45 x i8], [45 x i8]* @0, i32 0, i32 0), i32 %index, i32 %1)
  call void @_EN3std10fatalErrorE()
  ret void
}

define i32* @_ENM3std5ArrayI3intEixE5index4uint(%"Array<int>"* %this, i32 %index) {
  %size = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 1
  %size1 = load i32, i32* %size
  %1 = icmp uge i32 %index, %size1
  br i1 %1, label %then, label %else

then:                                             ; preds = %0
  call void @_EN3std5ArrayI3intE16indexOutOfBoundsE5index4uint(%"Array<int>"* %this, i32 %index)
  br label %endif

else:                                             ; preds = %0
  br label %endif

endif:                                            ; preds = %else, %then
  %bufferPointer = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 0
  %bufferPointer2 = load i32*, i32** %bufferPointer
  %2 = getelementptr i32, i32* %bufferPointer2, i32 %index
  ret i32* %2
}

define i32* @_EN3std5ArrayI3intE5firstE(%"Array<int>"* %this) {
  %size = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 1
  %size1 = load i32, i32* %size
  %1 = icmp eq i32 %size1, 0
  br i1 %1, label %then, label %else

then:                                             ; preds = %0
  call void @_EN3std5ArrayI3intE15emptyArrayFirstE(%"Array<int>"* %this)
  br label %endif

else:                                             ; preds = %0
  br label %endif

endif:                                            ; preds = %else, %then
  %bufferPointer = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 0
  %bufferPointer2 = load i32*, i32** %bufferPointer
  ret i32* %bufferPointer2
}

define void @_EN3std5ArrayI3intE15emptyArrayFirstE(%"Array<int>"* %this) {
  %1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([31 x i8], [31 x i8]* @1, i32 0, i32 0))
  call void @_EN3std10fatalErrorE()
  ret void
}

define i32* @_EN3std5ArrayI3intE4lastE(%"Array<int>"* %this) {
  %size = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 1
  %size1 = load i32, i32* %size
  %1 = icmp eq i32 %size1, 0
  br i1 %1, label %then, label %else

then:                                             ; preds = %0
  call void @_EN3std5ArrayI3intE14emptyArrayLastE(%"Array<int>"* %this)
  br label %endif

else:                                             ; preds = %0
  br label %endif

endif:                                            ; preds = %else, %then
  %bufferPointer = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 0
  %bufferPointer2 = load i32*, i32** %bufferPointer
  %size3 = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 1
  %size4 = load i32, i32* %size3
  %2 = getelementptr i32, i32* %bufferPointer2, i32 %size4
  ret i32* %2
}

define void @_EN3std5ArrayI3intE14emptyArrayLastE(%"Array<int>"* %this) {
  %1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([30 x i8], [30 x i8]* @2, i32 0, i32 0))
  call void @_EN3std10fatalErrorE()
  ret void
}

define i32* @_EN3std5ArrayI3intE4dataE(%"Array<int>"* %this) {
  %bufferPointer = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 0
  %bufferPointer1 = load i32*, i32** %bufferPointer
  ret i32* %bufferPointer1
}

define i32* @_ENM3std5ArrayI3intE4dataE(%"Array<int>"* %this) {
  %bufferPointer = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 0
  %bufferPointer1 = load i32*, i32** %bufferPointer
  ret i32* %bufferPointer1
}

define void @_ENM3std5ArrayI3intE4growE(%"Array<int>"* %this) {
  %capacity = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 2
  %capacity1 = load i32, i32* %capacity
  %1 = icmp eq i32 %capacity1, 0
  br i1 %1, label %then, label %else

then:                                             ; preds = %0
  call void @_ENM3std5ArrayI3intE7reserveE15minimumCapacity4uint(%"Array<int>"* %this, i32 1)
  br label %endif

else:                                             ; preds = %0
  %capacity2 = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 2
  %capacity3 = load i32, i32* %capacity2
  %2 = mul i32 %capacity3, 2
  call void @_ENM3std5ArrayI3intE7reserveE15minimumCapacity4uint(%"Array<int>"* %this, i32 %2)
  br label %endif

endif:                                            ; preds = %else, %then
  ret void
}

define i32* @_EN3std13allocateArrayI3intEE4size4uint(i32 %size) {
  %1 = zext i32 %size to i64
  %2 = mul i64 ptrtoint (i32* getelementptr (i32, i32* null, i32 1) to i64), %1
  %3 = call i8* @malloc(i64 %2)
  %assert.condition = icmp eq i8* %3, null
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %0
  %4 = call i32 @puts(i8* getelementptr inbounds ([71 x i8], [71 x i8]* @5, i32 0, i32 0))
  call void @llvm.trap()
  unreachable

assert.success:                                   ; preds = %0
  %5 = bitcast i8* %3 to i32*
  ret i32* %5
}

define void @_ENM3std5ArrayI3intE11removeFirstE(%"Array<int>"* %this) {
  %size = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 1
  %size1 = load i32, i32* %size
  %1 = icmp eq i32 %size1, 0
  br i1 %1, label %then, label %else

then:                                             ; preds = %0
  call void @_EN3std5ArrayI3intE21emptyArrayRemoveFirstE(%"Array<int>"* %this)
  br label %endif

else:                                             ; preds = %0
  br label %endif

endif:                                            ; preds = %else, %then
  call void @_ENM3std5ArrayI3intE14unsafeRemoveAtE5index4uint(%"Array<int>"* %this, i32 0)
  ret void
}

define void @_EN3std5ArrayI3intE21emptyArrayRemoveFirstE(%"Array<int>"* %this) {
  %1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([37 x i8], [37 x i8]* @3, i32 0, i32 0))
  call void @_EN3std10fatalErrorE()
  ret void
}

define void @_ENM3std5ArrayI3intE14unsafeRemoveAtE5index4uint(%"Array<int>"* %this, i32 %index) {
  %__iterator20 = alloca %"RangeIterator<uint>"
  %1 = alloca %"Range<uint>"
  %i = alloca i32
  %source = alloca i32*
  %target = alloca i32*
  %2 = add i32 %index, 1
  %size = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 1
  %size1 = load i32, i32* %size
  call void @_ENM3std5RangeI4uintE4initE5start4uint3end4uint(%"Range<uint>"* %1, i32 %2, i32 %size1)
  %3 = load %"Range<uint>", %"Range<uint>"* %1
  %4 = call %"RangeIterator<uint>" @_EN3std5RangeI4uintE8iteratorE(%"Range<uint>" %3)
  store %"RangeIterator<uint>" %4, %"RangeIterator<uint>"* %__iterator20
  br label %while

while:                                            ; preds = %body, %0
  %__iterator202 = load %"RangeIterator<uint>", %"RangeIterator<uint>"* %__iterator20
  %5 = call i1 @_EN3std13RangeIteratorI4uintE8hasValueE(%"RangeIterator<uint>" %__iterator202)
  br i1 %5, label %body, label %endwhile

body:                                             ; preds = %while
  %__iterator203 = load %"RangeIterator<uint>", %"RangeIterator<uint>"* %__iterator20
  %6 = call i32 @_EN3std13RangeIteratorI4uintE5valueE(%"RangeIterator<uint>" %__iterator203)
  store i32 %6, i32* %i
  %bufferPointer = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 0
  %bufferPointer4 = load i32*, i32** %bufferPointer
  %i5 = load i32, i32* %i
  %7 = getelementptr i32, i32* %bufferPointer4, i32 %i5
  store i32* %7, i32** %source
  %bufferPointer6 = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 0
  %bufferPointer7 = load i32*, i32** %bufferPointer6
  %i8 = load i32, i32* %i
  %8 = sub i32 %i8, 1
  %9 = getelementptr i32, i32* %bufferPointer7, i32 %8
  store i32* %9, i32** %target
  %target9 = load i32*, i32** %target
  %source10 = load i32*, i32** %source
  %10 = load i32, i32* %source10
  store i32 %10, i32* %target9
  call void @_ENM3std13RangeIteratorI4uintE9incrementE(%"RangeIterator<uint>"* %__iterator20)
  br label %while

endwhile:                                         ; preds = %while
  %size11 = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 1
  %11 = load i32, i32* %size11
  %12 = sub i32 %11, 1
  store i32 %12, i32* %size11
  ret void
}

define void @_ENM3std5ArrayI3intE10removeLastE(%"Array<int>"* %this) {
  %size = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 1
  %size1 = load i32, i32* %size
  %1 = icmp eq i32 %size1, 0
  br i1 %1, label %then, label %else

then:                                             ; preds = %0
  call void @_EN3std5ArrayI3intE20emptyArrayRemoveLastE(%"Array<int>"* %this)
  br label %endif

else:                                             ; preds = %0
  br label %endif

endif:                                            ; preds = %else, %then
  %size2 = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 1
  %2 = load i32, i32* %size2
  %3 = sub i32 %2, 1
  store i32 %3, i32* %size2
  ret void
}

define void @_EN3std5ArrayI3intE20emptyArrayRemoveLastE(%"Array<int>"* %this) {
  %1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([36 x i8], [36 x i8]* @4, i32 0, i32 0))
  call void @_EN3std10fatalErrorE()
  ret void
}

define void @_ENM3std5ArrayI3intE8removeAtE5index4uint(%"Array<int>"* %this, i32 %index) {
  %size = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 1
  %size1 = load i32, i32* %size
  %1 = icmp uge i32 %index, %size1
  br i1 %1, label %then, label %else

then:                                             ; preds = %0
  call void @_EN3std5ArrayI3intE16indexOutOfBoundsE5index4uint(%"Array<int>"* %this, i32 %index)
  br label %endif

else:                                             ; preds = %0
  br label %endif

endif:                                            ; preds = %else, %then
  call void @_ENM3std5ArrayI3intE14unsafeRemoveAtE5index4uint(%"Array<int>"* %this, i32 %index)
  ret void
}

define void @_ENM3std13ArrayIteratorI3intE4initE5array8ArrayRefI3intE(%"ArrayIterator<int>"* %this, %"ArrayRef<int>" %array) {
  %current = getelementptr inbounds %"ArrayIterator<int>", %"ArrayIterator<int>"* %this, i32 0, i32 0
  %1 = call i32* @_EN3std8ArrayRefI3intE4dataE(%"ArrayRef<int>" %array)
  store i32* %1, i32** %current
  %end = getelementptr inbounds %"ArrayIterator<int>", %"ArrayIterator<int>"* %this, i32 0, i32 1
  %2 = call i32* @_EN3std8ArrayRefI3intE4dataE(%"ArrayRef<int>" %array)
  %3 = call i32 @_EN3std8ArrayRefI3intE4sizeE(%"ArrayRef<int>" %array)
  %4 = getelementptr i32, i32* %2, i32 %3
  store i32* %4, i32** %end
  ret void
}

define i32* @_EN3std8ArrayRefI3intE4dataE(%"ArrayRef<int>" %this) {
  %data = extractvalue %"ArrayRef<int>" %this, 0
  ret i32* %data
}

define i32 @_EN3std8ArrayRefI3intE4sizeE(%"ArrayRef<int>" %this) {
  %size = extractvalue %"ArrayRef<int>" %this, 1
  ret i32 %size
}

define i1 @_EN3std13ArrayIteratorI3intE8hasValueE(%"ArrayIterator<int>" %this) {
  %current = extractvalue %"ArrayIterator<int>" %this, 0
  %end = extractvalue %"ArrayIterator<int>" %this, 1
  %1 = icmp ne i32* %current, %end
  ret i1 %1
}

define i32* @_EN3std13ArrayIteratorI3intE5valueE(%"ArrayIterator<int>" %this) {
  %current = extractvalue %"ArrayIterator<int>" %this, 0
  ret i32* %current
}

define void @_ENM3std13ArrayIteratorI3intE9incrementE(%"ArrayIterator<int>"* %this) {
  %current = getelementptr inbounds %"ArrayIterator<int>", %"ArrayIterator<int>"* %this, i32 0, i32 0
  %1 = load i32*, i32** %current
  %2 = getelementptr i32, i32* %1, i32 1
  store i32* %2, i32** %current
  ret void
}

define %"ArrayIterator<int>" @_EN3std5ArrayI3intE8iteratorE(%"Array<int>"* %this) {
  %1 = alloca %"ArrayIterator<int>"
  %2 = alloca %"ArrayRef<int>"
  call void @_ENM3std8ArrayRefI3intE4initE5arrayP5ArrayI3intE(%"ArrayRef<int>"* %2, %"Array<int>"* %this)
  %3 = load %"ArrayRef<int>", %"ArrayRef<int>"* %2
  call void @_ENM3std13ArrayIteratorI3intE4initE5array8ArrayRefI3intE(%"ArrayIterator<int>"* %1, %"ArrayRef<int>" %3)
  %4 = load %"ArrayIterator<int>", %"ArrayIterator<int>"* %1
  ret %"ArrayIterator<int>" %4
}

define void @_ENM3std8ArrayRefI3intE4initE5arrayP5ArrayI3intE(%"ArrayRef<int>"* %this, %"Array<int>"* %array) {
  %data = getelementptr inbounds %"ArrayRef<int>", %"ArrayRef<int>"* %this, i32 0, i32 0
  %1 = call i32* @_EN3std5ArrayI3intE4dataE(%"Array<int>"* %array)
  store i32* %1, i32** %data
  %size = getelementptr inbounds %"ArrayRef<int>", %"ArrayRef<int>"* %this, i32 0, i32 1
  %2 = call i32 @_EN3std5ArrayI3intE4sizeE(%"Array<int>"* %array)
  store i32 %2, i32* %size
  ret void
}

define void @_ENM3std20MutableArrayIteratorI3intE4initE5arrayPM5ArrayI3intE(%"MutableArrayIterator<int>"* %this, %"Array<int>"* %array) {
  %current = getelementptr inbounds %"MutableArrayIterator<int>", %"MutableArrayIterator<int>"* %this, i32 0, i32 0
  %1 = call i32* @_ENM3std5ArrayI3intE4dataE(%"Array<int>"* %array)
  store i32* %1, i32** %current
  %end = getelementptr inbounds %"MutableArrayIterator<int>", %"MutableArrayIterator<int>"* %this, i32 0, i32 1
  %2 = call i32* @_ENM3std5ArrayI3intE4dataE(%"Array<int>"* %array)
  %3 = call i32 @_EN3std5ArrayI3intE4sizeE(%"Array<int>"* %array)
  %4 = getelementptr i32, i32* %2, i32 %3
  store i32* %4, i32** %end
  ret void
}

define i1 @_EN3std20MutableArrayIteratorI3intE8hasValueE(%"MutableArrayIterator<int>" %this) {
  %current = extractvalue %"MutableArrayIterator<int>" %this, 0
  %end = extractvalue %"MutableArrayIterator<int>" %this, 1
  %1 = icmp ne i32* %current, %end
  ret i1 %1
}

define i32* @_EN3std20MutableArrayIteratorI3intE5valueE(%"MutableArrayIterator<int>" %this) {
  %current = extractvalue %"MutableArrayIterator<int>" %this, 0
  ret i32* %current
}

define void @_ENM3std20MutableArrayIteratorI3intE9incrementE(%"MutableArrayIterator<int>"* %this) {
  %current = getelementptr inbounds %"MutableArrayIterator<int>", %"MutableArrayIterator<int>"* %this, i32 0, i32 0
  %1 = load i32*, i32** %current
  %2 = getelementptr i32, i32* %1, i32 1
  store i32* %2, i32** %current
  ret void
}

define %"MutableArrayIterator<int>" @_ENM3std5ArrayI3intE8iteratorE(%"Array<int>"* %this) {
  %1 = alloca %"MutableArrayIterator<int>"
  call void @_ENM3std20MutableArrayIteratorI3intE4initE5arrayPM5ArrayI3intE(%"MutableArrayIterator<int>"* %1, %"Array<int>"* %this)
  %2 = load %"MutableArrayIterator<int>", %"MutableArrayIterator<int>"* %1
  ret %"MutableArrayIterator<int>" %2
}

define void @_ENM3std18EnumeratedIteratorI3intE4initE8iterator13ArrayIteratorI3intE(%"EnumeratedIterator<int>"* %this, %"ArrayIterator<int>" %iterator) {
  %base = getelementptr inbounds %"EnumeratedIterator<int>", %"EnumeratedIterator<int>"* %this, i32 0, i32 0
  store %"ArrayIterator<int>" %iterator, %"ArrayIterator<int>"* %base
  %index = getelementptr inbounds %"EnumeratedIterator<int>", %"EnumeratedIterator<int>"* %this, i32 0, i32 1
  store i32 0, i32* %index
  ret void
}

define i1 @_EN3std18EnumeratedIteratorI3intE8hasValueE(%"EnumeratedIterator<int>" %this) {
  %base = extractvalue %"EnumeratedIterator<int>" %this, 0
  %1 = call i1 @_EN3std13ArrayIteratorI3intE8hasValueE(%"ArrayIterator<int>" %base)
  ret i1 %1
}

define void @_ENM3std23EnumeratedIteratorEntryI3intE4initE5valueP3int5index4uint(%"EnumeratedIteratorEntry<int>"* %this, i32* %value, i32 %index) {
  %value1 = getelementptr inbounds %"EnumeratedIteratorEntry<int>", %"EnumeratedIteratorEntry<int>"* %this, i32 0, i32 0
  store i32* %value, i32** %value1
  %index2 = getelementptr inbounds %"EnumeratedIteratorEntry<int>", %"EnumeratedIteratorEntry<int>"* %this, i32 0, i32 1
  store i32 %index, i32* %index2
  ret void
}

define %"EnumeratedIteratorEntry<int>" @_EN3std18EnumeratedIteratorI3intE5valueE(%"EnumeratedIterator<int>" %this) {
  %1 = alloca %"EnumeratedIteratorEntry<int>"
  %base = extractvalue %"EnumeratedIterator<int>" %this, 0
  %2 = call i32* @_EN3std13ArrayIteratorI3intE5valueE(%"ArrayIterator<int>" %base)
  %index = extractvalue %"EnumeratedIterator<int>" %this, 1
  call void @_ENM3std23EnumeratedIteratorEntryI3intE4initE5valueP3int5index4uint(%"EnumeratedIteratorEntry<int>"* %1, i32* %2, i32 %index)
  %3 = load %"EnumeratedIteratorEntry<int>", %"EnumeratedIteratorEntry<int>"* %1
  ret %"EnumeratedIteratorEntry<int>" %3
}

define void @_ENM3std18EnumeratedIteratorI3intE9incrementE(%"EnumeratedIterator<int>"* %this) {
  %base = getelementptr inbounds %"EnumeratedIterator<int>", %"EnumeratedIterator<int>"* %this, i32 0, i32 0
  call void @_ENM3std13ArrayIteratorI3intE9incrementE(%"ArrayIterator<int>"* %base)
  %index = getelementptr inbounds %"EnumeratedIterator<int>", %"EnumeratedIterator<int>"* %this, i32 0, i32 1
  %1 = load i32, i32* %index
  %2 = add i32 %1, 1
  store i32 %2, i32* %index
  ret void
}

define %"EnumeratedIterator<int>" @_EN3std5ArrayI3intE9enumerateE(%"Array<int>"* %this) {
  %1 = alloca %"EnumeratedIterator<int>"
  %2 = call %"ArrayIterator<int>" @_EN3std5ArrayI3intE8iteratorE(%"Array<int>"* %this)
  call void @_ENM3std18EnumeratedIteratorI3intE4initE8iterator13ArrayIteratorI3intE(%"EnumeratedIterator<int>"* %1, %"ArrayIterator<int>" %2)
  %3 = load %"EnumeratedIterator<int>", %"EnumeratedIterator<int>"* %1
  ret %"EnumeratedIterator<int>" %3
}

declare i32 @printf(i8*, ...)

declare void @_EN3std10fatalErrorE()

declare void @_ENM3std8ArrayRefI3intE4initE5arrayP5ArrayI3intE.1(%"ArrayRef<int>"*, %"Array<int>"*)

define void @_ENM3std8ArrayRefI3intE4initE4dataP3int4size4uint(%"ArrayRef<int>"* %this, i32* %data, i32 %size) {
  %data1 = getelementptr inbounds %"ArrayRef<int>", %"ArrayRef<int>"* %this, i32 0, i32 0
  store i32* %data, i32** %data1
  %size2 = getelementptr inbounds %"ArrayRef<int>", %"ArrayRef<int>"* %this, i32 0, i32 1
  store i32 %size, i32* %size2
  ret void
}

define i32* @_EN3std8ArrayRefI3intEixE5index4uint(%"ArrayRef<int>" %this, i32 %index) {
  %data = extractvalue %"ArrayRef<int>" %this, 0
  %1 = getelementptr i32, i32* %data, i32 %index
  ret i32* %1
}

define %"ArrayIterator<int>" @_EN3std8ArrayRefI3intE8iteratorE(%"ArrayRef<int>" %this) {
  %1 = alloca %"ArrayIterator<int>"
  call void @_ENM3std13ArrayIteratorI3intE4initE5array8ArrayRefI3intE(%"ArrayIterator<int>"* %1, %"ArrayRef<int>" %this)
  %2 = load %"ArrayIterator<int>", %"ArrayIterator<int>"* %1
  ret %"ArrayIterator<int>" %2
}

define void @_EN4main1fI3intEE1aPAR_3int(%"ArrayRef<int>" %a) {
  %s = alloca i32
  %size = extractvalue %"ArrayRef<int>" %a, 1
  store i32 %size, i32* %s
  ret void
}

declare i8* @malloc(i64)

declare i32 @puts(i8*)

; Function Attrs: noreturn nounwind
declare void @llvm.trap() #0

declare void @free(i8*)

attributes #0 = { noreturn nounwind }
