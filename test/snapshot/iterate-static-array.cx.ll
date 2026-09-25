
%"ArrayIterator<int32>" = type { ptr, ptr }
%"Slice<int32>" = type { ptr, i32 }
%OutputStream = type { ptr, ptr }

define i32 @main() #0 !dbg !4 {
  %__iterator = alloca %"ArrayIterator<int32>", align 8
  %1 = alloca [3 x i32], align 4
  %e = alloca ptr, align 8
  %a = alloca [2 x i32], align 4
  %__iterator1 = alloca %"ArrayIterator<int32>", align 8
  %e2 = alloca ptr, align 8
  store [3 x i32] [i32 1, i32 2, i32 3], ptr %1, align 4
  %2 = call %"ArrayIterator<int32>" @_CX1N3stdM3std5ArrayIM3std5int32N3_E8iteratorEM3std13ArrayIteratorIM3std5int32E0_(ptr %1), !dbg !7
  store %"ArrayIterator<int32>" %2, ptr %__iterator, align 8
  br label %loop.condition

loop.condition:                                   ; preds = %loop.increment, %0
  %3 = call i1 @_CX1N3stdM3std13ArrayIteratorIM3std5int32E8hasValueEM3std4bool0_(ptr %__iterator), !dbg !7
  br i1 %3, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %4 = call ptr @_CX1N3stdM3std13ArrayIteratorIM3std5int32E5valueERM3std5int320_(ptr %__iterator), !dbg !7
  store ptr %4, ptr %e, align 8
  %e.load = load ptr, ptr %e, align 8
  call void @_CX1N3std7printlnIM3std5int32EE4void1_RM3std5int32(ptr %e.load), !dbg !8
  br label %loop.increment

loop.increment:                                   ; preds = %loop.body
  call void @_CX1N3stdM3std13ArrayIteratorIM3std5int32E9incrementE4void0_(ptr %__iterator), !dbg !7
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  store [2 x i32] [i32 4, i32 5], ptr %a, align 4
  %5 = call %"ArrayIterator<int32>" @_CX1N3stdM3std5ArrayIM3std5int32N2_E8iteratorEM3std13ArrayIteratorIM3std5int32E0_(ptr %a), !dbg !9
  store %"ArrayIterator<int32>" %5, ptr %__iterator1, align 8
  br label %loop.condition3

loop.condition3:                                  ; preds = %loop.increment6, %loop.end
  %6 = call i1 @_CX1N3stdM3std13ArrayIteratorIM3std5int32E8hasValueEM3std4bool0_(ptr %__iterator1), !dbg !9
  br i1 %6, label %loop.body4, label %loop.end7

loop.body4:                                       ; preds = %loop.condition3
  %7 = call ptr @_CX1N3stdM3std13ArrayIteratorIM3std5int32E5valueERM3std5int320_(ptr %__iterator1), !dbg !9
  store ptr %7, ptr %e2, align 8
  %e.load5 = load ptr, ptr %e2, align 8
  call void @_CX1N3std7printlnIM3std5int32EE4void1_RM3std5int32(ptr %e.load5), !dbg !10
  br label %loop.increment6

loop.increment6:                                  ; preds = %loop.body4
  call void @_CX1N3stdM3std13ArrayIteratorIM3std5int32E9incrementE4void0_(ptr %__iterator1), !dbg !9
  br label %loop.condition3

loop.end7:                                        ; preds = %loop.condition3
  call void @_CX1N3std10checkLeaksE4void0_(), !dbg !11
  ret i32 0
}

define %"ArrayIterator<int32>" @_CX1N3stdM3std5ArrayIM3std5int32N3_E8iteratorEM3std13ArrayIteratorIM3std5int32E0_(ptr %this) #0 !dbg !12 {
  %1 = alloca %"ArrayIterator<int32>", align 8
  %2 = getelementptr inbounds [3 x i32], ptr %this, i32 0, i32 0
  %3 = insertvalue %"Slice<int32>" undef, ptr %2, 0
  %4 = insertvalue %"Slice<int32>" %3, i32 3, 1
  call void @_CX1N3stdM3std13ArrayIteratorIM3std5int32E4initE4void1_M3std5SliceIM3std5int32E(ptr %1, %"Slice<int32>" %4), !dbg !14
  %.load = load %"ArrayIterator<int32>", ptr %1, align 8
  ret %"ArrayIterator<int32>" %.load
}

