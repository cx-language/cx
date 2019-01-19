
%"UniquePtr<UniquePtr<int>>" = type { %"UniquePtr<int>"* }
%"UniquePtr<int>" = type { i32* }

@0 = private unnamed_addr constant [37 x i8] c"Unwrap failed at allocate.delta:9:62\00"
@1 = private unnamed_addr constant [37 x i8] c"Unwrap failed at allocate.delta:9:62\00"

define i32 @main() {
  %p = alloca %"UniquePtr<UniquePtr<int>>"
  %1 = alloca %"UniquePtr<int>"
  %q = alloca %"UniquePtr<int>"
  %r = alloca %"UniquePtr<int>"
  call void @_ENM3std9UniquePtrI3intE4initE5value3int(%"UniquePtr<int>"* %1, i32 42)
  %2 = load %"UniquePtr<int>", %"UniquePtr<int>"* %1
  call void @_ENM3std9UniquePtrI9UniquePtrI3intEE4initE5value9UniquePtrI3intE(%"UniquePtr<UniquePtr<int>>"* %p, %"UniquePtr<int>" %2)
  call void @_ENM3std9UniquePtrI3intE4initE5value3int(%"UniquePtr<int>"* %q, i32 42)
  call void @_ENM3std9UniquePtrI3intE4initE5value3int(%"UniquePtr<int>"* %r, i32 42)
  %3 = call i32* @_ENM3std9UniquePtrI3intE3getE(%"UniquePtr<int>"* %q)
  store i32 0, i32* %3
  %4 = call i32* @_ENM3std9UniquePtrI3intE3getE(%"UniquePtr<int>"* %r)
  call void @_ENM3std9UniquePtrI3intE6deinitE(%"UniquePtr<int>"* %q)
  %r1 = load %"UniquePtr<int>", %"UniquePtr<int>"* %r
  store %"UniquePtr<int>" %r1, %"UniquePtr<int>"* %q
  call void @_ENM3std9UniquePtrI3intE6deinitE(%"UniquePtr<int>"* %q)
  call void @_ENM3std9UniquePtrI9UniquePtrI3intEE6deinitE(%"UniquePtr<UniquePtr<int>>"* %p)
  ret i32 0
}

define void @_ENM3std9UniquePtrI3intE4initE5value3int(%"UniquePtr<int>"* %this, i32 %value) {
  %pointer = getelementptr inbounds %"UniquePtr<int>", %"UniquePtr<int>"* %this, i32 0, i32 0
  %1 = call i32* @_EN3std8allocateI3intEE5value3int(i32 %value)
  store i32* %1, i32** %pointer
  ret void
}

define i32* @_EN3std8allocateI3intEE5value3int(i32 %value) {
  %allocation = alloca i32*
  %1 = call i8* @malloc(i64 ptrtoint (i32* getelementptr (i32, i32* null, i32 1) to i64))
  %assert.condition = icmp eq i8* %1, null
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %0
  %2 = call i32 @puts(i8* getelementptr inbounds ([37 x i8], [37 x i8]* @1, i32 0, i32 0))
  call void @abort()
  unreachable

assert.success:                                   ; preds = %0
  %3 = bitcast i8* %1 to i32*
  store i32* %3, i32** %allocation
  %allocation1 = load i32*, i32** %allocation
  store i32 %value, i32* %allocation1
  %allocation2 = load i32*, i32** %allocation
  ret i32* %allocation2
}

define void @_ENM3std9UniquePtrI3intE4initE7pointerPM3int(%"UniquePtr<int>"* %this, i32* %pointer) {
  %pointer1 = getelementptr inbounds %"UniquePtr<int>", %"UniquePtr<int>"* %this, i32 0, i32 0
  store i32* %pointer, i32** %pointer1
  ret void
}

define void @_ENM3std9UniquePtrI3intE6deinitE(%"UniquePtr<int>"* %this) {
  %pointer = getelementptr inbounds %"UniquePtr<int>", %"UniquePtr<int>"* %this, i32 0, i32 0
  %pointer1 = load i32*, i32** %pointer
  call void @_EN3std10deallocateI3intEE10allocationPM3int(i32* %pointer1)
  ret void
}

define void @_EN3std10deallocateI3intEE10allocationPM3int(i32* %allocation) {
  %1 = bitcast i32* %allocation to i8*
  call void @free(i8* %1)
  ret void
}

define i32* @_EN3std9UniquePtrI3intE3getE(%"UniquePtr<int>"* %this) {
  %pointer = getelementptr inbounds %"UniquePtr<int>", %"UniquePtr<int>"* %this, i32 0, i32 0
  %pointer1 = load i32*, i32** %pointer
  ret i32* %pointer1
}

