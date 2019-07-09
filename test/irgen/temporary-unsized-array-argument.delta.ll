
%"ArrayRef<int>" = type { i32*, i32 }
%"List<int>" = type { i32*, i32, i32 }
%"RangeIterator<int>" = type { i32, i32 }
%"Range<int>" = type { i32, i32 }
%"ArrayIterator<int>" = type { i32*, i32* }
%"EnumeratedIterator<int>" = type { %"ArrayIterator<int>", i32 }
%"EnumeratedIteratorEntry<int>" = type { i32*, i32 }

@0 = private unnamed_addr constant [37 x i8] c"Assertion failed at List.delta:112:9\00"
@1 = private unnamed_addr constant [37 x i8] c"Assertion failed at List.delta:113:9\00"
@2 = private unnamed_addr constant [44 x i8] c"List index %d is out of bounds, size is %d\0A\00"
@3 = private unnamed_addr constant [30 x i8] c"Called first() on empty List\0A\00"
@4 = private unnamed_addr constant [29 x i8] c"Called last() on empty List\0A\00"
@5 = private unnamed_addr constant [36 x i8] c"Called removeFirst() on empty List\0A\00"
@6 = private unnamed_addr constant [35 x i8] c"Called removeLast() on empty List\0A\00"
@7 = private unnamed_addr constant [38 x i8] c"Unwrap failed at allocate.delta:36:62\00"

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

define void @_EN3std8ArrayRefI3intE4initE(%"ArrayRef<int>"* %this) {
  %size = getelementptr inbounds %"ArrayRef<int>", %"ArrayRef<int>"* %this, i32 0, i32 1
  store i32 0, i32* %size
  ret void
}

define void @_EN3std4ListI3intE4initE(%"List<int>"* %this) {
  %size = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 1
  store i32 0, i32* %size
  %capacity = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 2
  store i32 0, i32* %capacity
  ret void
}

define void @_EN3std4ListI3intE4initE8capacity3int(%"List<int>"* %this, i32 %capacity) {
  call void @_EN3std4ListI3intE4initE(%"List<int>"* %this)
  call void @_EN3std4ListI3intE7reserveE15minimumCapacity3int(%"List<int>"* %this, i32 %capacity)
  ret void
}

define void @_EN3std4ListI3intE7reserveE15minimumCapacity3int(%"List<int>"* %this, i32 %minimumCapacity) {
  %newBuffer = alloca i32*
  %__iterator = alloca %"RangeIterator<int>"
  %1 = alloca %"Range<int>"
  %index = alloca i32
  %source = alloca i32*
  %target = alloca i32*
  %capacity = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 2
  %capacity.load = load i32, i32* %capacity
  %2 = icmp sgt i32 %minimumCapacity, %capacity.load
  br i1 %2, label %if.then, label %if.else

if.then:                                          ; preds = %0
  %3 = call i32* @_EN3std13allocateArrayI3intEE4size3int(i32 %minimumCapacity)
  store i32* %3, i32** %newBuffer
  %size = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size
  call void @_EN3std5RangeI3intE4initE5start3int3end3int(%"Range<int>"* %1, i32 0, i32 %size.load)
  %4 = call %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(%"Range<int>"* %1)
  store %"RangeIterator<int>" %4, %"RangeIterator<int>"* %__iterator
  br label %loop.condition

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.end5
  ret void

loop.condition:                                   ; preds = %loop.increment, %if.then
  %5 = call i1 @_EN3std13RangeIteratorI3intE8hasValueE(%"RangeIterator<int>"* %__iterator)
  br i1 %5, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %6 = call i32 @_EN3std13RangeIteratorI3intE5valueE(%"RangeIterator<int>"* %__iterator)
  store i32 %6, i32* %index
  %buffer = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 0
  %buffer.load = load i32*, i32** %buffer
  %index.load = load i32, i32* %index
  %7 = getelementptr i32, i32* %buffer.load, i32 %index.load
  store i32* %7, i32** %source
  %newBuffer.load = load i32*, i32** %newBuffer
  %index.load1 = load i32, i32* %index
  %8 = getelementptr i32, i32* %newBuffer.load, i32 %index.load1
  store i32* %8, i32** %target
  %target.load = load i32*, i32** %target
  %source.load = load i32*, i32** %source
  %source.load.load = load i32, i32* %source.load
  store i32 %source.load.load, i32* %target.load
  br label %loop.increment

loop.increment:                                   ; preds = %loop.body
  call void @_EN3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  %capacity2 = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 2
  %capacity2.load = load i32, i32* %capacity2
  %9 = icmp ne i32 %capacity2.load, 0
  br i1 %9, label %if.then3, label %if.else4

if.then3:                                         ; preds = %loop.end
  %buffer6 = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 0
  %buffer6.load = load i32*, i32** %buffer6
  call void @_EN3std10deallocateIAU_3intEE10allocationPAU_3int(i32* %buffer6.load)
  br label %if.end5

if.else4:                                         ; preds = %loop.end
  br label %if.end5

if.end5:                                          ; preds = %if.else4, %if.then3
  %buffer7 = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 0
  %newBuffer.load8 = load i32*, i32** %newBuffer
  store i32* %newBuffer.load8, i32** %buffer7
  %capacity9 = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 2
  store i32 %minimumCapacity, i32* %capacity9
  br label %if.end
}

