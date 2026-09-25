
@_CX1G4main3fooE = private global [3 x i32] [i32 0, i32 1, i32 2]

define i32 @main() #0 !dbg !4 {
  %qux = alloca [1 x i32], align 4
  store [1 x i32] [i32 42], ptr %qux, align 4
  store i32 3, ptr @_CX1G4main3fooE, align 4
  %1 = call i64 @_CX1N3stdM3std5int324hashEM3std6uint640_(ptr getelementptr inbounds ([3 x i32], ptr @_CX1G4main3fooE, i32 0, i32 1)), !dbg !7
  ret i32 0
}

declare i64 @_CX1N3stdM3std5int324hashEM3std6uint640_(ptr) #0

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "variable-of-array-type.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 5, type: !5, scopeLine: 5, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 8, column: 12, scope: !4)
