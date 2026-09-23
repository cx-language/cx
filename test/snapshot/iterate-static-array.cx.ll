
%"ArrayIterator<int>" = type { ptr, ptr }
%"Slice<int>" = type { ptr, i32 }

define i32 @main() #0 !dbg !4 {
  %__iterator = alloca %"ArrayIterator<int>", align 8
  %1 = alloca [3 x i32], align 4
  %e = alloca i32, align 4
  %a = alloca [2 x i32], align 4
  %__iterator1 = alloca %"ArrayIterator<int>", align 8
  %e2 = alloca i32, align 4
  store [3 x i32] [i32 1, i32 2, i32 3], ptr %1, align 4
  %2 = call %"ArrayIterator<int>" @_EN3std5ArrayI3intN3_E8iteratorE(ptr %1), !dbg !7
  store %"ArrayIterator<int>" %2, ptr %__iterator, align 8
  br label %loop.condition

loop.condition:                                   ; preds = %loop.increment, %0
  %3 = call i1 @_EN3std13ArrayIteratorI3intE8hasValueE(ptr %__iterator), !dbg !7
  br i1 %3, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %4 = call ptr @_EN3std13ArrayIteratorI3intE5valueE(ptr %__iterator), !dbg !7
  %.load = load i32, ptr %4, align 4
  store i32 %.load, ptr %e, align 4
  %e.load = load i32, ptr %e, align 4
  call void @_EN3std7printlnI3intEE3int(i32 %e.load), !dbg !8
  br label %loop.increment

loop.increment:                                   ; preds = %loop.body
  call void @_EN3std13ArrayIteratorI3intE9incrementE(ptr %__iterator), !dbg !7
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  store [2 x i32] [i32 4, i32 5], ptr %a, align 4
  %5 = call %"ArrayIterator<int>" @_EN3std5ArrayI3intN2_E8iteratorE(ptr %a), !dbg !9
  store %"ArrayIterator<int>" %5, ptr %__iterator1, align 8
  br label %loop.condition3

loop.condition3:                                  ; preds = %loop.increment7, %loop.end
  %6 = call i1 @_EN3std13ArrayIteratorI3intE8hasValueE(ptr %__iterator1), !dbg !9
  br i1 %6, label %loop.body4, label %loop.end8

loop.body4:                                       ; preds = %loop.condition3
  %7 = call ptr @_EN3std13ArrayIteratorI3intE5valueE(ptr %__iterator1), !dbg !9
  %.load5 = load i32, ptr %7, align 4
  store i32 %.load5, ptr %e2, align 4
  %e.load6 = load i32, ptr %e2, align 4
  call void @_EN3std7printlnI3intEE3int(i32 %e.load6), !dbg !10
  br label %loop.increment7

loop.increment7:                                  ; preds = %loop.body4
  call void @_EN3std13ArrayIteratorI3intE9incrementE(ptr %__iterator1), !dbg !9
  br label %loop.condition3

loop.end8:                                        ; preds = %loop.condition3
  ret i32 0
}

define %"ArrayIterator<int>" @_EN3std5ArrayI3intN3_E8iteratorE(ptr %this) #0 !dbg !11 {
  %1 = alloca %"ArrayIterator<int>", align 8
  %2 = getelementptr inbounds [3 x i32], ptr %this, i32 0, i32 0
  %3 = insertvalue %"Slice<int>" undef, ptr %2, 0
  %4 = insertvalue %"Slice<int>" %3, i32 3, 1
  call void @_EN3std13ArrayIteratorI3intE4initE5SliceI3intE(ptr %1, %"Slice<int>" %4), !dbg !13
  %.load = load %"ArrayIterator<int>", ptr %1, align 8
  ret %"ArrayIterator<int>" %.load
}

define i1 @_EN3std13ArrayIteratorI3intE8hasValueE(ptr %this) #0 !dbg !14 {
  %current = getelementptr inbounds %"ArrayIterator<int>", ptr %this, i32 0, i32 0
  %current.load = load ptr, ptr %current, align 8
  %end = getelementptr inbounds %"ArrayIterator<int>", ptr %this, i32 0, i32 1
  %end.load = load ptr, ptr %end, align 8
  %1 = icmp ne ptr %current.load, %end.load
  ret i1 %1
}

define ptr @_EN3std13ArrayIteratorI3intE5valueE(ptr %this) #0 !dbg !16 {
  %current = getelementptr inbounds %"ArrayIterator<int>", ptr %this, i32 0, i32 0
  %current.load = load ptr, ptr %current, align 8
  ret ptr %current.load
}

define void @_EN3std7printlnI3intEE3int(i32 %value) #0 !dbg !17 {
  %value1 = alloca i32, align 4
  %1 = alloca i8, align 1
  store i32 %value, ptr %value1, align 4
  call void @_EN3std5printI3intEER3int(ptr %value1), !dbg !19
  store i8 10, ptr %1, align 1
  call void @_EN3std5printI4charEER4char(ptr %1), !dbg !20
  ret void
}

define void @_EN3std13ArrayIteratorI3intE9incrementE(ptr %this) #0 !dbg !21 {
  %current = getelementptr inbounds %"ArrayIterator<int>", ptr %this, i32 0, i32 0
  %current.load = load ptr, ptr %current, align 8
  %1 = getelementptr inbounds i32, ptr %current.load, i32 1
  store ptr %1, ptr %current, align 8
  ret void
}