define void @_EN3std4ListI3intE4initE8elementsPAR_3int(%"List<int>"* %this, %"ArrayRef<int>" %elements) {
  %__iterator = alloca %"RangeIterator<int>"
  %1 = alloca %"Range<int>"
  %index = alloca i32
  %size = extractvalue %"ArrayRef<int>" %elements, 1
  call void @_EN3std4ListI3intE4initE8capacity3int(%"List<int>"* %this, i32 %size)
  %size1 = extractvalue %"ArrayRef<int>" %elements, 1
  call void @_EN3std5RangeI3intE4initE5start3int3end3int(%"Range<int>"* %1, i32 0, i32 %size1)
  %2 = call %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(%"Range<int>"* %1)
  store %"RangeIterator<int>" %2, %"RangeIterator<int>"* %__iterator
  br label %loop.condition

loop.condition:                                   ; preds = %loop.increment, %0
  %3 = call i1 @_EN3std13RangeIteratorI3intE8hasValueE(%"RangeIterator<int>"* %__iterator)
  br i1 %3, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %4 = call i32 @_EN3std13RangeIteratorI3intE5valueE(%"RangeIterator<int>"* %__iterator)
  store i32 %4, i32* %index
  %5 = extractvalue %"ArrayRef<int>" %elements, 0
  %index.load = load i32, i32* %index
  %6 = getelementptr i32, i32* %5, i32 %index.load
  %.load = load i32, i32* %6
  call void @_EN3std4ListI3intE4pushE10newElement3int(%"List<int>"* %this, i32 %.load)
  br label %loop.increment

loop.increment:                                   ; preds = %loop.body
  call void @_EN3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  ret void
}

declare %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(%"Range<int>"*)

declare void @_EN3std5RangeI3intE4initE5start3int3end3int(%"Range<int>"*, i32, i32)

declare i1 @_EN3std13RangeIteratorI3intE8hasValueE(%"RangeIterator<int>"*)

declare i32 @_EN3std13RangeIteratorI3intE5valueE(%"RangeIterator<int>"*)

define void @_EN3std4ListI3intE4pushE10newElement3int(%"List<int>"* %this, i32 %newElement) {
  %size = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size
  %capacity = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 2
  %capacity.load = load i32, i32* %capacity
  %1 = icmp eq i32 %size.load, %capacity.load
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  call void @_EN3std4ListI3intE4growE(%"List<int>"* %this)
  br label %if.end

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %buffer = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 0
  %buffer.load = load i32*, i32** %buffer
  %size1 = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 1
  %size1.load = load i32, i32* %size1
  %2 = getelementptr i32, i32* %buffer.load, i32 %size1.load
  store i32 %newElement, i32* %2
  %size2 = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 1
  %size2.load = load i32, i32* %size2
  %3 = add i32 %size2.load, 1
  store i32 %3, i32* %size2
  ret void
}

