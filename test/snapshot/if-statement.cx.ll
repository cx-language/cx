
declare void @foo() #0

declare void @bar() #0

define i32 @main() #0 !dbg !4 {
  br i1 false, label %if.then, label %if.else

if.then:                                          ; preds = %0
  call void @foo(), !dbg !7
  br label %if.end

if.else:                                          ; preds = %0
  call void @bar(), !dbg !8
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  ret i32 0
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "if-statement.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 6, type: !5, scopeLine: 6, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 8, column: 9, scope: !4)
!8 = !DILocation(line: 10, column: 9, scope: !4)
