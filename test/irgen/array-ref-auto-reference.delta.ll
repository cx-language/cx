
%"ArrayRef<int>" = type { i32*, i32 }
%"Array<int>" = type { i32*, i32, i32 }
%"RangeIterator<int>" = type { i32, i32 }
%"Range<int>" = type { i32, i32 }
%"ArrayIterator<int>" = type { i32*, i32* }
%"MutableArrayIterator<int>" = type { i32*, i32* }
%"EnumeratedIterator<int>" = type { %"ArrayIterator<int>", i32 }
%"EnumeratedIteratorEntry<int>" = type { i32*, i32 }

@0 = private unnamed_addr constant [45 x i8] c"Array index %d is out of bounds, size is %d\0A\00"
@1 = private unnamed_addr constant [31 x i8] c"Called first() on empty Array\0A\00"
@2 = private unnamed_addr constant [30 x i8] c"Called last() on empty Array\0A\00"
@3 = private unnamed_addr constant [37 x i8] c"Called removeFirst() on empty Array\0A\00"
@4 = private unnamed_addr constant [36 x i8] c"Called removeLast() on empty Array\0A\00"
@5 = private unnamed_addr constant [38 x i8] c"Unwrap failed at allocate.delta:36:70\00"

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

define void @_ENM3std5ArrayI3intE4initE8capacity3int(%"Array<int>"* %this, i32 %capacity) {
  call void @_ENM3std5ArrayI3intE4initE(%"Array<int>"* %this)
  call void @_ENM3std5ArrayI3intE7reserveE15minimumCapacity3int(%"Array<int>"* %this, i32 %capacity)
  ret void
}

define void @_ENM3std5ArrayI3intE7reserveE15minimumCapacity3int(%"Array<int>"* %this, i32 %minimumCapacity) {
  %newBuffer = alloca i32*
  %__iterator = alloca %"RangeIterator<int>"
  %1 = alloca %"Range<int>"
  %index = alloca i32
  %source = alloca i32*
  %target = alloca i32*
  %capacity = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 2
  %capacity.load = load i32, i32* %capacity
  %2 = icmp sgt i32 %minimumCapacity, %capacity.load
  br i1 %2, label %if.then, label %if.else

if.then:                                          ; preds = %0
  %3 = call i32* @_EN3std13allocateArrayI3intEE4size3int(i32 %minimumCapacity)
  store i32* %3, i32** %newBuffer
  %size = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size
  call void @_ENM3std5RangeI3intE4initE5start3int3end3int(%"Range<int>"* %1, i32 0, i32 %size.load)
  %.load = load %"Range<int>", %"Range<int>"* %1
  %4 = call %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(%"Range<int>" %.load)
  store %"RangeIterator<int>" %4, %"RangeIterator<int>"* %__iterator
  br label %loop.condition

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.end6
  ret void

loop.condition:                                   ; preds = %loop.increment, %if.then
  %__iterator.load = load %"RangeIterator<int>", %"RangeIterator<int>"* %__iterator
  %5 = call i1 @_EN3std13RangeIteratorI3intE8hasValueE(%"RangeIterator<int>" %__iterator.load)
  br i1 %5, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %__iterator.load1 = load %"RangeIterator<int>", %"RangeIterator<int>"* %__iterator
  %6 = call i32 @_EN3std13RangeIteratorI3intE5valueE(%"RangeIterator<int>" %__iterator.load1)
  store i32 %6, i32* %index
  %buffer = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 0
  %buffer.load = load i32*, i32** %buffer
  %index.load = load i32, i32* %index
  %7 = getelementptr i32, i32* %buffer.load, i32 %index.load
  store i32* %7, i32** %source
  %newBuffer.load = load i32*, i32** %newBuffer
  %index.load2 = load i32, i32* %index
  %8 = getelementptr i32, i32* %newBuffer.load, i32 %index.load2
  store i32* %8, i32** %target
  %target.load = load i32*, i32** %target
  %source.load = load i32*, i32** %source
  %source.load.load = load i32, i32* %source.load
  store i32 %source.load.load, i32* %target.load
  br label %loop.increment

loop.increment:                                   ; preds = %loop.body
  call void @_ENM3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  %capacity3 = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 2
  %capacity3.load = load i32, i32* %capacity3
  %9 = icmp ne i32 %capacity3.load, 0
  br i1 %9, label %if.then4, label %if.else5

if.then4:                                         ; preds = %loop.end
  %buffer7 = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 0
  %buffer7.load = load i32*, i32** %buffer7
  call void @_EN3std10deallocateIAU_3intEE10allocationPMAU_M3int(i32* %buffer7.load)
  br label %if.end6

if.else5:                                         ; preds = %loop.end
  br label %if.end6

if.end6:                                          ; preds = %if.else5, %if.then4
  %buffer8 = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 0
  %newBuffer.load9 = load i32*, i32** %newBuffer
  store i32* %newBuffer.load9, i32** %buffer8
  %capacity10 = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 2
  store i32 %minimumCapacity, i32* %capacity10
  br label %if.end
}

