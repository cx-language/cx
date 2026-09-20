
%"Box<Box<int>>" = type { ptr }
%"Box<int>" = type { ptr }

@0 = private unnamed_addr constant [35 x i8] c"Unwrap failed at allocate.cx:9:54\0A\00", align 1
@1 = private unnamed_addr constant [35 x i8] c"Unwrap failed at allocate.cx:9:54\0A\00", align 1

define i32 @main() {
  %p = alloca %"Box<Box<int>>", align 8
  %1 = alloca %"Box<int>", align 8
  %q = alloca %"Box<int>", align 8
  %r = alloca %"Box<int>", align 8
  call void @_EN3std3BoxI3intE4initE3int(ptr %1, i32 42)
  %.load = load %"Box<int>", ptr %1, align 8
  call void @_EN3std3BoxI3BoxI3intEE4initE3BoxI3intE(ptr %p, %"Box<int>" %.load)
  call void @_EN3std3BoxI3intE4initE3int(ptr %q, i32 42)
  call void @_EN3std3BoxI3intE4initE3int(ptr %r, i32 42)
  %2 = call ptr @_EN3std3BoxI3intE3getE(ptr %q)
  store i32 0, ptr %2, align 4
  %3 = call ptr @_EN3std3BoxI3intE3getE(ptr %r)
  call void @_EN3std3BoxI3intE6deinitE(ptr %q)
  %r.load = load %"Box<int>", ptr %r, align 8
  store %"Box<int>" %r.load, ptr %q, align 8
  call void @_EN3std3BoxI3intE6deinitE(ptr %q)
  call void @_EN3std3BoxI3BoxI3intEE6deinitE(ptr %p)
  ret i32 0
}

define void @_EN3std3BoxI3intE4initE3int(ptr %this, i32 %value) {
  %value1 = alloca i32, align 4
  store i32 %value, ptr %value1, align 4
  %pointer = getelementptr inbounds %"Box<int>", ptr %this, i32 0, i32 0
  %value.load = load i32, ptr %value1, align 4
  %1 = call ptr @_EN3std8allocateI3intEE3int(i32 %value.load)
  store ptr %1, ptr %pointer, align 8
  ret void
}

define void @_EN3std3BoxI3BoxI3intEE4initE3BoxI3intE(ptr %this, %"Box<int>" %value) {
  %value1 = alloca %"Box<int>", align 8
  store %"Box<int>" %value, ptr %value1, align 8
  %pointer = getelementptr inbounds %"Box<Box<int>>", ptr %this, i32 0, i32 0
  %value.load = load %"Box<int>", ptr %value1, align 8
  %1 = call ptr @_EN3std8allocateI3BoxI3intEEE3BoxI3intE(%"Box<int>" %value.load)
  store ptr %1, ptr %pointer, align 8
  ret void
}

define ptr @_EN3std3BoxI3intE3getE(ptr %this) {
  %pointer = getelementptr inbounds %"Box<int>", ptr %this, i32 0, i32 0
  %pointer.load = load ptr, ptr %pointer, align 8
  ret ptr %pointer.load
}

define void @_EN3std3BoxI3intE6deinitE(ptr %this) {
  %pointer = getelementptr inbounds %"Box<int>", ptr %this, i32 0, i32 0
  %pointer.load = load ptr, ptr %pointer, align 8
  call void @_EN3std10deallocateIP3intEEP3int(ptr %pointer.load)
  ret void
}

define void @_EN3std3BoxI3BoxI3intEE6deinitE(ptr %this) {
  %pointer = getelementptr inbounds %"Box<Box<int>>", ptr %this, i32 0, i32 0
  %pointer.load = load ptr, ptr %pointer, align 8
  call void @_EN3std10deallocateIP3BoxI3intEEEP3BoxI3intE(ptr %pointer.load)
  ret void
}

define void @_EN3std10deallocateIP3BoxI3intEEEP3BoxI3intE(ptr %allocation) {
  %allocation1 = alloca ptr, align 8
  store ptr %allocation, ptr %allocation1, align 8
  %allocation.load = load ptr, ptr %allocation1, align 8
  call void @free(ptr %allocation.load)
  ret void
}

define ptr @_EN3std8allocateI3BoxI3intEEE3BoxI3intE(%"Box<int>" %value) {
  %value1 = alloca %"Box<int>", align 8
  %allocation = alloca ptr, align 8
  store %"Box<int>" %value, ptr %value1, align 8
  %1 = call ptr @malloc(i64 ptrtoint (ptr getelementptr (%"Box<int>", ptr null, i32 1) to i64))
  %assert.condition = icmp eq ptr %1, null
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %0
  call void @_EN3std10assertFailEP4char(ptr @0)
  unreachable

assert.success:                                   ; preds = %0
  store ptr %1, ptr %allocation, align 8
  %allocation.load = load ptr, ptr %allocation, align 8
  %value.load = load %"Box<int>", ptr %value1, align 8
  store %"Box<int>" %value.load, ptr %allocation.load, align 8
  %allocation.load2 = load ptr, ptr %allocation, align 8
  ret ptr %allocation.load2
}

define ptr @_EN3std8allocateI3intEE3int(i32 %value) {
  %value1 = alloca i32, align 4
  %allocation = alloca ptr, align 8
  store i32 %value, ptr %value1, align 4
  %1 = call ptr @malloc(i64 4)
  %assert.condition = icmp eq ptr %1, null
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %0
  call void @_EN3std10assertFailEP4char(ptr @1)
  unreachable

assert.success:                                   ; preds = %0
  store ptr %1, ptr %allocation, align 8
  %allocation.load = load ptr, ptr %allocation, align 8
  %value.load = load i32, ptr %value1, align 4
  store i32 %value.load, ptr %allocation.load, align 4
  %allocation.load2 = load ptr, ptr %allocation, align 8
  ret ptr %allocation.load2
}

define void @_EN3std10deallocateIP3intEEP3int(ptr %allocation) {
  %allocation1 = alloca ptr, align 8
  store ptr %allocation, ptr %allocation1, align 8
  %allocation.load = load ptr, ptr %allocation1, align 8
  call void @free(ptr %allocation.load)
  ret void
}

declare void @free(ptr)

declare ptr @malloc(i64)

declare void @_EN3std10assertFailEP4char(ptr)