declare void @_EN3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"*)

define void @_EN3std4ListI3intE6deinitE(%"List<int>"* %this) {
  %capacity = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 2
  %capacity.load = load i32, i32* %capacity
  %1 = icmp ne i32 %capacity.load, 0
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  %buffer = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 0
  %buffer.load = load i32*, i32** %buffer
  call void @_EN3std10deallocateIAU_3intEE10allocationPAU_3int(i32* %buffer.load)
  br label %if.end

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  ret void
}

define void @_EN3std10deallocateIAU_3intEE10allocationPAU_3int(i32* %allocation) {
  %1 = bitcast i32* %allocation to i8*
  call void @free(i8* %1)
  ret void
}

define i32 @_EN3std4ListI3intE4sizeE(%"List<int>"* %this) {
  %size = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size
  ret i32 %size.load
}

define i1 @_EN3std4ListI3intE5emptyE(%"List<int>"* %this) {
  %size = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size
  %1 = icmp eq i32 %size.load, 0
  ret i1 %1
}

define i32 @_EN3std4ListI3intE8capacityE(%"List<int>"* %this) {
  %capacity = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 2
  %capacity.load = load i32, i32* %capacity
  ret i32 %capacity.load
}

define i32* @_EN3std4ListI3intEixE5index3int(%"List<int>"* %this, i32 %index) {
  %size = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size
  %1 = icmp sge i32 %index, %size.load
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  call void @_EN3std4ListI3intE16indexOutOfBoundsE5index3int(%"List<int>"* %this, i32 %index)
  br label %if.end

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %buffer = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 0
  %buffer.load = load i32*, i32** %buffer
  %2 = getelementptr i32, i32* %buffer.load, i32 %index
  ret i32* %2
}

define void @_EN3std4ListI3intE16indexOutOfBoundsE5index3int(%"List<int>"* %this, i32 %index) {
  %1 = call i32 @_EN3std4ListI3intE4sizeE(%"List<int>"* %this)
  %2 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([44 x i8], [44 x i8]* @2, i32 0, i32 0), i32 %index, i32 %1)
  call void @_EN3std10fatalErrorE()
  ret void
}

define i32* @_EN3std4ListI3intE5firstE(%"List<int>"* %this) {
  %size = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size
  %1 = icmp eq i32 %size.load, 0
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  call void @_EN3std4ListI3intE14emptyListFirstE(%"List<int>"* %this)
  br label %if.end

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %buffer = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 0
  %buffer.load = load i32*, i32** %buffer
  %2 = getelementptr i32, i32* %buffer.load, i32 0
  ret i32* %2
}

define void @_EN3std4ListI3intE14emptyListFirstE(%"List<int>"* %this) {
  %1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([30 x i8], [30 x i8]* @3, i32 0, i32 0))
  call void @_EN3std10fatalErrorE()
  ret void
}

define i32* @_EN3std4ListI3intE4lastE(%"List<int>"* %this) {
  %size = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size
  %1 = icmp eq i32 %size.load, 0
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  call void @_EN3std4ListI3intE13emptyListLastE(%"List<int>"* %this)
  br label %if.end

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %buffer = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 0
  %buffer.load = load i32*, i32** %buffer
  %size1 = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 1
  %size1.load = load i32, i32* %size1
  %2 = sub i32 %size1.load, 1
  %3 = getelementptr i32, i32* %buffer.load, i32 %2
  ret i32* %3
}

define void @_EN3std4ListI3intE13emptyListLastE(%"List<int>"* %this) {
  %1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([29 x i8], [29 x i8]* @4, i32 0, i32 0))
  call void @_EN3std10fatalErrorE()
  ret void
}

define i32* @_EN3std4ListI3intE4dataE(%"List<int>"* %this) {
  %buffer = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 0
  %buffer.load = load i32*, i32** %buffer
  ret i32* %buffer.load
}

