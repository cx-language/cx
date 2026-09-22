
%"List<int>" = type { ptr, i32, i32 }
%"ArrayIterator<int>" = type { ptr, ptr }
%"ArrayRef<int>" = type { ptr, i32 }
%"RangeIterator<int>" = type { i32, i32 }
%"Range<int>" = type { i32, i32 }

@0 = private unnamed_addr constant [39 x i8] c"integer overflow at allocate.cx:36:46\0A\00", align 1
@1 = private unnamed_addr constant [36 x i8] c"Unwrap failed at allocate.cx:36:61\0A\00", align 1

define i32 @main() #0 !dbg !4 {
  %i = alloca %"List<int>", align 8
  %j = alloca %"List<int>", align 8
  %1 = alloca %"List<int>", align 8
  call void @_EN3std4ListI3intE4initE8capacity3int(ptr %i, i32 10), !dbg !7
  call void @_EN3std4ListI3intE4initE(ptr %j), !dbg !8
  call void @_EN3std4ListI3intE6deinitE(ptr %i), !dbg !9
  call void @_EN3std4ListI3intE4initE(ptr %1), !dbg !10
  %.load = load %"List<int>", ptr %1, align 8
  store %"List<int>" %.load, ptr %i, align 8
  call void @_EN3std4ListI3intE6deinitE(ptr %j), !dbg !9
  call void @_EN3std4ListI3intE6deinitE(ptr %i), !dbg !9
  ret i32 0
}

define void @_EN3std4ListI3intE4initE8capacity3int(ptr %this, i32 %capacity) #0 !dbg !11 {
  %capacity1 = alloca i32, align 4
  store i32 %capacity, ptr %capacity1, align 4
  call void @_EN3std4ListI3intE4initE(ptr %this), !dbg !13
  %capacity.load = load i32, ptr %capacity1, align 4
  call void @_EN3std4ListI3intE7reserveE3int(ptr %this, i32 %capacity.load), !dbg !14
  ret void
}

define void @_EN3std4ListI3intE4initE(ptr %this) #0 !dbg !15 {
  %size = getelementptr inbounds %"List<int>", ptr %this, i32 0, i32 1
  store i32 0, ptr %size, align 4
  %capacity = getelementptr inbounds %"List<int>", ptr %this, i32 0, i32 2
  store i32 0, ptr %capacity, align 4
  ret void
}

define void @_EN3std4ListI3intE6deinitE(ptr %this) #0 !dbg !16 {
  %__iterator = alloca %"ArrayIterator<int>", align 8
  %element = alloca ptr, align 8
  %capacity = getelementptr inbounds %"List<int>", ptr %this, i32 0, i32 2
  %capacity.load = load i32, ptr %capacity, align 4
  %1 = icmp ne i32 %capacity.load, 0
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  %2 = call %"ArrayIterator<int>" @_EN3std4ListI3intE8iteratorE(ptr %this), !dbg !17
  store %"ArrayIterator<int>" %2, ptr %__iterator, align 8
  br label %loop.condition

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %loop.end, %if.else
  ret void

loop.condition:                                   ; preds = %loop.increment, %if.then
  %3 = call i1 @_EN3std13ArrayIteratorI3intE8hasValueE(ptr %__iterator), !dbg !17
  br i1 %3, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %4 = call ptr @_EN3std13ArrayIteratorI3intE5valueE(ptr %__iterator), !dbg !17
  store ptr %4, ptr %element, align 8
  br label %loop.increment

loop.increment:                                   ; preds = %loop.body
  call void @_EN3std13ArrayIteratorI3intE9incrementE(ptr %__iterator), !dbg !17
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  %buffer = getelementptr inbounds %"List<int>", ptr %this, i32 0, i32 0
  %buffer.load = load ptr, ptr %buffer, align 8
  call void @_EN3std10deallocateIAU_3intEEAU_3int(ptr %buffer.load), !dbg !18
  br label %if.end
}

define %"ArrayIterator<int>" @_EN3std4ListI3intE8iteratorE(ptr %this) #0 !dbg !19 {
  %1 = alloca %"ArrayIterator<int>", align 8
  %2 = alloca %"ArrayRef<int>", align 8
  call void @_EN3std8ArrayRefI3intE4initEP4ListI3intE(ptr %2, ptr %this), !dbg !20
  %.load = load %"ArrayRef<int>", ptr %2, align 8
  call void @_EN3std13ArrayIteratorI3intE4initE8ArrayRefI3intE(ptr %1, %"ArrayRef<int>" %.load), !dbg !21
  %.load1 = load %"ArrayIterator<int>", ptr %1, align 8
  ret %"ArrayIterator<int>" %.load1
}

