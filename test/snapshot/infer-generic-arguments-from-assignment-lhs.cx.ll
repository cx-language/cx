
%"List<int>" = type { ptr, i32, i32 }
%"ArrayIterator<int>" = type { ptr, ptr }
%"ArrayRef<int>" = type { ptr, i32 }
%"RangeIterator<int>" = type { i32, i32 }
%"Range<int>" = type { i32, i32 }

@0 = private unnamed_addr constant [36 x i8] c"Unwrap failed at allocate.cx:36:61\0A\00", align 1

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
  %capacity1 = alloca i32, align 4
  store i32 %capacity, ptr %capacity1, align 4
  call void @_EN3std4ListI3intE4initE(ptr %this)
  %capacity.load = load i32, ptr %capacity1, align 4
  call void @_EN3std4ListI3intE7reserveE3int(ptr %this, i32 %capacity.load)
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
  %allocation1 = alloca ptr, align 8
  store ptr %allocation, ptr %allocation1, align 8
  %allocation.load = load ptr, ptr %allocation1, align 8
  call void @free(ptr %allocation.load)
  ret void
}

define void @_EN3std4ListI3intE7reserveE3int(ptr %this, i32 %minimumCapacity) {
  %minimumCapacity1 = alloca i32, align 4
  %newBuffer = alloca ptr, align 8
  %__iterator = alloca %"RangeIterator<int>", align 8
  %1 = alloca %"Range<int>", align 8
  %index = alloca i32, align 4
  %source = alloca ptr, align 8
  %target = alloca ptr, align 8
  store i32 %minimumCapacity, ptr %minimumCapacity1, align 4
  %minimumCapacity.load = load i32, ptr %minimumCapacity1, align 4
  %capacity = getelementptr inbounds %"List<int>", ptr %this, i32 0, i32 2
  %capacity.load = load i32, ptr %capacity, align 4
  %2 = icmp sgt i32 %minimumCapacity.load, %capacity.load
  br i1 %2, label %if.then, label %if.else

if.then:                                          ; preds = %0
  %minimumCapacity.load2 = load i32, ptr %minimumCapacity1, align 4
  %3 = call ptr @_EN3std13allocateArrayI3intEE3int(i32 %minimumCapacity.load2)
  store ptr %3, ptr %newBuffer, align 8
  %size = getelementptr inbounds %"List<int>", ptr %this, i32 0, i32 1
  %size.load = load i32, ptr %size, align 4
  call void @_EN3std5RangeI3intE4initE3int3int(ptr %1, i32 0, i32 %size.load)
  %4 = call %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(ptr %1)
  store %"RangeIterator<int>" %4, ptr %__iterator, align 4
  br label %loop.condition

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.end10, %if.else
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
  %index.load3 = load i32, ptr %index, align 4
  %8 = getelementptr inbounds i32, ptr %newBuffer.load, i32 %index.load3
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
  %capacity4 = getelementptr inbounds %"List<int>", ptr %this, i32 0, i32 2
  %capacity.load5 = load i32, ptr %capacity4, align 4
  %9 = icmp ne i32 %capacity.load5, 0
  br i1 %9, label %if.then6, label %if.else9

if.then6:                                         ; preds = %loop.end
  %buffer7 = getelementptr inbounds %"List<int>", ptr %this, i32 0, i32 0
  %buffer.load8 = load ptr, ptr %buffer7, align 8
  call void @_EN3std10deallocateIAU_3intEEAU_3int(ptr %buffer.load8)
  br label %if.end10

if.else9:                                         ; preds = %loop.end
  br label %if.end10

if.end10:                                         ; preds = %if.else9, %if.then6
  %buffer11 = getelementptr inbounds %"List<int>", ptr %this, i32 0, i32 0
  %newBuffer.load12 = load ptr, ptr %newBuffer, align 8
  store ptr %newBuffer.load12, ptr %buffer11, align 8
  %capacity13 = getelementptr inbounds %"List<int>", ptr %this, i32 0, i32 2
  %minimumCapacity.load14 = load i32, ptr %minimumCapacity1, align 4
  store i32 %minimumCapacity.load14, ptr %capacity13, align 4
  br label %if.end
}

define void @_EN3std8ArrayRefI3intE4initEP4ListI3intE(ptr %this, ptr %list) {
  %list1 = alloca ptr, align 8
  store ptr %list, ptr %list1, align 8
  %data = getelementptr inbounds %"ArrayRef<int>", ptr %this, i32 0, i32 0
  %list.load = load ptr, ptr %list1, align 8
  %1 = call ptr @_EN3std4ListI3intE4dataE(ptr %list.load)
  store ptr %1, ptr %data, align 8
  %size = getelementptr inbounds %"ArrayRef<int>", ptr %this, i32 0, i32 1
  %list.load2 = load ptr, ptr %list1, align 8
  %2 = call i32 @_EN3std4ListI3intE4sizeE(ptr %list.load2)
  store i32 %2, ptr %size, align 4
  ret void
}

define void @_EN3std13ArrayIteratorI3intE4initE8ArrayRefI3intE(ptr %this, %"ArrayRef<int>" %array) {
  %array1 = alloca %"ArrayRef<int>", align 8
  store %"ArrayRef<int>" %array, ptr %array1, align 8
  %current = getelementptr inbounds %"ArrayIterator<int>", ptr %this, i32 0, i32 0
  %1 = call ptr @_EN3std8ArrayRefI3intE4dataE(ptr %array1)
  store ptr %1, ptr %current, align 8
  %end = getelementptr inbounds %"ArrayIterator<int>", ptr %this, i32 0, i32 1
  %2 = call ptr @_EN3std8ArrayRefI3intE4dataE(ptr %array1)
  %3 = call i32 @_EN3std8ArrayRefI3intE4sizeE(ptr %array1)
  %4 = getelementptr inbounds i32, ptr %2, i32 %3
  store ptr %4, ptr %end, align 8
  ret void
}

declare void @free(ptr)

define ptr @_EN3std13allocateArrayI3intEE3int(i32 %size) {
  %size1 = alloca i32, align 4
  store i32 %size, ptr %size1, align 4
  %size.load = load i32, ptr %size1, align 4
  %1 = sext i32 %size.load to i64
  %2 = mul i64 4, %1
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