define void @_EN3std4ListI3intE4growE(%"List<int>"* %this) {
  %capacity = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 2
  %capacity.load = load i32, i32* %capacity
  %1 = icmp eq i32 %capacity.load, 0
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  call void @_EN3std4ListI3intE7reserveE15minimumCapacity3int(%"List<int>"* %this, i32 1)
  br label %if.end

if.else:                                          ; preds = %0
  %capacity1 = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 2
  %capacity1.load = load i32, i32* %capacity1
  %2 = mul i32 %capacity1.load, 2
  call void @_EN3std4ListI3intE7reserveE15minimumCapacity3int(%"List<int>"* %this, i32 %2)
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  ret void
}

define i32* @_EN3std13allocateArrayI3intEE4size3int(i32 %size) {
  %1 = sext i32 %size to i64
  %2 = mul i64 ptrtoint (i32* getelementptr (i32, i32* null, i32 1) to i64), %1
  %3 = call i8* @malloc(i64 %2)
  %assert.condition = icmp eq i8* %3, null
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %0
  %4 = call i32 @puts(i8* getelementptr inbounds ([38 x i8], [38 x i8]* @7, i32 0, i32 0))
  call void @abort()
  unreachable

assert.success:                                   ; preds = %0
  %5 = bitcast i8* %3 to i32*
  ret i32* %5
}

define void @_EN3std4ListI3intE19resizeUninitializedE4size3int(%"List<int>"* %this, i32 %size) {
  %size1 = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 1
  %size1.load = load i32, i32* %size1
  %1 = icmp eq i32 %size1.load, 0
  %assert.condition = icmp eq i1 %1, false
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %0
  %2 = call i32 @puts(i8* getelementptr inbounds ([37 x i8], [37 x i8]* @0, i32 0, i32 0))
  call void @abort()
  unreachable

assert.success:                                   ; preds = %0
  %capacity = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 2
  %capacity.load = load i32, i32* %capacity
  %3 = icmp eq i32 %capacity.load, 0
  %assert.condition2 = icmp eq i1 %3, false
  br i1 %assert.condition2, label %assert.fail3, label %assert.success4

assert.fail3:                                     ; preds = %assert.success
  %4 = call i32 @puts(i8* getelementptr inbounds ([37 x i8], [37 x i8]* @1, i32 0, i32 0))
  call void @abort()
  unreachable

assert.success4:                                  ; preds = %assert.success
  %buffer = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 0
  %5 = call i32* @_EN3std13allocateArrayI3intEE4size3int(i32 %size)
  store i32* %5, i32** %buffer
  ret void
}

declare i32 @puts(i8*)

declare void @abort()

define void @_EN3std4ListI3intE11removeFirstE(%"List<int>"* %this) {
  %size = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size
  %1 = icmp eq i32 %size.load, 0
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  call void @_EN3std4ListI3intE20emptyListRemoveFirstE(%"List<int>"* %this)
  br label %if.end

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  call void @_EN3std4ListI3intE14unsafeRemoveAtE5index3int(%"List<int>"* %this, i32 0)
  ret void
}

define void @_EN3std4ListI3intE20emptyListRemoveFirstE(%"List<int>"* %this) {
  %1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([36 x i8], [36 x i8]* @5, i32 0, i32 0))
  call void @_EN3std10fatalErrorE()
  ret void
}

