
%"List<int>" = type { ptr, i32, i32 }
%"ArrayIterator<int>" = type { ptr, ptr }
%"ArrayRef<int>" = type { ptr, i32 }
%"RangeIterator<int>" = type { i32, i32 }
%"Range<int>" = type { i32, i32 }

@0 = private unnamed_addr constant [36 x i8] c"Unwrap failed at allocate.cx:36:64\0A\00", align 1

define i32 @main() {
  %i = alloca %"List<int>", align 8
  %j = alloca %"List<int>", align 8
  %1 = alloca %"List<int>", align 8
  call void @_EN3std4ListI3intE4initE8capacity3int(ptr %i, i32 10)
  call void @_EN3std4ListI3intE4initE(ptr %j)
  call void @_EN3std4ListI3intE6deinitE(ptr %i)
  call void @_EN3std4ListI3intE4initE(ptr %1)
  %.load = load %"List<int>", ptr %1, align 8
  store %"List<int>" %.load, ptr %i, align 8
  call void @_EN3std4ListI3intE6deinitE(ptr %j)
  call void @_EN3std4ListI3intE6deinitE(ptr %i)
  ret i32 0
}

define void @_EN3std4ListI3intE4initE8capacity3int(ptr %this, i32 %capacity) {
  call void @_EN3std4ListI3intE4initE(ptr %this)
  call void @_EN3std4ListI3intE7reserveE3int(ptr %this, i32 %capacity)
  ret void
}

define void @_EN3std4ListI3intE4initE(ptr %this) {
  %size = getelementptr inbounds %"List<int>", ptr %this, i32 0, i32 1
  store i32 0, ptr %size, align 4
  %capacity = getelementptr inbounds %"List<int>", ptr %this, i32 0, i32 2
  store i32 0, ptr %capacity, align 4
  ret void
}

