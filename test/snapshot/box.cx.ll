
%"Box<Box<int>>" = type { %"Box<int>"* }
%"Box<int>" = type { i32* }

@0 = private unnamed_addr constant [35 x i8] c"Unwrap failed at allocate.cx:9:54\0A\00", align 1
@1 = private unnamed_addr constant [35 x i8] c"Unwrap failed at allocate.cx:9:54\0A\00", align 1

define i32 @main() {
  %p = alloca %"Box<Box<int>>", align 8
  %1 = alloca %"Box<int>", align 8
  %q = alloca %"Box<int>", align 8
  %r = alloca %"Box<int>", align 8
  call void @_EN3std3BoxI3intE4initE3int(%"Box<int>"* %1, i32 42)
  %.load = load %"Box<int>", %"Box<int>"* %1, align 8
  call void @_EN3std3BoxI3BoxI3intEE4initE3BoxI3intE(%"Box<Box<int>>"* %p, %"Box<int>" %.load)
  call void @_EN3std3BoxI3intE4initE3int(%"Box<int>"* %q, i32 42)
  call void @_EN3std3BoxI3intE4initE3int(%"Box<int>"* %r, i32 42)
  %2 = call i32* @_EN3std3BoxI3intE3getE(%"Box<int>"* %q)
  store i32 0, i32* %2, align 4
  %3 = call i32* @_EN3std3BoxI3intE3getE(%"Box<int>"* %r)
  call void @_EN3std3BoxI3intE6deinitE(%"Box<int>"* %q)
  %r.load = load %"Box<int>", %"Box<int>"* %r, align 8
  store %"Box<int>" %r.load, %"Box<int>"* %q, align 8
  call void @_EN3std3BoxI3intE6deinitE(%"Box<int>"* %q)
  call void @_EN3std3BoxI3BoxI3intEE6deinitE(%"Box<Box<int>>"* %p)
  ret i32 0
}

define void @_EN3std3BoxI3intE4initE3int(%"Box<int>"* %this, i32 %value) {
  %pointer = getelementptr inbounds %"Box<int>", %"Box<int>"* %this, i32 0, i32 0
  %1 = call i32* @_EN3std8allocateI3intEE3int(i32 %value)
  store i32* %1, i32** %pointer, align 8
  ret void
}

define void @_EN3std3BoxI3BoxI3intEE4initE3BoxI3intE(%"Box<Box<int>>"* %this, %"Box<int>" %value) {
  %pointer = getelementptr inbounds %"Box<Box<int>>", %"Box<Box<int>>"* %this, i32 0, i32 0
  %1 = call %"Box<int>"* @_EN3std8allocateI3BoxI3intEEE3BoxI3intE(%"Box<int>" %value)
  store %"Box<int>"* %1, %"Box<int>"** %pointer, align 8
  ret void
}

define i32* @_EN3std3BoxI3intE3getE(%"Box<int>"* %this) {
  %pointer = getelementptr inbounds %"Box<int>", %"Box<int>"* %this, i32 0, i32 0
  %pointer.load = load i32*, i32** %pointer, align 8
  ret i32* %pointer.load
}

define void @_EN3std3BoxI3intE6deinitE(%"Box<int>"* %this) {
  %pointer = getelementptr inbounds %"Box<int>", %"Box<int>"* %this, i32 0, i32 0
  %pointer.load = load i32*, i32** %pointer, align 8
  call void @_EN3std10deallocateIP3intEEP3int(i32* %pointer.load)
  ret void
}

define void @_EN3std3BoxI3BoxI3intEE6deinitE(%"Box<Box<int>>"* %this) {
  %pointer = getelementptr inbounds %"Box<Box<int>>", %"Box<Box<int>>"* %this, i32 0, i32 0
  %pointer.load = load %"Box<int>"*, %"Box<int>"** %pointer, align 8
  call void @_EN3std10deallocateIP3BoxI3intEEEP3BoxI3intE(%"Box<int>"* %pointer.load)
  ret void
}

define void @_EN3std10deallocateIP3BoxI3intEEEP3BoxI3intE(%"Box<int>"* %allocation) {
  %1 = bitcast %"Box<int>"* %allocation to i8*
  call void @free(i8* %1)
  ret void
}

define %"Box<int>"* @_EN3std8allocateI3BoxI3intEEE3BoxI3intE(%"Box<int>" %value) {
  %allocation = alloca %"Box<int>"*, align 8
  %1 = call i8* @malloc(i64 ptrtoint (i1** getelementptr (i1*, i1** null, i32 1) to i64))
  %assert.condition = icmp eq i8* %1, null
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %0
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([35 x i8], [35 x i8]* @0, i32 0, i32 0))
  unreachable

assert.success:                                   ; preds = %0
  %2 = bitcast i8* %1 to %"Box<int>"*
  store %"Box<int>"* %2, %"Box<int>"** %allocation, align 8
  %allocation.load = load %"Box<int>"*, %"Box<int>"** %allocation, align 8
  store %"Box<int>" %value, %"Box<int>"* %allocation.load, align 8
  %allocation.load1 = load %"Box<int>"*, %"Box<int>"** %allocation, align 8
  ret %"Box<int>"* %allocation.load1
}

define i32* @_EN3std8allocateI3intEE3int(i32 %value) {
  %allocation = alloca i32*, align 8
  %1 = call i8* @malloc(i64 ptrtoint (i32* getelementptr (i32, i32* null, i32 1) to i64))
  %assert.condition = icmp eq i8* %1, null
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %0
  call void @_EN3std10assertFailEP4char(i8* getelementptr inbounds ([35 x i8], [35 x i8]* @1, i32 0, i32 0))
  unreachable

assert.success:                                   ; preds = %0
  %2 = bitcast i8* %1 to i32*
  store i32* %2, i32** %allocation, align 8
  %allocation.load = load i32*, i32** %allocation, align 8
  store i32 %value, i32* %allocation.load, align 4
  %allocation.load1 = load i32*, i32** %allocation, align 8
  ret i32* %allocation.load1
}

define void @_EN3std10deallocateIP3intEEP3int(i32* %allocation) {
  %1 = bitcast i32* %allocation to i8*
  call void @free(i8* %1)
  ret void
}

declare void @free(i8*)

declare i8* @malloc(i64)

declare void @_EN3std10assertFailEP4char(i8*)