define void @_EN3std4ListI3intE14unsafeRemoveAtE5index3int(%"List<int>"* %this, i32 %index) {
  %__iterator = alloca %"RangeIterator<int>"
  %1 = alloca %"Range<int>"
  %i = alloca i32
  %source = alloca i32*
  %target = alloca i32*
  %2 = add i32 %index, 1
  %size = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size
  call void @_EN3std5RangeI3intE4initE5start3int3end3int(%"Range<int>"* %1, i32 %2, i32 %size.load)
  %3 = call %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(%"Range<int>"* %1)
  store %"RangeIterator<int>" %3, %"RangeIterator<int>"* %__iterator
  br label %loop.condition

loop.condition:                                   ; preds = %loop.increment, %0
  %4 = call i1 @_EN3std13RangeIteratorI3intE8hasValueE(%"RangeIterator<int>"* %__iterator)
  br i1 %4, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %5 = call i32 @_EN3std13RangeIteratorI3intE5valueE(%"RangeIterator<int>"* %__iterator)
  store i32 %5, i32* %i
  %buffer = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 0
  %buffer.load = load i32*, i32** %buffer
  %i.load = load i32, i32* %i
  %6 = getelementptr i32, i32* %buffer.load, i32 %i.load
  store i32* %6, i32** %source
  %buffer1 = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 0
  %buffer1.load = load i32*, i32** %buffer1
  %i.load2 = load i32, i32* %i
  %7 = sub i32 %i.load2, 1
  %8 = getelementptr i32, i32* %buffer1.load, i32 %7
  store i32* %8, i32** %target
  %target.load = load i32*, i32** %target
  %source.load = load i32*, i32** %source
  %source.load.load = load i32, i32* %source.load
  store i32 %source.load.load, i32* %target.load
  br label %loop.increment

loop.increment:                                   ; preds = %loop.body
  call void @_EN3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  %size3 = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 1
  %size3.load = load i32, i32* %size3
  %9 = sub i32 %size3.load, 1
  store i32 %9, i32* %size3
  ret void
}

define void @_EN3std4ListI3intE11removeFirstE12shouldRemoveFP3int_4bool(%"List<int>"* %this, i1 (i32*)* %shouldRemove) {
  %__iterator = alloca %"RangeIterator<int>"
  %1 = alloca %"Range<int>"
  %index = alloca i32
  %size = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size
  call void @_EN3std5RangeI3intE4initE5start3int3end3int(%"Range<int>"* %1, i32 0, i32 %size.load)
  %2 = call %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(%"Range<int>"* %1)
  store %"RangeIterator<int>" %2, %"RangeIterator<int>"* %__iterator
  br label %loop.condition

loop.condition:                                   ; preds = %loop.increment, %0
  %3 = call i1 @_EN3std13RangeIteratorI3intE8hasValueE(%"RangeIterator<int>"* %__iterator)
  br i1 %3, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %4 = call i32 @_EN3std13RangeIteratorI3intE5valueE(%"RangeIterator<int>"* %__iterator)
  store i32 %4, i32* %index
  %index.load = load i32, i32* %index
  %5 = call i32* @_EN3std4ListI3intEixE5index3int(%"List<int>"* %this, i32 %index.load)
  %6 = call i1 %shouldRemove(i32* %5)
  br i1 %6, label %if.then, label %if.else

loop.increment:                                   ; preds = %if.end
  call void @_EN3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %if.then, %loop.condition
  ret void

if.then:                                          ; preds = %loop.body
  %index.load1 = load i32, i32* %index
  call void @_EN3std4ListI3intE14unsafeRemoveAtE5index3int(%"List<int>"* %this, i32 %index.load1)
  br label %loop.end

if.else:                                          ; preds = %loop.body
  br label %if.end

if.end:                                           ; preds = %if.else
  br label %loop.increment
}

define void @_EN3std4ListI3intE10removeLastE(%"List<int>"* %this) {
  %size = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size
  %1 = icmp eq i32 %size.load, 0
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  call void @_EN3std4ListI3intE19emptyListRemoveLastE(%"List<int>"* %this)
  br label %if.end

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %size1 = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 1
  %size1.load = load i32, i32* %size1
  %2 = sub i32 %size1.load, 1
  store i32 %2, i32* %size1
  ret void
}

define void @_EN3std4ListI3intE19emptyListRemoveLastE(%"List<int>"* %this) {
  %1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([35 x i8], [35 x i8]* @6, i32 0, i32 0))
  call void @_EN3std10fatalErrorE()
  ret void
}

