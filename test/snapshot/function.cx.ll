
define i32 @main() #0 !dbg !4 {
  call void @bar(), !dbg !7
  call void @_EN4main3fooE(), !dbg !8
  ret i32 0
}

declare void @bar() #0

define void @_EN4main3fooE() #0 !dbg !9 {
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "function.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 4, column: 5, scope: !4)
!8 = !DILocation(line: 5, column: 5, scope: !4)
!9 = distinct !DISubprogram(name: "foo", linkageName: "_EN4main3fooE", scope: !3, file: !3, line: 8, type: !5, scopeLine: 8, spFlags: DISPFlagDefinition, unit: !2)