define i1 @_EN3std13ArrayIteratorI3intE8hasValueE(ptr %this) #0 !dbg !22 {
  %current = getelementptr inbounds %"ArrayIterator<int>", ptr %this, i32 0, i32 0
  %current.load = load ptr, ptr %current, align 8
  %end = getelementptr inbounds %"ArrayIterator<int>", ptr %this, i32 0, i32 1
  %end.load = load ptr, ptr %end, align 8
  %1 = icmp ne ptr %current.load, %end.load
  ret i1 %1
}

define ptr @_EN3std13ArrayIteratorI3intE5valueE(ptr %this) #0 !dbg !24 {
  %current = getelementptr inbounds %"ArrayIterator<int>", ptr %this, i32 0, i32 0
  %current.load = load ptr, ptr %current, align 8
  ret ptr %current.load
}

define void @_EN3std13ArrayIteratorI3intE9incrementE(ptr %this) #0 !dbg !25 {
  %current = getelementptr inbounds %"ArrayIterator<int>", ptr %this, i32 0, i32 0
  %current.load = load ptr, ptr %current, align 8
  %1 = getelementptr inbounds i32, ptr %current.load, i32 1
  store ptr %1, ptr %current, align 8
  ret void
}

define void @_EN3std10deallocateIAU_3intEEAU_3int(ptr %allocation) #0 !dbg !26 {
  %allocation1 = alloca ptr, align 8
  store ptr %allocation, ptr %allocation1, align 8
  %allocation.load = load ptr, ptr %allocation1, align 8
  call void @free(ptr %allocation.load), !dbg !28
  ret void
}

define void @_EN3std4ListI3intE7reserveE3int(ptr %this, i32 %minimumCapacity) #0 !dbg !29 {
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
  %3 = call ptr @_EN3std13allocateArrayI3intEE3int(i32 %minimumCapacity.load2), !dbg !30
  store ptr %3, ptr %newBuffer, align 8
  %size = getelementptr inbounds %"List<int>", ptr %this, i32 0, i32 1
  %size.load = load i32, ptr %size, align 4
  call void @_EN3std5RangeI3intE4initE3int3int(ptr %1, i32 0, i32 %size.load), !dbg !31
  %4 = call %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(ptr %1), !dbg !32
  store %"RangeIterator<int>" %4, ptr %__iterator, align 4
  br label %loop.condition

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.end10, %if.else
  ret void

loop.condition:                                   ; preds = %loop.increment, %if.then
  %5 = call i1 @_EN3std13RangeIteratorI3intE8hasValueE(ptr %__iterator), !dbg !32
  br i1 %5, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %6 = call i32 @_EN3std13RangeIteratorI3intE5valueE(ptr %__iterator), !dbg !32
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
  call void @_EN3std13RangeIteratorI3intE9incrementE(ptr %__iterator), !dbg !32
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  %capacity4 = getelementptr inbounds %"List<int>", ptr %this, i32 0, i32 2
  %capacity.load5 = load i32, ptr %capacity4, align 4
  %9 = icmp ne i32 %capacity.load5, 0
  br i1 %9, label %if.then6, label %if.else9

if.then6:                                         ; preds = %loop.end
  %buffer7 = getelementptr inbounds %"List<int>", ptr %this, i32 0, i32 0
  %buffer.load8 = load ptr, ptr %buffer7, align 8
  call void @_EN3std10deallocateIAU_3intEEAU_3int(ptr %buffer.load8), !dbg !33
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

define void @_EN3std8ArrayRefI3intE4initEP4ListI3intE(ptr %this, ptr %list) #0 !dbg !34 {
  %list1 = alloca ptr, align 8
  store ptr %list, ptr %list1, align 8
  %data = getelementptr inbounds %"ArrayRef<int>", ptr %this, i32 0, i32 0
  %list.load = load ptr, ptr %list1, align 8
  %1 = call ptr @_EN3std4ListI3intE4dataE(ptr %list.load), !dbg !36
  store ptr %1, ptr %data, align 8
  %size = getelementptr inbounds %"ArrayRef<int>", ptr %this, i32 0, i32 1
  %list.load2 = load ptr, ptr %list1, align 8
  %2 = call i32 @_EN3std4ListI3intE4sizeE(ptr %list.load2), !dbg !37
  store i32 %2, ptr %size, align 4
  ret void
}

define void @_EN3std13ArrayIteratorI3intE4initE8ArrayRefI3intE(ptr %this, %"ArrayRef<int>" %array) #0 !dbg !38 {
  %array1 = alloca %"ArrayRef<int>", align 8
  store %"ArrayRef<int>" %array, ptr %array1, align 8
  %current = getelementptr inbounds %"ArrayIterator<int>", ptr %this, i32 0, i32 0
  %1 = call ptr @_EN3std8ArrayRefI3intE4dataE(ptr %array1), !dbg !39
  store ptr %1, ptr %current, align 8
  %end = getelementptr inbounds %"ArrayIterator<int>", ptr %this, i32 0, i32 1
  %2 = call ptr @_EN3std8ArrayRefI3intE4dataE(ptr %array1), !dbg !40
  %3 = call i32 @_EN3std8ArrayRefI3intE4sizeE(ptr %array1), !dbg !41
  %4 = getelementptr inbounds i32, ptr %2, i32 %3
  store ptr %4, ptr %end, align 8
  ret void
}

