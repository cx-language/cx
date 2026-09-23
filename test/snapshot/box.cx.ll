
%"Box<Box<int>>" = type { ptr }
%"Box<int>" = type { ptr }

@0 = private unnamed_addr constant [35 x i8] c"Unwrap failed at allocate.cx:9:64\0A\00", align 1
@1 = private unnamed_addr constant [35 x i8] c"Unwrap failed at allocate.cx:9:64\0A\00", align 1

define i32 @main() #0 !dbg !4 {
  %p = alloca %"Box<Box<int>>", align 8
  %1 = alloca %"Box<int>", align 8
  %q = alloca %"Box<int>", align 8
  %r = alloca %"Box<int>", align 8
  call void @_EN3std3BoxI3intE4initE3int(ptr %1, i32 42), !dbg !7
  %.load = load %"Box<int>", ptr %1, align 8
  call void @_EN3std3BoxI3BoxI3intEE4initE3BoxI3intE(ptr %p, %"Box<int>" %.load), !dbg !8
  call void @_EN3std3BoxI3intE4initE3int(ptr %q, i32 42), !dbg !9
  call void @_EN3std3BoxI3intE4initE3int(ptr %r, i32 42), !dbg !10
  %2 = call ptr @_EN3std3BoxI3intE3getE(ptr %q), !dbg !11
  store i32 0, ptr %2, align 4
  %3 = call ptr @_EN3std3BoxI3intE3getE(ptr %r), !dbg !12
  call void @_EN3std3BoxI3intE6deinitE(ptr %q), !dbg !13
  %r.load = load %"Box<int>", ptr %r, align 8
  store %"Box<int>" %r.load, ptr %q, align 8
  call void @_EN3std3BoxI3intE6deinitE(ptr %q), !dbg !13
  call void @_EN3std3BoxI3BoxI3intEE6deinitE(ptr %p), !dbg !13
  ret i32 0
}

define void @_EN3std3BoxI3intE4initE3int(ptr %this, i32 %value) #0 !dbg !14 {
  %value1 = alloca i32, align 4
  store i32 %value, ptr %value1, align 4
  %pointer = getelementptr inbounds %"Box<int>", ptr %this, i32 0, i32 0
  %value.load = load i32, ptr %value1, align 4
  %1 = call ptr @_EN3std8allocateI3intEE3int(i32 %value.load), !dbg !16
  store ptr %1, ptr %pointer, align 8
  ret void
}

define void @_EN3std3BoxI3BoxI3intEE4initE3BoxI3intE(ptr %this, %"Box<int>" %value) #0 !dbg !17 {
  %value1 = alloca %"Box<int>", align 8
  store %"Box<int>" %value, ptr %value1, align 8
  %pointer = getelementptr inbounds %"Box<Box<int>>", ptr %this, i32 0, i32 0
  %value.load = load %"Box<int>", ptr %value1, align 8
  %1 = call ptr @_EN3std8allocateI3BoxI3intEEE3BoxI3intE(%"Box<int>" %value.load), !dbg !18
  store ptr %1, ptr %pointer, align 8
  ret void
}

define ptr @_EN3std3BoxI3intE3getE(ptr %this) #0 !dbg !19 {
  %pointer = getelementptr inbounds %"Box<int>", ptr %this, i32 0, i32 0
  %pointer.load = load ptr, ptr %pointer, align 8
  ret ptr %pointer.load
}

define void @_EN3std3BoxI3intE6deinitE(ptr %this) #0 !dbg !20 {
  %pointer = getelementptr inbounds %"Box<int>", ptr %this, i32 0, i32 0
  %pointer.load = load ptr, ptr %pointer, align 8
  call void @_EN3std10deallocateIP3intEEP3int(ptr %pointer.load), !dbg !21
  ret void
}

define void @_EN3std3BoxI3BoxI3intEE6deinitE(ptr %this) #0 !dbg !22 {
  %pointer = getelementptr inbounds %"Box<Box<int>>", ptr %this, i32 0, i32 0
  %pointer.load = load ptr, ptr %pointer, align 8
  call void @_EN3std10deallocateIP3BoxI3intEEEP3BoxI3intE(ptr %pointer.load), !dbg !23
  ret void
}

define void @_EN3std10deallocateIP3BoxI3intEEEP3BoxI3intE(ptr %allocation) #0 !dbg !24 {
  %allocation1 = alloca ptr, align 8
  store ptr %allocation, ptr %allocation1, align 8
  %allocation.load = load ptr, ptr %allocation1, align 8
  call void @free(ptr %allocation.load), !dbg !26
  ret void
}

define ptr @_EN3std8allocateI3BoxI3intEEE3BoxI3intE(%"Box<int>" %value) #0 !dbg !27 {
  %value1 = alloca %"Box<int>", align 8
  %allocation = alloca ptr, align 8
  store %"Box<int>" %value, ptr %value1, align 8
  %1 = call ptr @malloc(i64 ptrtoint (ptr getelementptr (%"Box<int>", ptr null, i32 1) to i64)), !dbg !28
  %assert.condition = icmp eq ptr %1, null
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %0
  call void @_EN3std10assertFailEP4char(ptr @0), !dbg !29
  unreachable