define void @_ENM3std5ArrayI3intE4initE8elementsPAR_3int(%"Array<int>"* %this, %"ArrayRef<int>" %elements) {
  %__iterator = alloca %"RangeIterator<int>"
  %1 = alloca %"Range<int>"
  %index = alloca i32
  %size = extractvalue %"ArrayRef<int>" %elements, 1
  call void @_ENM3std5ArrayI3intE4initE8capacity3int(%"Array<int>"* %this, i32 %size)
  %size1 = extractvalue %"ArrayRef<int>" %elements, 1
  call void @_ENM3std5RangeI3intE4initE5start3int3end3int(%"Range<int>"* %1, i32 0, i32 %size1)
  %.load = load %"Range<int>", %"Range<int>"* %1
  %2 = call %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(%"Range<int>" %.load)
  store %"RangeIterator<int>" %2, %"RangeIterator<int>"* %__iterator
  br label %loop.condition

loop.condition:                                   ; preds = %loop.increment, %0
  %__iterator.load = load %"RangeIterator<int>", %"RangeIterator<int>"* %__iterator
  %3 = call i1 @_EN3std13RangeIteratorI3intE8hasValueE(%"RangeIterator<int>" %__iterator.load)
  br i1 %3, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %__iterator.load2 = load %"RangeIterator<int>", %"RangeIterator<int>"* %__iterator
  %4 = call i32 @_EN3std13RangeIteratorI3intE5valueE(%"RangeIterator<int>" %__iterator.load2)
  store i32 %4, i32* %index
  %5 = extractvalue %"ArrayRef<int>" %elements, 0
  %index.load = load i32, i32* %index
  %6 = getelementptr i32, i32* %5, i32 %index.load
  %.load3 = load i32, i32* %6
  call void @_ENM3std5ArrayI3intE4pushE10newElement3int(%"Array<int>"* %this, i32 %.load3)
  br label %loop.increment

loop.increment:                                   ; preds = %loop.body
  call void @_ENM3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  ret void
}

declare %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(%"Range<int>")

declare void @_ENM3std5RangeI3intE4initE5start3int3end3int(%"Range<int>"*, i32, i32)

declare i1 @_EN3std13RangeIteratorI3intE8hasValueE(%"RangeIterator<int>")

declare i32 @_EN3std13RangeIteratorI3intE5valueE(%"RangeIterator<int>")

define void @_ENM3std5ArrayI3intE4pushE10newElement3int(%"Array<int>"* %this, i32 %newElement) {
  %size = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size
  %capacity = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 2
  %capacity.load = load i32, i32* %capacity
  %1 = icmp eq i32 %size.load, %capacity.load
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  call void @_ENM3std5ArrayI3intE4growE(%"Array<int>"* %this)
  br label %if.end

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %buffer = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 0
  %buffer.load = load i32*, i32** %buffer
  %size1 = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 1
  %size1.load = load i32, i32* %size1
  %2 = getelementptr i32, i32* %buffer.load, i32 %size1.load
  store i32 %newElement, i32* %2
  %size2 = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 1
  %size2.load = load i32, i32* %size2
  %3 = add i32 %size2.load, 1
  store i32 %3, i32* %size2
  ret void
}

declare void @_ENM3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"*)