define i32 @_EN3std4ListI3intE3popE(%"List<int>"* %this) {
  %size = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size
  %1 = icmp eq i32 %size.load, 0
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  call void @_EN3std4ListI3intE19emptyListRemoveLastE(%"List<int>"* %this)
  br label %if.end

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %size1 = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 1
  %size1.load = load i32, i32* %size1
  %2 = sub i32 %size1.load, 1
  store i32 %2, i32* %size1
  %buffer = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 0
  %buffer.load = load i32*, i32** %buffer
  %size2 = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 1
  %size2.load = load i32, i32* %size2
  %3 = getelementptr i32, i32* %buffer.load, i32 %size2.load
  %.load = load i32, i32* %3
  ret i32 %.load
}

define void @_EN3std4ListI3intE8removeAtE5index3int(%"List<int>"* %this, i32 %index) {
  %size = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size
  %1 = icmp sge i32 %index, %size.load
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  call void @_EN3std4ListI3intE16indexOutOfBoundsE5index3int(%"List<int>"* %this, i32 %index)
  br label %if.end

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  call void @_EN3std4ListI3intE14unsafeRemoveAtE5index3int(%"List<int>"* %this, i32 %index)
  ret void
}

define void @_EN3std13ArrayIteratorI3intE4initE5array8ArrayRefI3intE(%"ArrayIterator<int>"* %this, %"ArrayRef<int>" %array) {
  %1 = alloca %"ArrayRef<int>"
  %2 = alloca %"ArrayRef<int>"
  %3 = alloca %"ArrayRef<int>"
  %current = getelementptr inbounds %"ArrayIterator<int>", %"ArrayIterator<int>"* %this, i32 0, i32 0
  store %"ArrayRef<int>" %array, %"ArrayRef<int>"* %1
  %4 = call i32* @_EN3std8ArrayRefI3intE4dataE(%"ArrayRef<int>"* %1)
  store i32* %4, i32** %current
  %end = getelementptr inbounds %"ArrayIterator<int>", %"ArrayIterator<int>"* %this, i32 0, i32 1
  store %"ArrayRef<int>" %array, %"ArrayRef<int>"* %2
  %5 = call i32* @_EN3std8ArrayRefI3intE4dataE(%"ArrayRef<int>"* %2)
  store %"ArrayRef<int>" %array, %"ArrayRef<int>"* %3
  %6 = call i32 @_EN3std8ArrayRefI3intE4sizeE(%"ArrayRef<int>"* %3)
  %7 = getelementptr i32, i32* %5, i32 %6
  store i32* %7, i32** %end
  ret void
}

define i32* @_EN3std8ArrayRefI3intE4dataE(%"ArrayRef<int>"* %this) {
  %data = getelementptr inbounds %"ArrayRef<int>", %"ArrayRef<int>"* %this, i32 0, i32 0
  %data.load = load i32*, i32** %data
  ret i32* %data.load
}

define i32 @_EN3std8ArrayRefI3intE4sizeE(%"ArrayRef<int>"* %this) {
  %size = getelementptr inbounds %"ArrayRef<int>", %"ArrayRef<int>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size
  ret i32 %size.load
}

define i1 @_EN3std13ArrayIteratorI3intE8hasValueE(%"ArrayIterator<int>"* %this) {
  %current = getelementptr inbounds %"ArrayIterator<int>", %"ArrayIterator<int>"* %this, i32 0, i32 0
  %current.load = load i32*, i32** %current
  %end = getelementptr inbounds %"ArrayIterator<int>", %"ArrayIterator<int>"* %this, i32 0, i32 1
  %end.load = load i32*, i32** %end
  %1 = icmp ne i32* %current.load, %end.load
  ret i1 %1
}

define i32* @_EN3std13ArrayIteratorI3intE5valueE(%"ArrayIterator<int>"* %this) {
  %current = getelementptr inbounds %"ArrayIterator<int>", %"ArrayIterator<int>"* %this, i32 0, i32 0
  %current.load = load i32*, i32** %current
  ret i32* %current.load
}

