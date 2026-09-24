
%"ClosedRangeIterator<int32>" = type { i32, i32 }
%"ClosedRange<int32>" = type { i32, i32 }

@0 = private unnamed_addr constant [38 x i8] c"integer overflow at for-loop.cx:6:13\0A\00", align 1

define i32 @main() #0 !dbg !4 {
  %sum = alloca i32, align 4
  %__iterator = alloca %"ClosedRangeIterator<int32>", align 8
  %1 = alloca %"ClosedRange<int32>", align 8
  %i = alloca i32, align 4
  store i32 0, ptr %sum, align 4
  call void @_EN3std11ClosedRangeI5int32E4initE5int325int32(ptr %1, i32 68, i32 75), !dbg !7
  %2 = call %"ClosedRangeIterator<int32>" @_EN3std11ClosedRangeI5int32E8iteratorE(ptr %1), !dbg !8
  store %"ClosedRangeIterator<int32>" %2, ptr %__iterator, align 4
  br label %loop.condition

loop.condition:                                   ; preds = %loop.increment, %0
  %3 = call i1 @_EN3std19ClosedRangeIteratorI5int32E8hasValueE(ptr %__iterator), !dbg !8
  br i1 %3, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %4 = call i32 @_EN3std19ClosedRangeIteratorI5int32E5valueE(ptr %__iterator), !dbg !8
  store i32 %4, ptr %i, align 4
  %sum.load = load i32, ptr %sum, align 4
  %i.load = load i32, ptr %i, align 4
  %5 = sext i32 %sum.load to i64
  %6 = sext i32 %i.load to i64
  %7 = add i64 %5, %6
  %8 = trunc i64 %7 to i32
  %9 = sext i32 %8 to i64
  %10 = icmp ne i64 %7, %9
  %11 = xor i1 %10, true
  %overflow.condition = icmp eq i1 %11, false
  br i1 %overflow.condition, label %overflow.fail, label %overflow.success

loop.increment:                                   ; preds = %overflow.success
  call void @_EN3std19ClosedRangeIteratorI5int32E9incrementE(ptr %__iterator), !dbg !8
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  ret i32 0

overflow.fail:                                    ; preds = %loop.body
  call void @_EN3std10assertFailEP4char(ptr @0), !dbg !9
  unreachable

overflow.success:                                 ; preds = %loop.body
  store i32 %8, ptr %sum, align 4
  br label %loop.increment
}

define void @_EN3std11ClosedRangeI5int32E4initE5int325int32(ptr %this, i32 %start, i32 %end) #0 !dbg !10 {
  %start1 = alloca i32, align 4
  %end2 = alloca i32, align 4
  store i32 %start, ptr %start1, align 4
  store i32 %end, ptr %end2, align 4
  %start3 = getelementptr inbounds %"ClosedRange<int32>", ptr %this, i32 0, i32 0
  %start.load = load i32, ptr %start1, align 4
  store i32 %start.load, ptr %start3, align 4
  %end4 = getelementptr inbounds %"ClosedRange<int32>", ptr %this, i32 0, i32 1
  %end.load = load i32, ptr %end2, align 4
  store i32 %end.load, ptr %end4, align 4
  ret void
}

define %"ClosedRangeIterator<int32>" @_EN3std11ClosedRangeI5int32E8iteratorE(ptr %this) #0 !dbg !12 {
  %1 = alloca %"ClosedRangeIterator<int32>", align 8
  %this.load = load %"ClosedRange<int32>", ptr %this, align 4
  call void @_EN3std19ClosedRangeIteratorI5int32E4initE11ClosedRangeI5int32E(ptr %1, %"ClosedRange<int32>" %this.load), !dbg !13
  %.load = load %"ClosedRangeIterator<int32>", ptr %1, align 4
  ret %"ClosedRangeIterator<int32>" %.load
}

define i1 @_EN3std19ClosedRangeIteratorI5int32E8hasValueE(ptr %this) #0 !dbg !14 {
  %current = getelementptr inbounds %"ClosedRangeIterator<int32>", ptr %this, i32 0, i32 0
  %current.load = load i32, ptr %current, align 4
  %end = getelementptr inbounds %"ClosedRangeIterator<int32>", ptr %this, i32 0, i32 1
  %end.load = load i32, ptr %end, align 4
  %1 = icmp sle i32 %current.load, %end.load
  ret i1 %1
}

define i32 @_EN3std19ClosedRangeIteratorI5int32E5valueE(ptr %this) #0 !dbg !16 {
  %current = getelementptr inbounds %"ClosedRangeIterator<int32>", ptr %this, i32 0, i32 0
  %current.load = load i32, ptr %current, align 4
  ret i32 %current.load
}

