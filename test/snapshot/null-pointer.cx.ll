
define ptr @_CX1N4main3fooEOPM3std5int320_() #0 !dbg !4 {
  ret ptr null
}

define i32 @main() #0 !dbg !7 {
  %isNull = alloca i1, align 1
  %isNonNull = alloca i1, align 1
  %ptr = alloca ptr, align 8
  %ptr2 = alloca ptr, align 8
  %1 = call ptr @_CX1N4main3fooEOPM3std5int320_(), !dbg !8
  %2 = icmp eq ptr %1, null
  store i1 %2, ptr %isNull, align 1
  %3 = call ptr @_CX1N4main3fooEOPM3std5int320_(), !dbg !9
  %4 = icmp ne ptr %3, null
  store i1 %4, ptr %isNonNull, align 1
  %5 = call ptr @_CX1N4main3fooEOPM3std5int320_(), !dbg !10
  store ptr %5, ptr %ptr, align 8
  store ptr null, ptr %ptr2, align 8
  call void @_CX1N3std10checkLeaksE4void0_(), !dbg !11
  ret i32 0
}

declare void @_CX1N3std10checkLeaksE4void0_() #0

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "null-pointer.cx")
!4 = distinct !DISubprogram(name: "foo", linkageName: "_CX1N4main3fooEOPM3std5int320_", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 5, type: !5, scopeLine: 5, spFlags: DISPFlagDefinition, unit: !2)
!8 = !DILocation(line: 6, column: 18, scope: !7)
!9 = !DILocation(line: 7, column: 21, scope: !7)
!10 = !DILocation(line: 8, column: 15, scope: !7)
!11 = !DILocation(line: 5, column: 6, scope: !7)
