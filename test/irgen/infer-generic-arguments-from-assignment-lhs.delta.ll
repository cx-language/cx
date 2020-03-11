
%"List<int>" = type { i32*, i32, i32 }
%"RangeIterator<int>" = type { i32, i32 }
%"Range<int>" = type { i32, i32 }

@0 = private unnamed_addr constant [39 x i8] c"Unwrap failed at allocate.delta:36:62\0A\00", align 1

define i32 @main() {
  %i = alloca %"List<int>"
  %j = alloca %"List<int>"
  %1 = alloca %"List<int>"
  call void @_EN3std4ListI3intE4initE8capacity3int(%"List<int>"* %i, i32 10)
  call void @_EN3std4ListI3intE4initE(%"List<int>"* %j)
  call void @_EN3std4ListI3intE6deinitE(%"List<int>"* %i)
  call void @_EN3std4ListI3intE4initE(%"List<int>"* %1)
  %.load = load %"List<int>", %"List<int>"* %1
  store %"List<int>" %.load, %"List<int>"* %i
  call void @_EN3std4ListI3intE6deinitE(%"List<int>"* %j)
  call void @_EN3std4ListI3intE6deinitE(%"List<int>"* %i)
  ret i32 0
}

define void @_EN3std4ListI3intE6deinitE(%"List<int>"* %this) {
  %capacity = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 2
  %capacity.load = load i32, i32* %capacity
  %1 = icmp ne i32 %capacity.load, 0
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  %buffer = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 0
  %buffer.load = load i32*, i32** %buffer
  call void @_EN3std10deallocateIAU_3intEEPAU_3int(i32* %buffer.load)
  br label %if.end

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  ret void
}

define void @_EN3std4ListI3intE4initE8capacity3int(%"List<int>"* %this, i32 %capacity) {
  call void @_EN3std4ListI3intE4initE(%"List<int>"* %this)
  call void @_EN3std4ListI3intE7reserveE3int(%"List<int>"* %this, i32 %capacity)
  ret void
}

define void @_EN3std4ListI3intE4initE(%"List<int>"* %this) {
  %size = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 1
  store i32 0, i32* %size
  %capacity = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 2
  store i32 0, i32* %capacity
  ret void
}

define void @_EN3std4ListI3intE7reserveE3int(%"List<int>"* %this, i32 %minimumCapacity) {
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
  %3 = call i32* @_EN3std13allocateArrayI3intEE3int(i32 %minimumCapacity)
  store i32* %3, i32** %newBuffer
  %size = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size
  call void @_EN3std5RangeI3intE4initE3int3int(%"Range<int>"* %1, i32 0, i32 %size.load)
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
  call void @_EN3std10deallocateIAU_3intEEPAU_3int(i32* %buffer6.load)
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

define void @_EN3std10deallocateIAU_3intEEPAU_3int(i32* %allocation) {
  %1 = bitcast i32* %allocation to i8*
  call void @free(i8* %1)
  ret void
}

declare void @free(i8*)

define i32* @_EN3std13allocateArrayI3intEE3int(i32 %size) {
  %1 = sext i32 %size to i64
  %2 = mul i64 ptrtoint (i32* getelementptr (i32, i32* null, i32 1) to i64), %1
  %3 = call i8* @malloc(i64 %2)
  %assert.condition = icmp eq i8* %3, null
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %0
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([39 x i8], [39 x i8]* @0, i32 0, i32 0))
  unreachable

assert.success:                                   ; preds = %0
  %4 = bitcast i8* %3 to i32*
  ret i32* %4
}

declare %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(%"Range<int>"*)

declare void @_EN3std5RangeI3intE4initE3int3int(%"Range<int>"*, i32, i32)

declare i1 @_EN3std13RangeIteratorI3intE8hasValueE(%"RangeIterator<int>"*)

declare i32 @_EN3std13RangeIteratorI3intE5valueE(%"RangeIterator<int>"*)

declare void @_EN3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"*)

declare i8* @malloc(i64)

declare void @_EN3std10assertFailEP4char(i8*)
