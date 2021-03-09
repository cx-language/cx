
%"List<int>" = type { i32*, i32, i32 }
%"ArrayIterator<int>" = type { i32*, i32* }
%"ArrayRef<int>" = type { i32*, i32 }
%"RangeIterator<int>" = type { i32, i32 }
%"Range<int>" = type { i32, i32 }

@0 = private unnamed_addr constant [39 x i8] c"Unwrap failed at allocate.delta:36:61\0A\00", align 1

define i32 @main() {
  %i = alloca %"List<int>", align 8
  %j = alloca %"List<int>", align 8
  %1 = alloca %"List<int>", align 8
  call void @_EN3std4ListI3intE4initE8capacity3int(%"List<int>"* %i, i32 10)
  call void @_EN3std4ListI3intE4initE(%"List<int>"* %j)
  call void @_EN3std4ListI3intE6deinitE(%"List<int>"* %i)
  call void @_EN3std4ListI3intE4initE(%"List<int>"* %1)
  %.load = load %"List<int>", %"List<int>"* %1, align 8
  store %"List<int>" %.load, %"List<int>"* %i, align 8
  call void @_EN3std4ListI3intE6deinitE(%"List<int>"* %j)
  call void @_EN3std4ListI3intE6deinitE(%"List<int>"* %i)
  ret i32 0
}

define void @_EN3std4ListI3intE4initE8capacity3int(%"List<int>"* %this, i32 %capacity) {
  call void @_EN3std4ListI3intE4initE(%"List<int>"* %this)
  call void @_EN3std4ListI3intE7reserveE3int(%"List<int>"* %this, i32 %capacity)
  ret void
}

define void @_EN3std4ListI3intE4initE(%"List<int>"* %this) {
  %size = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 1
  store i32 0, i32* %size, align 4
  %capacity = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 2
  store i32 0, i32* %capacity, align 4
  ret void
}