define void @_EN3std19ClosedRangeIteratorI5int32E9incrementE(ptr %this) #0 !dbg !17 {
  %current = getelementptr inbounds %"ClosedRangeIterator<int32>", ptr %this, i32 0, i32 0
  %current.load = load i32, ptr %current, align 4
  %1 = add i32 %current.load, 1
  store i32 %1, ptr %current, align 4
  ret void
}

declare void @_EN3std10assertFailEP4char(ptr) #0

define void @_EN3std19ClosedRangeIteratorI5int32E4initE11ClosedRangeI5int32E(ptr %this, %"ClosedRange<int32>" %range) #0 !dbg !18 {
  %range1 = alloca %"ClosedRange<int32>", align 8
  store %"ClosedRange<int32>" %range, ptr %range1, align 4
  %current = getelementptr inbounds %"ClosedRangeIterator<int32>", ptr %this, i32 0, i32 0
  %1 = call i32 @_EN3std11ClosedRangeI5int32E5startE(ptr %range1), !dbg !19
  store i32 %1, ptr %current, align 4
  %end = getelementptr inbounds %"ClosedRangeIterator<int32>", ptr %this, i32 0, i32 1
  %2 = call i32 @_EN3std11ClosedRangeI5int32E3endE(ptr %range1), !dbg !20
  store i32 %2, ptr %end, align 4
  ret void
}

define i32 @_EN3std11ClosedRangeI5int32E5startE(ptr %this) #0 !dbg !21 {
  %start = getelementptr inbounds %"ClosedRange<int32>", ptr %this, i32 0, i32 0
  %start.load = load i32, ptr %start, align 4
  ret i32 %start.load
}

define i32 @_EN3std11ClosedRangeI5int32E3endE(ptr %this) #0 !dbg !22 {
  %end = getelementptr inbounds %"ClosedRange<int32>", ptr %this, i32 0, i32 1
  %end.load = load i32, ptr %end, align 4
  ret i32 %end.load
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "for-loop.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 5, column: 16, scope: !4)
!8 = !DILocation(line: 5, column: 5, scope: !4)
!9 = !DILocation(line: 3, column: 6, scope: !4)
!10 = distinct !DISubprogram(name: "init", linkageName: "_EN3std11ClosedRangeI5int32E4initE5int325int32", scope: !11, file: !11, line: 7, type: !5, scopeLine: 7, spFlags: DISPFlagDefinition, unit: !2)
!11 = !DIFile(filename: "ClosedRange.cx")
!12 = distinct !DISubprogram(name: "iterator", linkageName: "_EN3std11ClosedRangeI5int32E8iteratorE", scope: !11, file: !11, line: 34, type: !5, scopeLine: 34, spFlags: DISPFlagDefinition, unit: !2)
!13 = !DILocation(line: 35, column: 16, scope: !12)
!14 = distinct !DISubprogram(name: "hasValue", linkageName: "_EN3std19ClosedRangeIteratorI5int32E8hasValueE", scope: !15, file: !15, line: 13, type: !5, scopeLine: 13, spFlags: DISPFlagDefinition, unit: !2)
!15 = !DIFile(filename: "ClosedRangeIterator.cx")
!16 = distinct !DISubprogram(name: "value", linkageName: "_EN3std19ClosedRangeIteratorI5int32E5valueE", scope: !15, file: !15, line: 18, type: !5, scopeLine: 18, spFlags: DISPFlagDefinition, unit: !2)
!17 = distinct !DISubprogram(name: "increment", linkageName: "_EN3std19ClosedRangeIteratorI5int32E9incrementE", scope: !15, file: !15, line: 23, type: !5, scopeLine: 23, spFlags: DISPFlagDefinition, unit: !2)
!18 = distinct !DISubprogram(name: "init", linkageName: "_EN3std19ClosedRangeIteratorI5int32E4initE11ClosedRangeI5int32E", scope: !15, file: !15, line: 7, type: !5, scopeLine: 7, spFlags: DISPFlagDefinition, unit: !2)
!19 = !DILocation(line: 8, column: 25, scope: !18)
!20 = !DILocation(line: 9, column: 21, scope: !18)
!21 = distinct !DISubprogram(name: "start", linkageName: "_EN3std11ClosedRangeI5int32E5startE", scope: !11, file: !11, line: 24, type: !5, scopeLine: 24, spFlags: DISPFlagDefinition, unit: !2)
!22 = distinct !DISubprogram(name: "end", linkageName: "_EN3std11ClosedRangeI5int32E3endE", scope: !11, file: !11, line: 29, type: !5, scopeLine: 29, spFlags: DISPFlagDefinition, unit: !2)