define i1 @_CX1N3stdM3std13ArrayIteratorIM3std5int32E8hasValueEM3std4bool0_(ptr %this) #0 !dbg !15 {
  %current = getelementptr inbounds %"ArrayIterator<int32>", ptr %this, i32 0, i32 0
  %current.load = load ptr, ptr %current, align 8
  %end = getelementptr inbounds %"ArrayIterator<int32>", ptr %this, i32 0, i32 1
  %end.load = load ptr, ptr %end, align 8
  %1 = icmp ne ptr %current.load, %end.load
  ret i1 %1
}

define ptr @_CX1N3stdM3std13ArrayIteratorIM3std5int32E5valueERM3std5int320_(ptr %this) #0 !dbg !17 {
  %current = getelementptr inbounds %"ArrayIterator<int32>", ptr %this, i32 0, i32 0
  %current.load = load ptr, ptr %current, align 8
  ret ptr %current.load
}

define void @_CX1N3std7printlnIM3std5int32EE4void1_RM3std5int32(ptr %value) #0 !dbg !18 {
  %value1 = alloca ptr, align 8
  store ptr %value, ptr %value1, align 8
  %value.load = load ptr, ptr %value1, align 8
  call void @_CX1N3std5printIM3std5int32EE4void1_RM3std5int32(ptr %value.load), !dbg !20
  call void @_CX1N3std5printIM3std4charEE4void1_M3std4char(i8 10), !dbg !21
  ret void
}

define void @_CX1N3stdM3std13ArrayIteratorIM3std5int32E9incrementE4void0_(ptr %this) #0 !dbg !22 {
  %current = getelementptr inbounds %"ArrayIterator<int32>", ptr %this, i32 0, i32 0
  %current.load = load ptr, ptr %current, align 8
  %1 = getelementptr inbounds i32, ptr %current.load, i32 1
  store ptr %1, ptr %current, align 8
  ret void
}

define %"ArrayIterator<int32>" @_CX1N3stdM3std5ArrayIM3std5int32N2_E8iteratorEM3std13ArrayIteratorIM3std5int32E0_(ptr %this) #0 !dbg !23 {
  %1 = alloca %"ArrayIterator<int32>", align 8
  %2 = getelementptr inbounds [2 x i32], ptr %this, i32 0, i32 0
  %3 = insertvalue %"Slice<int32>" undef, ptr %2, 0
  %4 = insertvalue %"Slice<int32>" %3, i32 2, 1
  call void @_CX1N3stdM3std13ArrayIteratorIM3std5int32E4initE4void1_M3std5SliceIM3std5int32E(ptr %1, %"Slice<int32>" %4), !dbg !24
  %.load = load %"ArrayIterator<int32>", ptr %1, align 8
  ret %"ArrayIterator<int32>" %.load
}

declare void @_CX1N3std10checkLeaksE4void0_() #0

define void @_CX1N3stdM3std13ArrayIteratorIM3std5int32E4initE4void1_M3std5SliceIM3std5int32E(ptr %this, %"Slice<int32>" %array) #0 !dbg !25 {
  %array1 = alloca %"Slice<int32>", align 8
  store %"Slice<int32>" %array, ptr %array1, align 8
  %current = getelementptr inbounds %"ArrayIterator<int32>", ptr %this, i32 0, i32 0
  %1 = call ptr @_CX1N3stdM3std5SliceIM3std5int32E4dataEAM3std5int320_(ptr %array1), !dbg !26
  store ptr %1, ptr %current, align 8
  %end = getelementptr inbounds %"ArrayIterator<int32>", ptr %this, i32 0, i32 1
  %2 = call ptr @_CX1N3stdM3std5SliceIM3std5int32E4dataEAM3std5int320_(ptr %array1), !dbg !27
  %3 = call i32 @_CX1N3stdM3std5SliceIM3std5int32E4sizeEM3std5int320_(ptr %array1), !dbg !28
  %4 = getelementptr inbounds i32, ptr %2, i32 %3
  store ptr %4, ptr %end, align 8
  ret void
}

define void @_CX1N3std5printIM3std5int32EE4void1_RM3std5int32(ptr %value) #0 !dbg !29 {
  %value1 = alloca ptr, align 8
  %stream = alloca %OutputStream, align 8
  store ptr %value, ptr %value1, align 8
  call void @_CX1N3stdM3std12OutputStream4initE4void0_(ptr %stream), !dbg !30
  %value.load = load ptr, ptr %value1, align 8
  call void @_CX1N3stdM3std5int325printE4void1_RM3std12OutputStream(ptr %value.load, ptr %stream), !dbg !31
  ret void
}

declare void @_CX1N3std5printIM3std4charEE4void1_M3std4char(i8) #0

define ptr @_CX1N3stdM3std5SliceIM3std5int32E4dataEAM3std5int320_(ptr %this) #0 !dbg !32 {
  %data = getelementptr inbounds %"Slice<int32>", ptr %this, i32 0, i32 0
  %data.load = load ptr, ptr %data, align 8
  ret ptr %data.load
}

