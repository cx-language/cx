
%"ClosedRangeIterator<int>" = type { i32, i32 }
%"ClosedRange<int>" = type { i32, i32 }

define i32 @main() #0 !dbg !4 {
  %__iterator = alloca %"ClosedRangeIterator<int>", align 8
  %1 = alloca %"ClosedRange<int>", align 8
  %i = alloca i32, align 4
  %__iterator1 = alloca %"ClosedRangeIterator<int>", align 8
  %2 = alloca %"ClosedRange<int>", align 8
  %i2 = alloca i32, align 4
  call void @_EN3std11ClosedRangeI3intE4initE3int3int(ptr %1, i32 0, i32 5), !dbg !7
  %3 = call %"ClosedRangeIterator<int>" @_EN3std11ClosedRangeI3intE8iteratorE(ptr %1), !dbg !8
  store %"ClosedRangeIterator<int>" %3, ptr %__iterator, align 4
  br label %loop.condition

loop.condition:                                   ; preds = %loop.increment, %0
  %4 = call i1 @_EN3std19ClosedRangeIteratorI3intE8hasValueE(ptr %__iterator), !dbg !8
  br i1 %4, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %5 = call i32 @_EN3std19ClosedRangeIteratorI3intE5valueE(ptr %__iterator), !dbg !8
  store i32 %5, ptr %i, align 4
  br label %loop.increment

loop.increment:                                   ; preds = %loop.body
  call void @_EN3std19ClosedRangeIteratorI3intE9incrementE(ptr %__iterator), !dbg !8
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  call void @_EN3std11ClosedRangeI3intE4initE3int3int(ptr %2, i32 0, i32 5), !dbg !9
  %6 = call %"ClosedRangeIterator<int>" @_EN3std11ClosedRangeI3intE8iteratorE(ptr %2), !dbg !10
  store %"ClosedRangeIterator<int>" %6, ptr %__iterator1, align 4
  br label %loop.condition3

loop.condition3:                                  ; preds = %loop.increment5, %loop.end
  %7 = call i1 @_EN3std19ClosedRangeIteratorI3intE8hasValueE(ptr %__iterator1), !dbg !10
  br i1 %7, label %loop.body4, label %loop.end6

loop.body4:                                       ; preds = %loop.condition3
  %8 = call i32 @_EN3std19ClosedRangeIteratorI3intE5valueE(ptr %__iterator1), !dbg !10
  store i32 %8, ptr %i2, align 4
  br label %loop.increment5

loop.increment5:                                  ; preds = %loop.body4
  call void @_EN3std19ClosedRangeIteratorI3intE9incrementE(ptr %__iterator1), !dbg !10
  br label %loop.condition3

loop.end6:                                        ; preds = %loop.condition3
  ret i32 0
}

define void @_EN3std11ClosedRangeI3intE4initE3int3int(ptr %this, i32 %start, i32 %end) #0 !dbg !11 {
  %start1 = alloca i32, align 4
  %end2 = alloca i32, align 4
  store i32 %start, ptr %start1, align 4
  store i32 %end, ptr %end2, align 4
  %start3 = getelementptr inbounds %"ClosedRange<int>", ptr %this, i32 0, i32 0
  %start.load = load i32, ptr %start1, align 4
  store i32 %start.load, ptr %start3, align 4
  %end4 = getelementptr inbounds %"ClosedRange<int>", ptr %this, i32 0, i32 1
  %end.load = load i32, ptr %end2, align 4
  store i32 %end.load, ptr %end4, align 4
  ret void
}

define %"ClosedRangeIterator<int>" @_EN3std11ClosedRangeI3intE8iteratorE(ptr %this) #0 !dbg !13 {
  %1 = alloca %"ClosedRangeIterator<int>", align 8
  %this.load = load %"ClosedRange<int>", ptr %this, align 4
  call void @_EN3std19ClosedRangeIteratorI3intE4initE11ClosedRangeI3intE(ptr %1, %"ClosedRange<int>" %this.load), !dbg !14
  %.load = load %"ClosedRangeIterator<int>", ptr %1, align 4
  ret %"ClosedRangeIterator<int>" %.load
}

define i1 @_EN3std19ClosedRangeIteratorI3intE8hasValueE(ptr %this) #0 !dbg !15 {
  %current = getelementptr inbounds %"ClosedRangeIterator<int>", ptr %this, i32 0, i32 0
  %current.load = load i32, ptr %current, align 4
  %end = getelementptr inbounds %"ClosedRangeIterator<int>", ptr %this, i32 0, i32 1
  %end.load = load i32, ptr %end, align 4
  %1 = icmp sle i32 %current.load, %end.load
  ret i1 %1
}