define void @_ENM3std5ArrayI3intE6deinitE(%"Array<int>"* %this) {
  %capacity = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 2
  %capacity.load = load i32, i32* %capacity
  %1 = icmp ne i32 %capacity.load, 0
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  %buffer = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 0
  %buffer.load = load i32*, i32** %buffer
  call void @_EN3std10deallocateIAU_3intEE10allocationPMAU_M3int(i32* %buffer.load)
  br label %if.end

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  ret void
}

define void @_EN3std10deallocateIAU_3intEE10allocationPMAU_M3int(i32* %allocation) {
  %1 = bitcast i32* %allocation to i8*
  call void @free(i8* %1)
  ret void
}

define i32 @_EN3std5ArrayI3intE4sizeE(%"Array<int>"* %this) {
  %size = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size
  ret i32 %size.load
}

define i1 @_EN3std5ArrayI3intE5emptyE(%"Array<int>"* %this) {
  %size = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size
  %1 = icmp eq i32 %size.load, 0
  ret i1 %1
}

define i32 @_EN3std5ArrayI3intE8capacityE(%"Array<int>"* %this) {
  %capacity = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 2
  %capacity.load = load i32, i32* %capacity
  ret i32 %capacity.load
}

define i32* @_EN3std5ArrayI3intEixE5index3int(%"Array<int>"* %this, i32 %index) {
  %size = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size
  %1 = icmp sge i32 %index, %size.load
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  call void @_EN3std5ArrayI3intE16indexOutOfBoundsE5index3int(%"Array<int>"* %this, i32 %index)
  br label %if.end

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %buffer = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 0
  %buffer.load = load i32*, i32** %buffer
  %2 = getelementptr i32, i32* %buffer.load, i32 %index
  ret i32* %2
}

define void @_EN3std5ArrayI3intE16indexOutOfBoundsE5index3int(%"Array<int>"* %this, i32 %index) {
  %1 = call i32 @_EN3std5ArrayI3intE4sizeE(%"Array<int>"* %this)
  %2 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([45 x i8], [45 x i8]* @0, i32 0, i32 0), i32 %index, i32 %1)
  call void @_EN3std10fatalErrorE()
  ret void
}

define i32* @_ENM3std5ArrayI3intEixE5index3int(%"Array<int>"* %this, i32 %index) {
  %size = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size
  %1 = icmp sge i32 %index, %size.load
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  call void @_EN3std5ArrayI3intE16indexOutOfBoundsE5index3int(%"Array<int>"* %this, i32 %index)
  br label %if.end

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %buffer = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 0
  %buffer.load = load i32*, i32** %buffer
  %2 = getelementptr i32, i32* %buffer.load, i32 %index
  ret i32* %2
}

define i32* @_EN3std5ArrayI3intE5firstE(%"Array<int>"* %this) {
  %size = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size
  %1 = icmp eq i32 %size.load, 0
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  call void @_EN3std5ArrayI3intE15emptyArrayFirstE(%"Array<int>"* %this)
  br label %if.end

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %buffer = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 0
  %buffer.load = load i32*, i32** %buffer
  %2 = getelementptr i32, i32* %buffer.load, i32 0
  ret i32* %2
}

define void @_EN3std5ArrayI3intE15emptyArrayFirstE(%"Array<int>"* %this) {
  %1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([31 x i8], [31 x i8]* @1, i32 0, i32 0))
  call void @_EN3std10fatalErrorE()
  ret void
}

define i32* @_EN3std5ArrayI3intE4lastE(%"Array<int>"* %this) {
  %size = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size
  %1 = icmp eq i32 %size.load, 0
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  call void @_EN3std5ArrayI3intE14emptyArrayLastE(%"Array<int>"* %this)
  br label %if.end

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %buffer = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 0
  %buffer.load = load i32*, i32** %buffer
  %size1 = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 1
  %size1.load = load i32, i32* %size1
  %2 = sub i32 %size1.load, 1
  %3 = getelementptr i32, i32* %buffer.load, i32 %2
  ret i32* %3
}

define void @_EN3std5ArrayI3intE14emptyArrayLastE(%"Array<int>"* %this) {
  %1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([30 x i8], [30 x i8]* @2, i32 0, i32 0))
  call void @_EN3std10fatalErrorE()
  ret void
}

