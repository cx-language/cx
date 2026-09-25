
define i32 @main() #0 !dbg !4 {
  %1 = call i32 @_CX1N4main3fooEM3std5int320_(), !dbg !7
  ret i32 %1
}

define i32 @_CX1N4main3fooEM3std5int320_() #0 !dbg !8 {
  %1 = call i32 @_CX1N4main3fooEM3std5int320_(), !dbg !9
  ret i32 42
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "return-statement.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 4, column: 12, scope: !4)
!8 = distinct !DISubprogram(name: "foo", linkageName: "_CX1N4main3fooEM3std5int320_", scope: !3, file: !3, line: 7, type: !5, scopeLine: 7, spFlags: DISPFlagDefinition, unit: !2)
!9 = !DILocation(line: 8, column: 5, scope: !8)