define void @_EN3std4ListI3intE6deinitE(ptr %this) {
  %__iterator = alloca %"ArrayIterator<int>", align 8
  %element = alloca ptr, align 8
  %capacity = getelementptr inbounds %"List<int>", ptr %this, i32 0, i32 2
  %capacity.load = load i32, ptr %capacity, align 4
  %1 = icmp ne i32 %capacity.load, 0
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  %2 = call %"ArrayIterator<int>" @_EN3std4ListI3intE8iteratorE(ptr %this)
  store %"ArrayIterator<int>" %2, ptr %__iterator, align 8
  br label %loop.condition

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %loop.end, %if.else
  ret void

loop.condition:                                   ; preds = %loop.increment, %if.then
  %3 = call i1 @_EN3std13ArrayIteratorI3intE8hasValueE(ptr %__iterator)
  br i1 %3, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %4 = call ptr @_EN3std13ArrayIteratorI3intE5valueE(ptr %__iterator)
  store ptr %4, ptr %element, align 8
  br label %loop.increment

loop.increment:                                   ; preds = %loop.body
  call void @_EN3std13ArrayIteratorI3intE9incrementE(ptr %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  %buffer = getelementptr inbounds %"List<int>", ptr %this, i32 0, i32 0
  %buffer.load = load ptr, ptr %buffer, align 8
  call void @_EN3std10deallocateIAU_3intEEAU_3int(ptr %buffer.load)
  br label %if.end
}

define %"ArrayIterator<int>" @_EN3std4ListI3intE8iteratorE(ptr %this) {
  %1 = alloca %"ArrayIterator<int>", align 8
  %2 = alloca %"ArrayRef<int>", align 8
  call void @_EN3std8ArrayRefI3intE4initEP4ListI3intE(ptr %2, ptr %this)
  %.load = load %"ArrayRef<int>", ptr %2, align 8
  call void @_EN3std13ArrayIteratorI3intE4initE8ArrayRefI3intE(ptr %1, %"ArrayRef<int>" %.load)
  %.load1 = load %"ArrayIterator<int>", ptr %1, align 8
  ret %"ArrayIterator<int>" %.load1
}

define i1 @_EN3std13ArrayIteratorI3intE8hasValueE(ptr %this) {
  %current = getelementptr inbounds %"ArrayIterator<int>", ptr %this, i32 0, i32 0
  %current.load = load ptr, ptr %current, align 8
  %end = getelementptr inbounds %"ArrayIterator<int>", ptr %this, i32 0, i32 1
  %end.load = load ptr, ptr %end, align 8
  %1 = icmp ne ptr %current.load, %end.load
  ret i1 %1
}

define ptr @_EN3std13ArrayIteratorI3intE5valueE(ptr %this) {
  %current = getelementptr inbounds %"ArrayIterator<int>", ptr %this, i32 0, i32 0
  %current.load = load ptr, ptr %current, align 8
  ret ptr %current.load
}

define void @_EN3std13ArrayIteratorI3intE9incrementE(ptr %this) {
  %current = getelementptr inbounds %"ArrayIterator<int>", ptr %this, i32 0, i32 0
  %current.load = load ptr, ptr %current, align 8
  %1 = getelementptr inbounds i32, ptr %current.load, i32 1
  store ptr %1, ptr %current, align 8
  ret void
}

define void @_EN3std10deallocateIAU_3intEEAU_3int(ptr %allocation) {
  call void @free(ptr %allocation)
  ret void
}

define void @_EN3std4ListI3intE7reserveE3int(ptr %this, i32 %minimumCapacity) {
  %newBuffer = alloca ptr, align 8
  %__iterator = alloca %"RangeIterator<int>", align 8
  %1 = alloca %"Range<int>", align 8
  %index = alloca i32, align 4
  %source = alloca ptr, align 8
  %target = alloca ptr, align 8
  %capacity = getelementptr inbounds %"List<int>", ptr %this, i32 0, i32 2
  %capacity.load = load i32, ptr %capacity, align 4
  %2 = icmp sgt i32 %minimumCapacity, %capacity.load
  br i1 %2, label %if.then, label %if.else

if.then:                                          ; preds = %0
  %3 = call ptr @_EN3std13allocateArrayI3intEE3int(i32 %minimumCapacity)
  store ptr %3, ptr %newBuffer, align 8
  %size = getelementptr inbounds %"List<int>", ptr %this, i32 0, i32 1
  %size.load = load i32, ptr %size, align 4
  call void @_EN3std5RangeI3intE4initE3int3int(ptr %1, i32 0, i32 %size.load)
  %4 = call %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(ptr %1)
  store %"RangeIterator<int>" %4, ptr %__iterator, align 4
  br label %loop.condition

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.end8, %if.else
  ret void

loop.condition:                                   ; preds = %loop.increment, %if.then
  %5 = call i1 @_EN3std13RangeIteratorI3intE8hasValueE(ptr %__iterator)
  br i1 %5, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %6 = call i32 @_EN3std13RangeIteratorI3intE5valueE(ptr %__iterator)
  store i32 %6, ptr %index, align 4
  %buffer = getelementptr inbounds %"List<int>", ptr %this, i32 0, i32 0
  %buffer.load = load ptr, ptr %buffer, align 8
  %index.load = load i32, ptr %index, align 4
  %7 = getelementptr inbounds i32, ptr %buffer.load, i32 %index.load
  store ptr %7, ptr %source, align 8
  %newBuffer.load = load ptr, ptr %newBuffer, align 8
  %index.load1 = load i32, ptr %index, align 4
  %8 = getelementptr inbounds i32, ptr %newBuffer.load, i32 %index.load1
  store ptr %8, ptr %target, align 8
  %target.load = load ptr, ptr %target, align 8
  %source.load = load ptr, ptr %source, align 8
  %source.load.load = load i32, ptr %source.load, align 4
  store i32 %source.load.load, ptr %target.load, align 4
  br label %loop.increment

loop.increment:                                   ; preds = %loop.body
  call void @_EN3std13RangeIteratorI3intE9incrementE(ptr %__iterator)
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  %capacity2 = getelementptr inbounds %"List<int>", ptr %this, i32 0, i32 2
  %capacity.load3 = load i32, ptr %capacity2, align 4
  %9 = icmp ne i32 %capacity.load3, 0
  br i1 %9, label %if.then4, label %if.else7

if.then4:                                         ; preds = %loop.end
  %buffer5 = getelementptr inbounds %"List<int>", ptr %this, i32 0, i32 0
  %buffer.load6 = load ptr, ptr %buffer5, align 8
  call void @_EN3std10deallocateIAU_3intEEAU_3int(ptr %buffer.load6)
  br label %if.end8

if.else7:                                         ; preds = %loop.end
  br label %if.end8

if.end8:                                          ; preds = %if.else7, %if.then4
  %buffer9 = getelementptr inbounds %"List<int>", ptr %this, i32 0, i32 0
  %newBuffer.load10 = load ptr, ptr %newBuffer, align 8
  store ptr %newBuffer.load10, ptr %buffer9, align 8
  %capacity11 = getelementptr inbounds %"List<int>", ptr %this, i32 0, i32 2
  store i32 %minimumCapacity, ptr %capacity11, align 4
  br label %if.end
}

define void @_EN3std8ArrayRefI3intE4initEP4ListI3intE(ptr %this, ptr %list) {
  %data = getelementptr inbounds %"ArrayRef<int>", ptr %this, i32 0, i32 0
  %1 = call ptr @_EN3std4ListI3intE4dataE(ptr %list)
  store ptr %1, ptr %data, align 8
  %size = getelementptr inbounds %"ArrayRef<int>", ptr %this, i32 0, i32 1
  %2 = call i32 @_EN3std4ListI3intE4sizeE(ptr %list)
  store i32 %2, ptr %size, align 4
  ret void
}

define void @_EN3std13ArrayIteratorI3intE4initE8ArrayRefI3intE(ptr %this, %"ArrayRef<int>" %array) {
  %1 = alloca %"ArrayRef<int>", align 8
  %2 = alloca %"ArrayRef<int>", align 8
  %3 = alloca %"ArrayRef<int>", align 8
  %current = getelementptr inbounds %"ArrayIterator<int>", ptr %this, i32 0, i32 0
  store %"ArrayRef<int>" %array, ptr %1, align 8
  %4 = call ptr @_EN3std8ArrayRefI3intE4dataE(ptr %1)
  store ptr %4, ptr %current, align 8
  %end = getelementptr inbounds %"ArrayIterator<int>", ptr %this, i32 0, i32 1
  store %"ArrayRef<int>" %array, ptr %2, align 8
  %5 = call ptr @_EN3std8ArrayRefI3intE4dataE(ptr %2)
  store %"ArrayRef<int>" %array, ptr %3, align 8
  %6 = call i32 @_EN3std8ArrayRefI3intE4sizeE(ptr %3)
  %7 = getelementptr inbounds i32, ptr %5, i32 %6
  store ptr %7, ptr %end, align 8
  ret void
}

declare void @free(ptr)

define ptr @_EN3std13allocateArrayI3intEE3int(i32 %size) {
  %1 = sext i32 %size to i64
  %2 = mul i64 ptrtoint (ptr getelementptr (i32, ptr null, i32 1) to i64), %1
  %3 = call ptr @malloc(i64 %2)
  %assert.condition = icmp eq ptr %3, null
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %0
  call void @_EN3std10assertFailEP4char(ptr @0)
  unreachable

assert.success:                                   ; preds = %0
  ret ptr %3
}

declare void @_EN3std5RangeI3intE4initE3int3int(ptr, i32, i32)

declare %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(ptr)

declare i1 @_EN3std13RangeIteratorI3intE8hasValueE(ptr)

declare i32 @_EN3std13RangeIteratorI3intE5valueE(ptr)

declare void @_EN3std13RangeIteratorI3intE9incrementE(ptr)

define ptr @_EN3std8ArrayRefI3intE4dataE(ptr %this) {
  %data = getelementptr inbounds %"ArrayRef<int>", ptr %this, i32 0, i32 0
  %data.load = load ptr, ptr %data, align 8
  ret ptr %data.load
}

define i32 @_EN3std8ArrayRefI3intE4sizeE(ptr %this) {
  %size = getelementptr inbounds %"ArrayRef<int>", ptr %this, i32 0, i32 1
  %size.load = load i32, ptr %size, align 4
  ret i32 %size.load
}

define ptr @_EN3std4ListI3intE4dataE(ptr %this) {
  %buffer = getelementptr inbounds %"List<int>", ptr %this, i32 0, i32 0
  %buffer.load = load ptr, ptr %buffer, align 8
  ret ptr %buffer.load
}

define i32 @_EN3std4ListI3intE4sizeE(ptr %this) {
  %size = getelementptr inbounds %"List<int>", ptr %this, i32 0, i32 1
  %size.load = load i32, ptr %size, align 4
  ret i32 %size.load
}

declare ptr @malloc(i64)

declare void @_EN3std10assertFailEP4char(ptr)