define void @_EN3std13ArrayIteratorI3intE9incrementE(%"ArrayIterator<int>"* %this) {
  %current = getelementptr inbounds %"ArrayIterator<int>", %"ArrayIterator<int>"* %this, i32 0, i32 0
  %current.load = load i32*, i32** %current
  %1 = getelementptr i32, i32* %current.load, i32 1
  store i32* %1, i32** %current
  ret void
}

define %"ArrayIterator<int>" @_EN3std4ListI3intE8iteratorE(%"List<int>"* %this) {
  %1 = alloca %"ArrayIterator<int>"
  %2 = alloca %"ArrayRef<int>"
  call void @_EN3std8ArrayRefI3intE4initE4listP4ListI3intE(%"ArrayRef<int>"* %2, %"List<int>"* %this)
  %.load = load %"ArrayRef<int>", %"ArrayRef<int>"* %2
  call void @_EN3std13ArrayIteratorI3intE4initE5array8ArrayRefI3intE(%"ArrayIterator<int>"* %1, %"ArrayRef<int>" %.load)
  %.load1 = load %"ArrayIterator<int>", %"ArrayIterator<int>"* %1
  ret %"ArrayIterator<int>" %.load1
}

define void @_EN3std8ArrayRefI3intE4initE4listP4ListI3intE(%"ArrayRef<int>"* %this, %"List<int>"* %list) {
  %data = getelementptr inbounds %"ArrayRef<int>", %"ArrayRef<int>"* %this, i32 0, i32 0
  %1 = call i32* @_EN3std4ListI3intE4dataE(%"List<int>"* %list)
  store i32* %1, i32** %data
  %size = getelementptr inbounds %"ArrayRef<int>", %"ArrayRef<int>"* %this, i32 0, i32 1
  %2 = call i32 @_EN3std4ListI3intE4sizeE(%"List<int>"* %list)
  store i32 %2, i32* %size
  ret void
}

define void @_EN3std18EnumeratedIteratorI3intE4initE8iterator13ArrayIteratorI3intE(%"EnumeratedIterator<int>"* %this, %"ArrayIterator<int>" %iterator) {
  %base = getelementptr inbounds %"EnumeratedIterator<int>", %"EnumeratedIterator<int>"* %this, i32 0, i32 0
  store %"ArrayIterator<int>" %iterator, %"ArrayIterator<int>"* %base
  %index = getelementptr inbounds %"EnumeratedIterator<int>", %"EnumeratedIterator<int>"* %this, i32 0, i32 1
  store i32 0, i32* %index
  ret void
}

define i1 @_EN3std18EnumeratedIteratorI3intE8hasValueE(%"EnumeratedIterator<int>"* %this) {
  %base = getelementptr inbounds %"EnumeratedIterator<int>", %"EnumeratedIterator<int>"* %this, i32 0, i32 0
  %1 = call i1 @_EN3std13ArrayIteratorI3intE8hasValueE(%"ArrayIterator<int>"* %base)
  ret i1 %1
}

define void @_EN3std23EnumeratedIteratorEntryI3intE4initE5valueP3int5index3int(%"EnumeratedIteratorEntry<int>"* %this, i32* %value, i32 %index) {
  %value1 = getelementptr inbounds %"EnumeratedIteratorEntry<int>", %"EnumeratedIteratorEntry<int>"* %this, i32 0, i32 0
  store i32* %value, i32** %value1
  %index2 = getelementptr inbounds %"EnumeratedIteratorEntry<int>", %"EnumeratedIteratorEntry<int>"* %this, i32 0, i32 1
  store i32 %index, i32* %index2
  ret void
}