define %"ArrayIterator<int>" @_EN3std5ArrayI3intN2_E8iteratorE(ptr %this) #0 !dbg !22 {
  %1 = alloca %"ArrayIterator<int>", align 8
  %2 = getelementptr inbounds [2 x i32], ptr %this, i32 0, i32 0
  %3 = insertvalue %"Slice<int>" undef, ptr %2, 0
  %4 = insertvalue %"Slice<int>" %3, i32 2, 1
  call void @_EN3std13ArrayIteratorI3intE4initE5SliceI3intE(ptr %1, %"Slice<int>" %4), !dbg !23
  %.load = load %"ArrayIterator<int>", ptr %1, align 8
  ret %"ArrayIterator<int>" %.load
}

define void @_EN3std13ArrayIteratorI3intE4initE5SliceI3intE(ptr %this, %"Slice<int>" %array) #0 !dbg !24 {
  %array1 = alloca %"Slice<int>", align 8
  store %"Slice<int>" %array, ptr %array1, align 8
  %current = getelementptr inbounds %"ArrayIterator<int>", ptr %this, i32 0, i32 0
  %1 = call ptr @_EN3std5SliceI3intE4dataE(ptr %array1), !dbg !25
  store ptr %1, ptr %current, align 8
  %end = getelementptr inbounds %"ArrayIterator<int>", ptr %this, i32 0, i32 1
  %2 = call ptr @_EN3std5SliceI3intE4dataE(ptr %array1), !dbg !26
  %3 = call i32 @_EN3std5SliceI3intE4sizeE(ptr %array1), !dbg !27
  %4 = getelementptr inbounds i32, ptr %2, i32 %3
  store ptr %4, ptr %end, align 8
  ret void
}

declare void @_EN3std5printI3intEER3int(ptr) #0

declare void @_EN3std5printI4charEER4char(ptr) #0

define ptr @_EN3std5SliceI3intE4dataE(ptr %this) #0 !dbg !28 {
  %data = getelementptr inbounds %"Slice<int>", ptr %this, i32 0, i32 0
  %data.load = load ptr, ptr %data, align 8
  ret ptr %data.load
}

define i32 @_EN3std5SliceI3intE4sizeE(ptr %this) #0 !dbg !30 {
  %size = getelementptr inbounds %"Slice<int>", ptr %this, i32 0, i32 1
  %size.load = load i32, ptr %size, align 4
  ret i32 %size.load
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "iterate-static-array.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 4, type: !5, scopeLine: 4, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 5, column: 5, scope: !4)
!8 = !DILocation(line: 9, column: 9, scope: !4)
!9 = !DILocation(line: 13, column: 5, scope: !4)
!10 = !DILocation(line: 16, column: 9, scope: !4)
!11 = distinct !DISubprogram(name: "iterator", linkageName: "_EN3std5ArrayI3intN3_E8iteratorE", scope: !12, file: !12, line: 29, type: !5, scopeLine: 29, spFlags: DISPFlagDefinition, unit: !2)
!12 = !DIFile(filename: "Array.cx")
!13 = !DILocation(line: 30, column: 16, scope: !11)
!14 = distinct !DISubprogram(name: "hasValue", linkageName: "_EN3std13ArrayIteratorI3intE8hasValueE", scope: !15, file: !15, line: 13, type: !5, scopeLine: 13, spFlags: DISPFlagDefinition, unit: !2)
!15 = !DIFile(filename: "ArrayIterator.cx")
!16 = distinct !DISubprogram(name: "value", linkageName: "_EN3std13ArrayIteratorI3intE5valueE", scope: !15, file: !15, line: 18, type: !5, scopeLine: 18, spFlags: DISPFlagDefinition, unit: !2)
!17 = distinct !DISubprogram(name: "println", linkageName: "_EN3std7printlnI3intEE3int", scope: !18, file: !18, line: 8, type: !5, scopeLine: 8, spFlags: DISPFlagDefinition, unit: !2)
!18 = !DIFile(filename: "stdio.cx")
!19 = !DILocation(line: 9, column: 5, scope: !17)
!20 = !DILocation(line: 10, column: 5, scope: !17)
!21 = distinct !DISubprogram(name: "increment", linkageName: "_EN3std13ArrayIteratorI3intE9incrementE", scope: !15, file: !15, line: 23, type: !5, scopeLine: 23, spFlags: DISPFlagDefinition, unit: !2)
!22 = distinct !DISubprogram(name: "iterator", linkageName: "_EN3std5ArrayI3intN2_E8iteratorE", scope: !12, file: !12, line: 29, type: !5, scopeLine: 29, spFlags: DISPFlagDefinition, unit: !2)
!23 = !DILocation(line: 30, column: 16, scope: !22)
!24 = distinct !DISubprogram(name: "init", linkageName: "_EN3std13ArrayIteratorI3intE4initE5SliceI3intE", scope: !15, file: !15, line: 7, type: !5, scopeLine: 7, spFlags: DISPFlagDefinition, unit: !2)
!25 = !DILocation(line: 8, column: 25, scope: !24)
!26 = !DILocation(line: 9, column: 22, scope: !24)
!27 = !DILocation(line: 9, column: 35, scope: !24)
!28 = distinct !DISubprogram(name: "data", linkageName: "_EN3std5SliceI3intE4dataE", scope: !29, file: !29, line: 55, type: !5, scopeLine: 55, spFlags: DISPFlagDefinition, unit: !2)
!29 = !DIFile(filename: "Slice.cx")
!30 = distinct !DISubprogram(name: "size", linkageName: "_EN3std5SliceI3intE4sizeE", scope: !29, file: !29, line: 31, type: !5, scopeLine: 31, spFlags: DISPFlagDefinition, unit: !2)