define i32 @_EN3std19ClosedRangeIteratorI3intE5valueE(ptr %this) #0 !dbg !17 {
  %current = getelementptr inbounds %"ClosedRangeIterator<int>", ptr %this, i32 0, i32 0
  %current.load = load i32, ptr %current, align 4
  ret i32 %current.load
}

define void @_EN3std19ClosedRangeIteratorI3intE9incrementE(ptr %this) #0 !dbg !18 {
  %current = getelementptr inbounds %"ClosedRangeIterator<int>", ptr %this, i32 0, i32 0
  %current.load = load i32, ptr %current, align 4
  %1 = add i32 %current.load, 1
  store i32 %1, ptr %current, align 4
  ret void
}

define void @_EN3std19ClosedRangeIteratorI3intE4initE11ClosedRangeI3intE(ptr %this, %"ClosedRange<int>" %range) #0 !dbg !19 {
  %range1 = alloca %"ClosedRange<int>", align 8
  store %"ClosedRange<int>" %range, ptr %range1, align 4
  %current = getelementptr inbounds %"ClosedRangeIterator<int>", ptr %this, i32 0, i32 0
  %1 = call i32 @_EN3std11ClosedRangeI3intE5startE(ptr %range1), !dbg !20
  store i32 %1, ptr %current, align 4
  %end = getelementptr inbounds %"ClosedRangeIterator<int>", ptr %this, i32 0, i32 1
  %2 = call i32 @_EN3std11ClosedRangeI3intE3endE(ptr %range1), !dbg !21
  store i32 %2, ptr %end, align 4
  ret void
}

define i32 @_EN3std11ClosedRangeI3intE5startE(ptr %this) #0 !dbg !22 {
  %start = getelementptr inbounds %"ClosedRange<int>", ptr %this, i32 0, i32 0
  %start.load = load i32, ptr %start, align 4
  ret i32 %start.load
}

define i32 @_EN3std11ClosedRangeI3intE3endE(ptr %this) #0 !dbg !23 {
  %end = getelementptr inbounds %"ClosedRange<int>", ptr %this, i32 0, i32 1
  %end.load = load i32, ptr %end, align 4
  ret i32 %end.load
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "loop-variable-name-reuse.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 4, column: 15, scope: !4)
!8 = !DILocation(line: 4, column: 5, scope: !4)
!9 = !DILocation(line: 5, column: 15, scope: !4)
!10 = !DILocation(line: 5, column: 5, scope: !4)
!11 = distinct !DISubprogram(name: "init", linkageName: "_EN3std11ClosedRangeI3intE4initE3int3int", scope: !12, file: !12, line: 7, type: !5, scopeLine: 7, spFlags: DISPFlagDefinition, unit: !2)
!12 = !DIFile(filename: "ClosedRange.cx")
!13 = distinct !DISubprogram(name: "iterator", linkageName: "_EN3std11ClosedRangeI3intE8iteratorE", scope: !12, file: !12, line: 34, type: !5, scopeLine: 34, spFlags: DISPFlagDefinition, unit: !2)
!14 = !DILocation(line: 35, column: 16, scope: !13)
!15 = distinct !DISubprogram(name: "hasValue", linkageName: "_EN3std19ClosedRangeIteratorI3intE8hasValueE", scope: !16, file: !16, line: 13, type: !5, scopeLine: 13, spFlags: DISPFlagDefinition, unit: !2)
!16 = !DIFile(filename: "ClosedRangeIterator.cx")
!17 = distinct !DISubprogram(name: "value", linkageName: "_EN3std19ClosedRangeIteratorI3intE5valueE", scope: !16, file: !16, line: 18, type: !5, scopeLine: 18, spFlags: DISPFlagDefinition, unit: !2)
!18 = distinct !DISubprogram(name: "increment", linkageName: "_EN3std19ClosedRangeIteratorI3intE9incrementE", scope: !16, file: !16, line: 23, type: !5, scopeLine: 23, spFlags: DISPFlagDefinition, unit: !2)
!19 = distinct !DISubprogram(name: "init", linkageName: "_EN3std19ClosedRangeIteratorI3intE4initE11ClosedRangeI3intE", scope: !16, file: !16, line: 7, type: !5, scopeLine: 7, spFlags: DISPFlagDefinition, unit: !2)
!20 = !DILocation(line: 8, column: 25, scope: !19)
!21 = !DILocation(line: 9, column: 21, scope: !19)
!22 = distinct !DISubprogram(name: "start", linkageName: "_EN3std11ClosedRangeI3intE5startE", scope: !12, file: !12, line: 24, type: !5, scopeLine: 24, spFlags: DISPFlagDefinition, unit: !2)
!23 = distinct !DISubprogram(name: "end", linkageName: "_EN3std11ClosedRangeI3intE3endE", scope: !12, file: !12, line: 29, type: !5, scopeLine: 29, spFlags: DISPFlagDefinition, unit: !2)