define %"EnumeratedIteratorEntry<int>" @_EN3std18EnumeratedIteratorI3intE5valueE(%"EnumeratedIterator<int>"* %this) {
  %1 = alloca %"EnumeratedIteratorEntry<int>"
  %base = getelementptr inbounds %"EnumeratedIterator<int>", %"EnumeratedIterator<int>"* %this, i32 0, i32 0
  %2 = call i32* @_EN3std13ArrayIteratorI3intE5valueE(%"ArrayIterator<int>"* %base)
  %index = getelementptr inbounds %"EnumeratedIterator<int>", %"EnumeratedIterator<int>"* %this, i32 0, i32 1
  %index.load = load i32, i32* %index
  call void @_EN3std23EnumeratedIteratorEntryI3intE4initE5valueP3int5index3int(%"EnumeratedIteratorEntry<int>"* %1, i32* %2, i32 %index.load)
  %.load = load %"EnumeratedIteratorEntry<int>", %"EnumeratedIteratorEntry<int>"* %1
  ret %"EnumeratedIteratorEntry<int>" %.load
}

define void @_EN3std18EnumeratedIteratorI3intE9incrementE(%"EnumeratedIterator<int>"* %this) {
  %base = getelementptr inbounds %"EnumeratedIterator<int>", %"EnumeratedIterator<int>"* %this, i32 0, i32 0
  call void @_EN3std13ArrayIteratorI3intE9incrementE(%"ArrayIterator<int>"* %base)
  %index = getelementptr inbounds %"EnumeratedIterator<int>", %"EnumeratedIterator<int>"* %this, i32 0, i32 1
  %index.load = load i32, i32* %index
  %1 = add i32 %index.load, 1
  store i32 %1, i32* %index
  ret void
}

define %"EnumeratedIterator<int>" @_EN3std4ListI3intE9enumerateE(%"List<int>"* %this) {
  %1 = alloca %"EnumeratedIterator<int>"
  %2 = call %"ArrayIterator<int>" @_EN3std4ListI3intE8iteratorE(%"List<int>"* %this)
  call void @_EN3std18EnumeratedIteratorI3intE4initE8iterator13ArrayIteratorI3intE(%"EnumeratedIterator<int>"* %1, %"ArrayIterator<int>" %2)
  %.load = load %"EnumeratedIterator<int>", %"EnumeratedIterator<int>"* %1
  ret %"EnumeratedIterator<int>" %.load
}

declare i32 @printf(i8*, ...)

declare void @_EN3std10fatalErrorE()

declare void @_EN3std8ArrayRefI3intE4initE4listP4ListI3intE.1(%"ArrayRef<int>"*, %"List<int>"*)

define void @_EN3std8ArrayRefI3intE4initE4dataP3int4size3int(%"ArrayRef<int>"* %this, i32* %data, i32 %size) {
  %data1 = getelementptr inbounds %"ArrayRef<int>", %"ArrayRef<int>"* %this, i32 0, i32 0
  store i32* %data, i32** %data1
  %size2 = getelementptr inbounds %"ArrayRef<int>", %"ArrayRef<int>"* %this, i32 0, i32 1
  store i32 %size, i32* %size2
  ret void
}

define i32* @_EN3std8ArrayRefI3intEixE5index3int(%"ArrayRef<int>"* %this, i32 %index) {
  %data = getelementptr inbounds %"ArrayRef<int>", %"ArrayRef<int>"* %this, i32 0, i32 0
  %data.load = load i32*, i32** %data
  %1 = getelementptr i32, i32* %data.load, i32 %index
  ret i32* %1
}

define %"ArrayIterator<int>" @_EN3std8ArrayRefI3intE8iteratorE(%"ArrayRef<int>"* %this) {
  %1 = alloca %"ArrayIterator<int>"
  %this.load = load %"ArrayRef<int>", %"ArrayRef<int>"* %this
  call void @_EN3std13ArrayIteratorI3intE4initE5array8ArrayRefI3intE(%"ArrayIterator<int>"* %1, %"ArrayRef<int>" %this.load)
  %.load = load %"ArrayIterator<int>", %"ArrayIterator<int>"* %1
  ret %"ArrayIterator<int>" %.load
}

define void @_EN4main1fI3intEE1aPAR_3int(%"ArrayRef<int>" %a) {
  %s = alloca i32
  %size = extractvalue %"ArrayRef<int>" %a, 1
  store i32 %size, i32* %s
  ret void
}

declare void @free(i8*)

declare i8* @malloc(i64)