define i32* @_EN3std5ArrayI3intE4dataE(%"Array<int>"* %this) {
  %buffer = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 0
  %buffer.load = load i32*, i32** %buffer
  ret i32* %buffer.load
}

define i32* @_ENM3std5ArrayI3intE4dataE(%"Array<int>"* %this) {
  %buffer = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 0
  %buffer.load = load i32*, i32** %buffer
  ret i32* %buffer.load
}

define void @_ENM3std5ArrayI3intE4growE(%"Array<int>"* %this) {
  %capacity = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 2
  %capacity.load = load i32, i32* %capacity
  %1 = icmp eq i32 %capacity.load, 0
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  call void @_ENM3std5ArrayI3intE7reserveE15minimumCapacity3int(%"Array<int>"* %this, i32 1)
  br label %if.end

if.else:                                          ; preds = %0
  %capacity1 = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 2
  %capacity1.load = load i32, i32* %capacity1
  %2 = mul i32 %capacity1.load, 2
  call void @_ENM3std5ArrayI3intE7reserveE15minimumCapacity3int(%"Array<int>"* %this, i32 %2)
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
  %4 = call i32 @puts(i8* getelementptr inbounds ([38 x i8], [38 x i8]* @5, i32 0, i32 0))
  call void @abort()
  unreachable

assert.success:                                   ; preds = %0
  %5 = bitcast i8* %3 to i32*
  ret i32* %5
}

define void @_ENM3std5ArrayI3intE11removeFirstE(%"Array<int>"* %this) {
  %size = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size
  %1 = icmp eq i32 %size.load, 0
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  call void @_EN3std5ArrayI3intE21emptyArrayRemoveFirstE(%"Array<int>"* %this)
  br label %if.end

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  call void @_ENM3std5ArrayI3intE14unsafeRemoveAtE5index3int(%"Array<int>"* %this, i32 0)
  ret void
}

define void @_EN3std5ArrayI3intE21emptyArrayRemoveFirstE(%"Array<int>"* %this) {
  %1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([37 x i8], [37 x i8]* @3, i32 0, i32 0))
  call void @_EN3std10fatalErrorE()
  ret void
}

define void @_ENM3std5ArrayI3intE14unsafeRemoveAtE5index3int(%"Array<int>"* %this, i32 %index) {
  %__iterator = alloca %"RangeIterator<int>"
  %1 = alloca %"Range<int>"
  %i = alloca i32
  %source = alloca i32*
  %target = alloca i32*
  %2 = add i32 %index, 1
  %size = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size
  call void @_ENM3std5RangeI3intE4initE5start3int3end3int(%"Range<int>"* %1, i32 %2, i32 %size.load)
  %.load = load %"Range<int>", %"Range<int>"* %1
  %3 = call %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(%"Range<int>" %.load)
  store %"RangeIterator<int>" %3, %"RangeIterator<int>"* %__iterator
  br label %loop.condition

loop.condition:                                   ; preds = %loop.increment, %0
  %__iterator.load = load %"RangeIterator<int>", %"RangeIterator<int>"* %__iterator
  %4 = call i1 @_EN3std13RangeIteratorI3intE8hasValueE(%"RangeIterator<int>" %__iterator.load)
  br i1 %4, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %__iterator.load1 = load %"RangeIterator<int>", %"RangeIterator<int>"* %__iterator
  %5 = call i32 @_EN3std13RangeIteratorI3intE5valueE(%"RangeIterator<int>" %__iterator.load1)
  store i32 %5, i32* %i
  %buffer = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 0
  %buffer.load = load i32*, i32** %buffer
  %i.load = load i32, i32* %i
  %6 = getelementptr i32, i32* %buffer.load, i32 %i.load
  store i32* %6, i32** %source
  %buffer2 = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 0
  %buffer2.load = load i32*, i32** %buffer2
  %i.load3 = load i32, i32* %i
  %7 = sub i32 %i.load3, 1
  %8 = getelementptr i32, i32* %buffer2.load, i32 %7
  store i32* %8, i32** %target
  %target.load = load i32*, i32** %target
  %source.load = load i32*, i32** %source
  %source.load.load = load i32, i32* %source.load
  store i32 %source.load.load, i32* %target.load
  br label %loop.increment

loop.increment:                                   ; preds = %loop.body
  call void @_ENM3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  %size4 = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 1
  %size4.load = load i32, i32* %size4
  %9 = sub i32 %size4.load, 1
  store i32 %9, i32* %size4
  ret void
}