define i32 @_CX1N3stdM3std5SliceIM3std5int32E4sizeEM3std5int320_(ptr %this) #0 !dbg !34 {
  %size = getelementptr inbounds %"Slice<int32>", ptr %this, i32 0, i32 1
  %size.load = load i32, ptr %size, align 4
  ret i32 %size.load
}

declare void @_CX1N3stdM3std12OutputStream4initE4void0_(ptr) #0

declare void @_CX1N3stdM3std5int325printE4void1_RM3std12OutputStream(ptr, ptr) #0

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
!11 = !DILocation(line: 4, column: 6, scope: !4)
!12 = distinct !DISubprogram(name: "iterator", linkageName: "_CX1N3stdM3std5ArrayIM3std5int32N3_E8iteratorEM3std13ArrayIteratorIM3std5int32E0_", scope: !13, file: !13, line: 46, type: !5, scopeLine: 46, spFlags: DISPFlagDefinition, unit: !2)
!13 = !DIFile(filename: "Array.cx")
!14 = !DILocation(line: 47, column: 16, scope: !12)
!15 = distinct !DISubprogram(name: "hasValue", linkageName: "_CX1N3stdM3std13ArrayIteratorIM3std5int32E8hasValueEM3std4bool0_", scope: !16, file: !16, line: 13, type: !5, scopeLine: 13, spFlags: DISPFlagDefinition, unit: !2)
!16 = !DIFile(filename: "ArrayIterator.cx")
!17 = distinct !DISubprogram(name: "value", linkageName: "_CX1N3stdM3std13ArrayIteratorIM3std5int32E5valueERM3std5int320_", scope: !16, file: !16, line: 18, type: !5, scopeLine: 18, spFlags: DISPFlagDefinition, unit: !2)
!18 = distinct !DISubprogram(name: "println", linkageName: "_CX1N3std7printlnIM3std5int32EE4void1_RM3std5int32", scope: !19, file: !19, line: 2, type: !5, scopeLine: 2, spFlags: DISPFlagDefinition, unit: !2)
!19 = !DIFile(filename: "stdio.cx")
!20 = !DILocation(line: 3, column: 5, scope: !18)
!21 = !DILocation(line: 4, column: 5, scope: !18)
!22 = distinct !DISubprogram(name: "increment", linkageName: "_CX1N3stdM3std13ArrayIteratorIM3std5int32E9incrementE4void0_", scope: !16, file: !16, line: 23, type: !5, scopeLine: 23, spFlags: DISPFlagDefinition, unit: !2)
!23 = distinct !DISubprogram(name: "iterator", linkageName: "_CX1N3stdM3std5ArrayIM3std5int32N2_E8iteratorEM3std13ArrayIteratorIM3std5int32E0_", scope: !13, file: !13, line: 46, type: !5, scopeLine: 46, spFlags: DISPFlagDefinition, unit: !2)
!24 = !DILocation(line: 47, column: 16, scope: !23)
!25 = distinct !DISubprogram(name: "init", linkageName: "_CX1N3stdM3std13ArrayIteratorIM3std5int32E4initE4void1_M3std5SliceIM3std5int32E", scope: !16, file: !16, line: 7, type: !5, scopeLine: 7, spFlags: DISPFlagDefinition, unit: !2)
!26 = !DILocation(line: 8, column: 25, scope: !25)
!27 = !DILocation(line: 9, column: 22, scope: !25)
!28 = !DILocation(line: 9, column: 35, scope: !25)
!29 = distinct !DISubprogram(name: "print", linkageName: "_CX1N3std5printIM3std5int32EE4void1_RM3std5int32", scope: !19, file: !19, line: 33, type: !5, scopeLine: 33, spFlags: DISPFlagDefinition, unit: !2)
!30 = !DILocation(line: 34, column: 18, scope: !29)
!31 = !DILocation(line: 35, column: 11, scope: !29)
!32 = distinct !DISubprogram(name: "data", linkageName: "_CX1N3stdM3std5SliceIM3std5int32E4dataEAM3std5int320_", scope: !33, file: !33, line: 76, type: !5, scopeLine: 76, spFlags: DISPFlagDefinition, unit: !2)
!33 = !DIFile(filename: "Slice.cx")
!34 = distinct !DISubprogram(name: "size", linkageName: "_CX1N3stdM3std5SliceIM3std5int32E4sizeEM3std5int320_", scope: !33, file: !33, line: 31, type: !5, scopeLine: 31, spFlags: DISPFlagDefinition, unit: !2)