define i32* @_ENM3std9UniquePtrI3intE3getE(%"UniquePtr<int>"* %this) {
  %pointer = getelementptr inbounds %"UniquePtr<int>", %"UniquePtr<int>"* %this, i32 0, i32 0
  %pointer1 = load i32*, i32** %pointer
  ret i32* %pointer1
}

define void @_ENM3std9UniquePtrI9UniquePtrI3intEE4initE5value9UniquePtrI3intE(%"UniquePtr<UniquePtr<int>>"* %this, %"UniquePtr<int>" %value) {
  %pointer = getelementptr inbounds %"UniquePtr<UniquePtr<int>>", %"UniquePtr<UniquePtr<int>>"* %this, i32 0, i32 0
  %1 = call %"UniquePtr<int>"* @_EN3std8allocateI9UniquePtrI3intEEE5value9UniquePtrI3intE(%"UniquePtr<int>" %value)
  store %"UniquePtr<int>"* %1, %"UniquePtr<int>"** %pointer
  ret void
}

define %"UniquePtr<int>"* @_EN3std8allocateI9UniquePtrI3intEEE5value9UniquePtrI3intE(%"UniquePtr<int>" %value) {
  %allocation = alloca %"UniquePtr<int>"*
  %1 = call i8* @malloc(i64 ptrtoint (i1** getelementptr (i1*, i1** null, i32 1) to i64))
  %assert.condition = icmp eq i8* %1, null
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %0
  %2 = call i32 @puts(i8* getelementptr inbounds ([37 x i8], [37 x i8]* @0, i32 0, i32 0))
  call void @abort()
  unreachable

assert.success:                                   ; preds = %0
  %3 = bitcast i8* %1 to %"UniquePtr<int>"*
  store %"UniquePtr<int>"* %3, %"UniquePtr<int>"** %allocation
  %allocation1 = load %"UniquePtr<int>"*, %"UniquePtr<int>"** %allocation
  store %"UniquePtr<int>" %value, %"UniquePtr<int>"* %allocation1
  %allocation2 = load %"UniquePtr<int>"*, %"UniquePtr<int>"** %allocation
  ret %"UniquePtr<int>"* %allocation2
}

define void @_ENM3std9UniquePtrI9UniquePtrI3intEE4initE7pointerPM9UniquePtrI3intE(%"UniquePtr<UniquePtr<int>>"* %this, %"UniquePtr<int>"* %pointer) {
  %pointer1 = getelementptr inbounds %"UniquePtr<UniquePtr<int>>", %"UniquePtr<UniquePtr<int>>"* %this, i32 0, i32 0
  store %"UniquePtr<int>"* %pointer, %"UniquePtr<int>"** %pointer1
  ret void
}

define void @_ENM3std9UniquePtrI9UniquePtrI3intEE6deinitE(%"UniquePtr<UniquePtr<int>>"* %this) {
  %pointer = getelementptr inbounds %"UniquePtr<UniquePtr<int>>", %"UniquePtr<UniquePtr<int>>"* %this, i32 0, i32 0
  %pointer1 = load %"UniquePtr<int>"*, %"UniquePtr<int>"** %pointer
  call void @_EN3std10deallocateI9UniquePtrI3intEEE10allocationPM9UniquePtrI3intE(%"UniquePtr<int>"* %pointer1)
  ret void
}

define void @_EN3std10deallocateI9UniquePtrI3intEEE10allocationPM9UniquePtrI3intE(%"UniquePtr<int>"* %allocation) {
  %1 = bitcast %"UniquePtr<int>"* %allocation to i8*
  call void @free(i8* %1)
  ret void
}

define %"UniquePtr<int>"* @_EN3std9UniquePtrI9UniquePtrI3intEE3getE(%"UniquePtr<UniquePtr<int>>"* %this) {
  %pointer = getelementptr inbounds %"UniquePtr<UniquePtr<int>>", %"UniquePtr<UniquePtr<int>>"* %this, i32 0, i32 0
  %pointer1 = load %"UniquePtr<int>"*, %"UniquePtr<int>"** %pointer
  ret %"UniquePtr<int>"* %pointer1
}

define %"UniquePtr<int>"* @_ENM3std9UniquePtrI9UniquePtrI3intEE3getE(%"UniquePtr<UniquePtr<int>>"* %this) {
  %pointer = getelementptr inbounds %"UniquePtr<UniquePtr<int>>", %"UniquePtr<UniquePtr<int>>"* %this, i32 0, i32 0
  %pointer1 = load %"UniquePtr<int>"*, %"UniquePtr<int>"** %pointer
  ret %"UniquePtr<int>"* %pointer1
}

declare i8* @malloc(i64)

declare i32 @puts(i8*)

declare void @abort()

declare void @free(i8*)