define void @_EN3std4ListI3intE6deinitE(%"List<int>"* %this) {
  %__iterator = alloca %"ArrayIterator<int>", align 8
  %element = alloca i32*, align 8
  %capacity = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 2
  %capacity.load = load i32, i32* %capacity, align 4
  %1 = icmp ne i32 %capacity.load, 0
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  %2 = call %"ArrayIterator<int>" @_EN3std4ListI3intE8iteratorE(%"List<int>"* %this)
  store %"ArrayIterator<int>" %2, %"ArrayIterator<int>"* %__iterator, align 8
  br label %loop.condition

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %loop.end, %if.else
  ret void

loop.condition:                                   ; preds = %loop.increment, %if.then
  %3 = call i1 @_EN3std13ArrayIteratorI3intE8hasValueE(%"ArrayIterator<int>"* %__iterator)
  br i1 %3, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %4 = call i32* @_EN3std13ArrayIteratorI3intE5valueE(%"ArrayIterator<int>"* %__iterator)
  store i32* %4, i32** %element, align 8
  br label %loop.increment

loop.increment:                                   ; preds = %loop.body
  call void @_EN3std13ArrayIteratorI3intE9incrementE(%"ArrayIterator<int>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  %buffer = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 0
  %buffer.load = load i32*, i32** %buffer, align 8
  call void @_EN3std10deallocateIAU_3intEEAU_3int(i32* %buffer.load)
  br label %if.end
}

define %"ArrayIterator<int>" @_EN3std4ListI3intE8iteratorE(%"List<int>"* %this) {
  %1 = alloca %"ArrayIterator<int>", align 8
  %2 = alloca %"ArrayRef<int>", align 8
  call void @_EN3std8ArrayRefI3intE4initEP4ListI3intE(%"ArrayRef<int>"* %2, %"List<int>"* %this)
  %.load = load %"ArrayRef<int>", %"ArrayRef<int>"* %2, align 8
  call void @_EN3std13ArrayIteratorI3intE4initE8ArrayRefI3intE(%"ArrayIterator<int>"* %1, %"ArrayRef<int>" %.load)
  %.load1 = load %"ArrayIterator<int>", %"ArrayIterator<int>"* %1, align 8
  ret %"ArrayIterator<int>" %.load1
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

define void @_EN3std13ArrayIteratorI3intE9incrementE(%"ArrayIterator<int>"* %this) {
  %current = getelementptr inbounds %"ArrayIterator<int>", %"ArrayIterator<int>"* %this, i32 0, i32 0
  %current.load = load i32*, i32** %current, align 8
  %1 = getelementptr inbounds i32, i32* %current.load, i32 1
  store i32* %1, i32** %current, align 8
  ret void
}

define void @_EN3std10deallocateIAU_3intEEAU_3int(i32* %allocation) {
  %1 = bitcast i32* %allocation to i8*
  call void @free(i8* %1)
  ret void
}

define void @_EN3std4ListI3intE7reserveE3int(%"List<int>"* %this, i32 %minimumCapacity) {
  %newBuffer = alloca i32*, align 8
  %__iterator = alloca %"RangeIterator<int>", align 8
  %1 = alloca %"Range<int>", align 8
  %index = alloca i32, align 4
  %source = alloca i32*, align 8
  %target = alloca i32*, align 8
  %capacity = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 2
  %capacity.load = load i32, i32* %capacity, align 4
  %2 = icmp sgt i32 %minimumCapacity, %capacity.load
  br i1 %2, label %if.then, label %if.else

if.then:                                          ; preds = %0
  %3 = call i32* @_EN3std13allocateArrayI3intEE3int(i32 %minimumCapacity)
  store i32* %3, i32** %newBuffer, align 8
  %size = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  call void @_EN3std5RangeI3intE4initE3int3int(%"Range<int>"* %1, i32 0, i32 %size.load)
  %4 = call %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(%"Range<int>"* %1)
  store %"RangeIterator<int>" %4, %"RangeIterator<int>"* %__iterator, align 4
  br label %loop.condition

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.end8, %if.else
  ret void

loop.condition:                                   ; preds = %loop.increment, %if.then
  %5 = call i1 @_EN3std13RangeIteratorI3intE8hasValueE(%"RangeIterator<int>"* %__iterator)
  br i1 %5, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %6 = call i32 @_EN3std13RangeIteratorI3intE5valueE(%"RangeIterator<int>"* %__iterator)
  store i32 %6, i32* %index, align 4
  %buffer = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 0
  %buffer.load = load i32*, i32** %buffer, align 8
  %index.load = load i32, i32* %index, align 4
  %7 = getelementptr inbounds i32, i32* %buffer.load, i32 %index.load
  store i32* %7, i32** %source, align 8
  %newBuffer.load = load i32*, i32** %newBuffer, align 8
  %index.load1 = load i32, i32* %index, align 4
  %8 = getelementptr inbounds i32, i32* %newBuffer.load, i32 %index.load1
  store i32* %8, i32** %target, align 8
  %target.load = load i32*, i32** %target, align 8
  %source.load = load i32*, i32** %source, align 8
  %source.load.load = load i32, i32* %source.load, align 4
  store i32 %source.load.load, i32* %target.load, align 4
  br label %loop.increment

loop.increment:                                   ; preds = %loop.body
  call void @_EN3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"* %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  %capacity2 = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 2
  %capacity.load3 = load i32, i32* %capacity2, align 4
  %9 = icmp ne i32 %capacity.load3, 0
  br i1 %9, label %if.then4, label %if.else7

if.then4:                                         ; preds = %loop.end
  %buffer5 = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 0
  %buffer.load6 = load i32*, i32** %buffer5, align 8
  call void @_EN3std10deallocateIAU_3intEEAU_3int(i32* %buffer.load6)
  br label %if.end8

if.else7:                                         ; preds = %loop.end
  br label %if.end8

if.end8:                                          ; preds = %if.else7, %if.then4
  %buffer9 = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 0
  %newBuffer.load10 = load i32*, i32** %newBuffer, align 8
  store i32* %newBuffer.load10, i32** %buffer9, align 8
  %capacity11 = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 2
  store i32 %minimumCapacity, i32* %capacity11, align 4
  br label %if.end
}

define void @_EN3std8ArrayRefI3intE4initEP4ListI3intE(%"ArrayRef<int>"* %this, %"List<int>"* %list) {
  %data = getelementptr inbounds %"ArrayRef<int>", %"ArrayRef<int>"* %this, i32 0, i32 0
  %1 = call i32* @_EN3std4ListI3intE4dataE(%"List<int>"* %list)
  store i32* %1, i32** %data, align 8
  %size = getelementptr inbounds %"ArrayRef<int>", %"ArrayRef<int>"* %this, i32 0, i32 1
  %2 = call i32 @_EN3std4ListI3intE4sizeE(%"List<int>"* %list)
  store i32 %2, i32* %size, align 4
  ret void
}

define void @_EN3std13ArrayIteratorI3intE4initE8ArrayRefI3intE(%"ArrayIterator<int>"* %this, %"ArrayRef<int>" %array) {
  %1 = alloca %"ArrayRef<int>", align 8
  %2 = alloca %"ArrayRef<int>", align 8
  %3 = alloca %"ArrayRef<int>", align 8
  %current = getelementptr inbounds %"ArrayIterator<int>", %"ArrayIterator<int>"* %this, i32 0, i32 0
  store %"ArrayRef<int>" %array, %"ArrayRef<int>"* %1, align 8
  %4 = call i32* @_EN3std8ArrayRefI3intE4dataE(%"ArrayRef<int>"* %1)
  store i32* %4, i32** %current, align 8
  %end = getelementptr inbounds %"ArrayIterator<int>", %"ArrayIterator<int>"* %this, i32 0, i32 1
  store %"ArrayRef<int>" %array, %"ArrayRef<int>"* %2, align 8
  %5 = call i32* @_EN3std8ArrayRefI3intE4dataE(%"ArrayRef<int>"* %2)
  store %"ArrayRef<int>" %array, %"ArrayRef<int>"* %3, align 8
  %6 = call i32 @_EN3std8ArrayRefI3intE4sizeE(%"ArrayRef<int>"* %3)
  %7 = getelementptr inbounds i32, i32* %5, i32 %6
  store i32* %7, i32** %end, align 8
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

declare void @_EN3std5RangeI3intE4initE3int3int(%"Range<int>"*, i32, i32)

declare %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(%"Range<int>"*)

declare i1 @_EN3std13RangeIteratorI3intE8hasValueE(%"RangeIterator<int>"*)

declare i32 @_EN3std13RangeIteratorI3intE5valueE(%"RangeIterator<int>"*)

declare void @_EN3std13RangeIteratorI3intE9incrementE(%"RangeIterator<int>"*)

define i32* @_EN3std8ArrayRefI3intE4dataE(%"ArrayRef<int>"* %this) {
  %data = getelementptr inbounds %"ArrayRef<int>", %"ArrayRef<int>"* %this, i32 0, i32 0
  %data.load = load i32*, i32** %data, align 8
  ret i32* %data.load
}

define i32 @_EN3std8ArrayRefI3intE4sizeE(%"ArrayRef<int>"* %this) {
  %size = getelementptr inbounds %"ArrayRef<int>", %"ArrayRef<int>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  ret i32 %size.load
}

define i32* @_EN3std4ListI3intE4dataE(%"List<int>"* %this) {
  %buffer = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 0
  %buffer.load = load i32*, i32** %buffer, align 8
  ret i32* %buffer.load
}

define i32 @_EN3std4ListI3intE4sizeE(%"List<int>"* %this) {
  %size = getelementptr inbounds %"List<int>", %"List<int>"* %this, i32 0, i32 1
  %size.load = load i32, i32* %size, align 4
  ret i32 %size.load
}

declare i8* @malloc(i64)

declare void @_EN3std10assertFailEP4char(i8*)