define void @_ENM3std5ArrayI3intE10removeLastE(%"Array<int>"* %this) {
  %size = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size
  %1 = icmp eq i32 %size.load, 0
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  call void @_EN3std5ArrayI3intE20emptyArrayRemoveLastE(%"Array<int>"* %this)
  br label %if.end

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %size1 = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 1
  %size1.load = load i32, i32* %size1
  %2 = sub i32 %size1.load, 1
  store i32 %2, i32* %size1
  ret void
}

define void @_EN3std5ArrayI3intE20emptyArrayRemoveLastE(%"Array<int>"* %this) {
  %1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([36 x i8], [36 x i8]* @4, i32 0, i32 0))
  call void @_EN3std10fatalErrorE()
  ret void
}

define i32 @_ENM3std5ArrayI3intE3popE(%"Array<int>"* %this) {
  %size = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size
  %1 = icmp eq i32 %size.load, 0
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  call void @_EN3std5ArrayI3intE20emptyArrayRemoveLastE(%"Array<int>"* %this)
  br label %if.end

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %size1 = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 1
  %size1.load = load i32, i32* %size1
  %2 = sub i32 %size1.load, 1
  store i32 %2, i32* %size1
  %buffer = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 0
  %buffer.load = load i32*, i32** %buffer
  %size2 = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 1
  %size2.load = load i32, i32* %size2
  %3 = getelementptr i32, i32* %buffer.load, i32 %size2.load
  %.load = load i32, i32* %3
  ret i32 %.load
}

define void @_ENM3std5ArrayI3intE8removeAtE5index3int(%"Array<int>"* %this, i32 %index) {
  %size = getelementptr inbounds %"Array<int>", %"Array<int>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size
  %1 = icmp sge i32 %index, %size.load
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  call void @_EN3std5ArrayI3intE16indexOutOfBoundsE5index3int(%"Array<int>"* %this, i32 %index)
  br label %if.end

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  call void @_ENM3std5ArrayI3intE14unsafeRemoveAtE5index3int(%"Array<int>"* %this, i32 %index)
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
  %current.load = load i32*, i32** %current
  %1 = getelementptr i32, i32* %current.load, i32 1
  store i32* %1, i32** %current
  ret void
}

define %"ArrayIterator<int>" @_EN3std5ArrayI3intE8iteratorE(%"Array<int>"* %this) {
  %1 = alloca %"ArrayIterator<int>"
  %2 = alloca %"ArrayRef<int>"
  call void @_ENM3std8ArrayRefI3intE4initE5arrayP5ArrayI3intE(%"ArrayRef<int>"* %2, %"Array<int>"* %this)
  %.load = load %"ArrayRef<int>", %"ArrayRef<int>"* %2
  call void @_ENM3std13ArrayIteratorI3intE4initE5array8ArrayRefI3intE(%"ArrayIterator<int>"* %1, %"ArrayRef<int>" %.load)
  %.load1 = load %"ArrayIterator<int>", %"ArrayIterator<int>"* %1
  ret %"ArrayIterator<int>" %.load1
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
  %current.load = load i32*, i32** %current
  %1 = getelementptr i32, i32* %current.load, i32 1
  store i32* %1, i32** %current
  ret void
}