declare void @free(ptr) #0

define ptr @_EN3std13allocateArrayI3intEE3int(i32 %size) #0 !dbg !42 {
  %size1 = alloca i32, align 4
  store i32 %size, ptr %size1, align 4
  %size.load = load i32, ptr %size1, align 4
  %1 = sext i32 %size.load to i64
  %2 = mul i64 4, %1
  %3 = icmp eq i64 %1, 0
  br i1 %3, label %overflow.end, label %overflow.check

overflow.check:                                   ; preds = %0
  %4 = udiv i64 %2, %1
  %5 = icmp ne i64 %4, 4
  br label %overflow.end

overflow.end:                                     ; preds = %overflow.check, %0
  %overflowed = phi i1 [ false, %0 ], [ %5, %overflow.check ]
  %6 = xor i1 %overflowed, true
  %overflow.condition = icmp eq i1 %6, false
  br i1 %overflow.condition, label %overflow.fail, label %overflow.success

overflow.fail:                                    ; preds = %overflow.end
  call void @_EN3std10assertFailEP4char(ptr @0), !dbg !43
  unreachable

overflow.success:                                 ; preds = %overflow.end
  %7 = call ptr @malloc(i64 %2), !dbg !44
  %assert.condition = icmp eq ptr %7, null
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %overflow.success
  call void @_EN3std10assertFailEP4char(ptr @1), !dbg !43
  unreachable

assert.success:                                   ; preds = %overflow.success
  ret ptr %7
}

declare void @_EN3std5RangeI3intE4initE3int3int(ptr, i32, i32) #0

declare %"RangeIterator<int>" @_EN3std5RangeI3intE8iteratorE(ptr) #0

declare i1 @_EN3std13RangeIteratorI3intE8hasValueE(ptr) #0

declare i32 @_EN3std13RangeIteratorI3intE5valueE(ptr) #0

declare void @_EN3std13RangeIteratorI3intE9incrementE(ptr) #0

define ptr @_EN3std8ArrayRefI3intE4dataE(ptr %this) #0 !dbg !45 {
  %data = getelementptr inbounds %"ArrayRef<int>", ptr %this, i32 0, i32 0
  %data.load = load ptr, ptr %data, align 8
  ret ptr %data.load
}

define i32 @_EN3std8ArrayRefI3intE4sizeE(ptr %this) #0 !dbg !46 {
  %size = getelementptr inbounds %"ArrayRef<int>", ptr %this, i32 0, i32 1
  %size.load = load i32, ptr %size, align 4
  ret i32 %size.load
}

define ptr @_EN3std4ListI3intE4dataE(ptr %this) #0 !dbg !47 {
  %buffer = getelementptr inbounds %"List<int>", ptr %this, i32 0, i32 0
  %buffer.load = load ptr, ptr %buffer, align 8
  ret ptr %buffer.load
}

define i32 @_EN3std4ListI3intE4sizeE(ptr %this) #0 !dbg !48 {
  %size = getelementptr inbounds %"List<int>", ptr %this, i32 0, i32 1
  %size.load = load i32, ptr %size, align 4
  ret i32 %size.load
}

declare void @_EN3std10assertFailEP4char(ptr) #0

