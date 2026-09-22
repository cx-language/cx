
declare i1 @foo() #0

define i32 @main() #0 !dbg !4 {
  br label %loop.condition

loop.condition:                                   ; preds = %loop.body, %0
  %1 = call i1 @foo(), !dbg !7
  br i1 %1, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %2 = call i1 @foo(), !dbg !8
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  ret i32 0
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "while-loop.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 5, type: !5, scopeLine: 5, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 6, column: 11, scope: !4)
!8 = !DILocation(line: 7, column: 9, scope: !4)