define %"MutableArrayIterator<int>" @_ENM3std5ArrayI3intE8iteratorE(%"Array<int>"* %this) {
  %1 = alloca %"MutableArrayIterator<int>"
  call void @_ENM3std20MutableArrayIteratorI3intE4initE5arrayPM5ArrayI3intE(%"MutableArrayIterator<int>"* %1, %"Array<int>"* %this)
  %.load = load %"MutableArrayIterator<int>", %"MutableArrayIterator<int>"* %1
  ret %"MutableArrayIterator<int>" %.load
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

define void @_ENM3std23EnumeratedIteratorEntryI3intE4initE5valueP3int5index3int(%"EnumeratedIteratorEntry<int>"* %this, i32* %value, i32 %index) {
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
  call void @_ENM3std23EnumeratedIteratorEntryI3intE4initE5valueP3int5index3int(%"EnumeratedIteratorEntry<int>"* %1, i32* %2, i32 %index)
  %.load = load %"EnumeratedIteratorEntry<int>", %"EnumeratedIteratorEntry<int>"* %1
  ret %"EnumeratedIteratorEntry<int>" %.load
}

define void @_ENM3std18EnumeratedIteratorI3intE9incrementE(%"EnumeratedIterator<int>"* %this) {
  %base = getelementptr inbounds %"EnumeratedIterator<int>", %"EnumeratedIterator<int>"* %this, i32 0, i32 0
  call void @_ENM3std13ArrayIteratorI3intE9incrementE(%"ArrayIterator<int>"* %base)
  %index = getelementptr inbounds %"EnumeratedIterator<int>", %"EnumeratedIterator<int>"* %this, i32 0, i32 1
  %index.load = load i32, i32* %index
  %1 = add i32 %index.load, 1
  store i32 %1, i32* %index
  ret void
}

define %"EnumeratedIterator<int>" @_EN3std5ArrayI3intE9enumerateE(%"Array<int>"* %this) {
  %1 = alloca %"EnumeratedIterator<int>"
  %2 = call %"ArrayIterator<int>" @_EN3std5ArrayI3intE8iteratorE(%"Array<int>"* %this)
  call void @_ENM3std18EnumeratedIteratorI3intE4initE8iterator13ArrayIteratorI3intE(%"EnumeratedIterator<int>"* %1, %"ArrayIterator<int>" %2)
  %.load = load %"EnumeratedIterator<int>", %"EnumeratedIterator<int>"* %1
  ret %"EnumeratedIterator<int>" %.load
}

declare i32 @printf(i8*, ...)

declare void @_EN3std10fatalErrorE()

declare void @_ENM3std8ArrayRefI3intE4initE5arrayP5ArrayI3intE.1(%"ArrayRef<int>"*, %"Array<int>"*)

define void @_ENM3std8ArrayRefI3intE4initE4dataP3int4size3int(%"ArrayRef<int>"* %this, i32* %data, i32 %size) {
  %data1 = getelementptr inbounds %"ArrayRef<int>", %"ArrayRef<int>"* %this, i32 0, i32 0
  store i32* %data, i32** %data1
  %size2 = getelementptr inbounds %"ArrayRef<int>", %"ArrayRef<int>"* %this, i32 0, i32 1
  store i32 %size, i32* %size2
  ret void
}

define i32* @_EN3std8ArrayRefI3intEixE5index3int(%"ArrayRef<int>" %this, i32 %index) {
  %data = extractvalue %"ArrayRef<int>" %this, 0
  %1 = getelementptr i32, i32* %data, i32 %index
  ret i32* %1
}

define %"ArrayIterator<int>" @_EN3std8ArrayRefI3intE8iteratorE(%"ArrayRef<int>" %this) {
  %1 = alloca %"ArrayIterator<int>"
  call void @_ENM3std13ArrayIteratorI3intE4initE5array8ArrayRefI3intE(%"ArrayIterator<int>"* %1, %"ArrayRef<int>" %this)
  %.load = load %"ArrayIterator<int>", %"ArrayIterator<int>"* %1
  ret %"ArrayIterator<int>" %.load
}

define void @_EN4main3fooE1aPAR_3int(%"ArrayRef<int>" %a) {
  ret void
}

define i32 @main() {
  %a = alloca [3 x i32]
  store [3 x i32] [i32 1, i32 2, i32 3], [3 x i32]* %a
  %1 = getelementptr [3 x i32], [3 x i32]* %a, i32 0, i32 0
  %2 = insertvalue %"ArrayRef<int>" undef, i32* %1, 0
  %3 = insertvalue %"ArrayRef<int>" %2, i32 3, 1
  call void @_EN4main3fooE1aPAR_3int(%"ArrayRef<int>" %3)
  call void @_EN4main3bazE1bPA3_3int([3 x i32]* %a)
  ret i32 0
}

define void @_EN4main3bazE1bPA3_3int([3 x i32]* %b) {
  ret void
}

declare i8* @malloc(i64)

declare i32 @puts(i8*)

declare void @abort()

declare void @free(i8*)