declare ptr @malloc(i64) #0

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "infer-generic-arguments-from-assignment-lhs.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 4, column: 19, scope: !4)
!8 = !DILocation(line: 5, column: 19, scope: !4)
!9 = !DILocation(line: 3, column: 6, scope: !4)
!10 = !DILocation(line: 7, column: 9, scope: !4)
!11 = distinct !DISubprogram(name: "init", linkageName: "_EN3std4ListI3intE4initE8capacity3int", scope: !12, file: !12, line: 15, type: !5, scopeLine: 15, spFlags: DISPFlagDefinition, unit: !2)
!12 = !DIFile(filename: "List.cx")
!13 = !DILocation(line: 16, column: 9, scope: !11)
!14 = !DILocation(line: 17, column: 9, scope: !11)
!15 = distinct !DISubprogram(name: "init", linkageName: "_EN3std4ListI3intE4initE", scope: !12, file: !12, line: 8, type: !5, scopeLine: 8, spFlags: DISPFlagDefinition, unit: !2)
!16 = distinct !DISubprogram(name: "deinit", linkageName: "_EN3std4ListI3intE6deinitE", scope: !12, file: !12, line: 43, type: !5, scopeLine: 43, spFlags: DISPFlagDefinition, unit: !2)
!17 = !DILocation(line: 45, column: 13, scope: !16)
!18 = !DILocation(line: 48, column: 13, scope: !16)
!19 = distinct !DISubprogram(name: "iterator", linkageName: "_EN3std4ListI3intE8iteratorE", scope: !12, file: !12, line: 215, type: !5, scopeLine: 215, spFlags: DISPFlagDefinition, unit: !2)
!20 = !DILocation(line: 217, column: 30, scope: !19)
!21 = !DILocation(line: 217, column: 16, scope: !19)
!22 = distinct !DISubprogram(name: "hasValue", linkageName: "_EN3std13ArrayIteratorI3intE8hasValueE", scope: !23, file: !23, line: 13, type: !5, scopeLine: 13, spFlags: DISPFlagDefinition, unit: !2)
!23 = !DIFile(filename: "ArrayIterator.cx")
!24 = distinct !DISubprogram(name: "value", linkageName: "_EN3std13ArrayIteratorI3intE5valueE", scope: !23, file: !23, line: 18, type: !5, scopeLine: 18, spFlags: DISPFlagDefinition, unit: !2)
!25 = distinct !DISubprogram(name: "increment", linkageName: "_EN3std13ArrayIteratorI3intE9incrementE", scope: !23, file: !23, line: 23, type: !5, scopeLine: 23, spFlags: DISPFlagDefinition, unit: !2)
!26 = distinct !DISubprogram(name: "deallocate", linkageName: "_EN3std10deallocateIAU_3intEEAU_3int", scope: !27, file: !27, line: 52, type: !5, scopeLine: 52, spFlags: DISPFlagDefinition, unit: !2)
!27 = !DIFile(filename: "allocate.cx")
!28 = !DILocation(line: 53, column: 5, scope: !26)
!29 = distinct !DISubprogram(name: "reserve", linkageName: "_EN3std4ListI3intE7reserveE3int", scope: !12, file: !12, line: 140, type: !5, scopeLine: 140, spFlags: DISPFlagDefinition, unit: !2)
!30 = !DILocation(line: 142, column: 29, scope: !29)
!31 = !DILocation(line: 144, column: 27, scope: !29)
!32 = !DILocation(line: 144, column: 13, scope: !29)
!33 = !DILocation(line: 151, column: 17, scope: !29)
!34 = distinct !DISubprogram(name: "init", linkageName: "_EN3std8ArrayRefI3intE4initEP4ListI3intE", scope: !35, file: !35, line: 13, type: !5, scopeLine: 13, spFlags: DISPFlagDefinition, unit: !2)
!35 = !DIFile(filename: "ArrayRef.cx")
!36 = !DILocation(line: 14, column: 21, scope: !34)
!37 = !DILocation(line: 15, column: 21, scope: !34)
!38 = distinct !DISubprogram(name: "init", linkageName: "_EN3std13ArrayIteratorI3intE4initE8ArrayRefI3intE", scope: !23, file: !23, line: 7, type: !5, scopeLine: 7, spFlags: DISPFlagDefinition, unit: !2)
!39 = !DILocation(line: 8, column: 25, scope: !38)
!40 = !DILocation(line: 9, column: 22, scope: !38)
!41 = !DILocation(line: 9, column: 35, scope: !38)
!42 = distinct !DISubprogram(name: "allocateArray", linkageName: "_EN3std13allocateArrayI3intEE3int", scope: !27, file: !27, line: 35, type: !5, scopeLine: 35, spFlags: DISPFlagDefinition, unit: !2)
!43 = !DILocation(line: 35, column: 9, scope: !42)
!44 = !DILocation(line: 36, column: 26, scope: !42)
!45 = distinct !DISubprogram(name: "data", linkageName: "_EN3std8ArrayRefI3intE4dataE", scope: !35, file: !35, line: 55, type: !5, scopeLine: 55, spFlags: DISPFlagDefinition, unit: !2)
!46 = distinct !DISubprogram(name: "size", linkageName: "_EN3std8ArrayRefI3intE4sizeE", scope: !35, file: !35, line: 31, type: !5, scopeLine: 31, spFlags: DISPFlagDefinition, unit: !2)
!47 = distinct !DISubprogram(name: "data", linkageName: "_EN3std4ListI3intE4dataE", scope: !12, file: !12, line: 117, type: !5, scopeLine: 117, spFlags: DISPFlagDefinition, unit: !2)
!48 = distinct !DISubprogram(name: "size", linkageName: "_EN3std4ListI3intE4sizeE", scope: !12, file: !12, line: 53, type: !5, scopeLine: 53, spFlags: DISPFlagDefinition, unit: !2)
