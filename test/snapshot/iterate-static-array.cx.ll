
%"ArrayIterator<int>" = type { ptr, ptr }
%OutputStream = type { ptr, ptr }

define i32 @main() #0 !dbg !4 {
  %__iterator = alloca %"ArrayIterator<int>", align 8
  %1 = alloca [3 x i32], align 4
  %e = alloca ptr, align 8
  %a = alloca [2 x i32], align 4
  %__iterator1 = alloca %"ArrayIterator<int>", align 8
  %e2 = alloca ptr, align 8
  store [3 x i32] [i32 1, i32 2, i32 3], ptr %1, align 4
  %2 = getelementptr inbounds [3 x i32], ptr %1, i32 0, i32 0
  %3 = getelementptr inbounds i32, ptr %2, i32 3
  %4 = insertvalue %"ArrayIterator<int>" undef, ptr %2, 0
  %5 = insertvalue %"ArrayIterator<int>" %4, ptr %3, 1
  store %"ArrayIterator<int>" %5, ptr %__iterator, align 8
  br label %loop.condition

loop.condition:                                   ; preds = %loop.increment, %0
  %6 = call i1 @_EN3std13ArrayIteratorI3intE8hasValueE(ptr %__iterator), !dbg !7
  br i1 %6, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %7 = call ptr @_EN3std13ArrayIteratorI3intE5valueE(ptr %__iterator), !dbg !7
  store ptr %7, ptr %e, align 8
  %e.load = load ptr, ptr %e, align 8
  call void @_EN3std7printlnI3intEER3int(ptr %e.load), !dbg !8
  br label %loop.increment

loop.increment:                                   ; preds = %loop.body
  call void @_EN3std13ArrayIteratorI3intE9incrementE(ptr %__iterator), !dbg !7
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  store [2 x i32] [i32 4, i32 5], ptr %a, align 4
  %8 = getelementptr inbounds [2 x i32], ptr %a, i32 0, i32 0
  %9 = getelementptr inbounds i32, ptr %8, i32 2
  %10 = insertvalue %"ArrayIterator<int>" undef, ptr %8, 0
  %11 = insertvalue %"ArrayIterator<int>" %10, ptr %9, 1
  store %"ArrayIterator<int>" %11, ptr %__iterator1, align 8
  br label %loop.condition3

loop.condition3:                                  ; preds = %loop.increment6, %loop.end
  %12 = call i1 @_EN3std13ArrayIteratorI3intE8hasValueE(ptr %__iterator1), !dbg !9
  br i1 %12, label %loop.body4, label %loop.end7

loop.body4:                                       ; preds = %loop.condition3
  %13 = call ptr @_EN3std13ArrayIteratorI3intE5valueE(ptr %__iterator1), !dbg !9
  store ptr %13, ptr %e2, align 8
  %e.load5 = load ptr, ptr %e2, align 8
  call void @_EN3std7printlnI3intEER3int(ptr %e.load5), !dbg !10
  br label %loop.increment6

loop.increment6:                                  ; preds = %loop.body4
  call void @_EN3std13ArrayIteratorI3intE9incrementE(ptr %__iterator1), !dbg !9
  br label %loop.condition3

loop.end7:                                        ; preds = %loop.condition3
  ret i32 0
}

define i1 @_EN3std13ArrayIteratorI3intE8hasValueE(ptr %this) #0 !dbg !11 {
  %current = getelementptr inbounds %"ArrayIterator<int>", ptr %this, i32 0, i32 0
  %current.load = load ptr, ptr %current, align 8
  %end = getelementptr inbounds %"ArrayIterator<int>", ptr %this, i32 0, i32 1
  %end.load = load ptr, ptr %end, align 8
  %1 = icmp ne ptr %current.load, %end.load
  ret i1 %1
}

define ptr @_EN3std13ArrayIteratorI3intE5valueE(ptr %this) #0 !dbg !13 {
  %current = getelementptr inbounds %"ArrayIterator<int>", ptr %this, i32 0, i32 0
  %current.load = load ptr, ptr %current, align 8
  ret ptr %current.load
}

define void @_EN3std7printlnI3intEER3int(ptr %value) #0 !dbg !14 {
  %value1 = alloca ptr, align 8
  store ptr %value, ptr %value1, align 8
  %value.load = load ptr, ptr %value1, align 8
  call void @_EN3std5printI3intEER3int(ptr %value.load), !dbg !16
  call void @_EN3std5printI4charEE4char(i8 10), !dbg !17
  ret void
}

define void @_EN3std13ArrayIteratorI3intE9incrementE(ptr %this) #0 !dbg !18 {
  %current = getelementptr inbounds %"ArrayIterator<int>", ptr %this, i32 0, i32 0
  %current.load = load ptr, ptr %current, align 8
  %1 = getelementptr inbounds i32, ptr %current.load, i32 1
  store ptr %1, ptr %current, align 8
  ret void
}

define void @_EN3std5printI3intEER3int(ptr %value) #0 !dbg !19 {
  %value1 = alloca ptr, align 8
  %stream = alloca %OutputStream, align 8
  store ptr %value, ptr %value1, align 8
  call void @_EN3std12OutputStream4initE(ptr %stream), !dbg !20
  %value.load = load ptr, ptr %value1, align 8
  call void @_EN3std3int5printER12OutputStream(ptr %value.load, ptr %stream), !dbg !21
  ret void
}

declare void @_EN3std5printI4charEE4char(i8) #0

declare void @_EN3std12OutputStream4initE(ptr) #0

declare void @_EN3std3int5printER12OutputStream(ptr, ptr) #0

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
!11 = distinct !DISubprogram(name: "hasValue", linkageName: "_EN3std13ArrayIteratorI3intE8hasValueE", scope: !12, file: !12, line: 13, type: !5, scopeLine: 13, spFlags: DISPFlagDefinition, unit: !2)
!12 = !DIFile(filename: "ArrayIterator.cx")
!13 = distinct !DISubprogram(name: "value", linkageName: "_EN3std13ArrayIteratorI3intE5valueE", scope: !12, file: !12, line: 18, type: !5, scopeLine: 18, spFlags: DISPFlagDefinition, unit: !2)
!14 = distinct !DISubprogram(name: "println", linkageName: "_EN3std7printlnI3intEER3int", scope: !15, file: !15, line: 2, type: !5, scopeLine: 2, spFlags: DISPFlagDefinition, unit: !2)
!15 = !DIFile(filename: "stdio.cx")
!16 = !DILocation(line: 3, column: 5, scope: !14)
!17 = !DILocation(line: 4, column: 5, scope: !14)
!18 = distinct !DISubprogram(name: "increment", linkageName: "_EN3std13ArrayIteratorI3intE9incrementE", scope: !12, file: !12, line: 23, type: !5, scopeLine: 23, spFlags: DISPFlagDefinition, unit: !2)
!19 = distinct !DISubprogram(name: "print", linkageName: "_EN3std5printI3intEER3int", scope: !15, file: !15, line: 33, type: !5, scopeLine: 33, spFlags: DISPFlagDefinition, unit: !2)
!20 = !DILocation(line: 34, column: 18, scope: !19)
!21 = !DILocation(line: 35, column: 11, scope: !19)