assert.success:                                   ; preds = %0
  store ptr %1, ptr %allocation, align 8
  %allocation.load = load ptr, ptr %allocation, align 8
  %value.load = load %"Box<int>", ptr %value1, align 8
  store %"Box<int>" %value.load, ptr %allocation.load, align 8
  %allocation.load2 = load ptr, ptr %allocation, align 8
  ret ptr %allocation.load2
}

define ptr @_EN3std8allocateI3intEE3int(i32 %value) #0 !dbg !30 {
  %value1 = alloca i32, align 4
  %allocation = alloca ptr, align 8
  store i32 %value, ptr %value1, align 4
  %1 = call ptr @malloc(i64 4), !dbg !31
  %assert.condition = icmp eq ptr %1, null
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %0
  call void @_EN3std10assertFailEP4char(ptr @1), !dbg !32
  unreachable

assert.success:                                   ; preds = %0
  store ptr %1, ptr %allocation, align 8
  %allocation.load = load ptr, ptr %allocation, align 8
  %value.load = load i32, ptr %value1, align 4
  store i32 %value.load, ptr %allocation.load, align 4
  %allocation.load2 = load ptr, ptr %allocation, align 8
  ret ptr %allocation.load2
}

define void @_EN3std10deallocateIP3intEEP3int(ptr %allocation) #0 !dbg !33 {
  %allocation1 = alloca ptr, align 8
  store ptr %allocation, ptr %allocation1, align 8
  %allocation.load = load ptr, ptr %allocation1, align 8
  call void @free(ptr %allocation.load), !dbg !34
  ret void
}

declare void @free(ptr) #0

declare ptr @malloc(i64) #0

declare void @_EN3std10assertFailEP4char(ptr) #0

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "box.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 4, column: 17, scope: !4)
!8 = !DILocation(line: 4, column: 13, scope: !4)
!9 = !DILocation(line: 5, column: 13, scope: !4)
!10 = !DILocation(line: 6, column: 13, scope: !4)
!11 = !DILocation(line: 7, column: 8, scope: !4)
!12 = !DILocation(line: 8, column: 7, scope: !4)
!13 = !DILocation(line: 3, column: 6, scope: !4)
!14 = distinct !DISubprogram(name: "init", linkageName: "_EN3std3BoxI3intE4initE3int", scope: !15, file: !15, line: 6, type: !5, scopeLine: 6, spFlags: DISPFlagDefinition, unit: !2)
!15 = !DIFile(filename: "Box.cx")
!16 = !DILocation(line: 7, column: 24, scope: !14)
!17 = distinct !DISubprogram(name: "init", linkageName: "_EN3std3BoxI3BoxI3intEE4initE3BoxI3intE", scope: !15, file: !15, line: 6, type: !5, scopeLine: 6, spFlags: DISPFlagDefinition, unit: !2)
!18 = !DILocation(line: 7, column: 24, scope: !17)
!19 = distinct !DISubprogram(name: "get", linkageName: "_EN3std3BoxI3intE3getE", scope: !15, file: !15, line: 20, type: !5, scopeLine: 20, spFlags: DISPFlagDefinition, unit: !2)
!20 = distinct !DISubprogram(name: "deinit", linkageName: "_EN3std3BoxI3intE6deinitE", scope: !15, file: !15, line: 15, type: !5, scopeLine: 15, spFlags: DISPFlagDefinition, unit: !2)
!21 = !DILocation(line: 16, column: 9, scope: !20)
!22 = distinct !DISubprogram(name: "deinit", linkageName: "_EN3std3BoxI3BoxI3intEE6deinitE", scope: !15, file: !15, line: 15, type: !5, scopeLine: 15, spFlags: DISPFlagDefinition, unit: !2)
!23 = !DILocation(line: 16, column: 9, scope: !22)
!24 = distinct !DISubprogram(name: "deallocate", linkageName: "_EN3std10deallocateIP3BoxI3intEEEP3BoxI3intE", scope: !25, file: !25, line: 52, type: !5, scopeLine: 52, spFlags: DISPFlagDefinition, unit: !2)
!25 = !DIFile(filename: "allocate.cx")
!26 = !DILocation(line: 53, column: 5, scope: !24)
!27 = distinct !DISubprogram(name: "allocate", linkageName: "_EN3std8allocateI3BoxI3intEEE3BoxI3intE", scope: !25, file: !25, line: 8, type: !5, scopeLine: 8, spFlags: DISPFlagDefinition, unit: !2)
!28 = !DILocation(line: 9, column: 34, scope: !27)
!29 = !DILocation(line: 8, column: 7, scope: !27)
!30 = distinct !DISubprogram(name: "allocate", linkageName: "_EN3std8allocateI3intEE3int", scope: !25, file: !25, line: 8, type: !5, scopeLine: 8, spFlags: DISPFlagDefinition, unit: !2)
!31 = !DILocation(line: 9, column: 34, scope: !30)
!32 = !DILocation(line: 8, column: 7, scope: !30)
!33 = distinct !DISubprogram(name: "deallocate", linkageName: "_EN3std10deallocateIP3intEEP3int", scope: !25, file: !25, line: 52, type: !5, scopeLine: 52, spFlags: DISPFlagDefinition, unit: !2)
!34 = !DILocation(line: 53, column: 5, scope: !33)
