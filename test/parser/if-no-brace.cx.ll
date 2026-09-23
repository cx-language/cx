
define i32 @main() #0 !dbg !4 {
  br i1 false, label %if.then, label %if.else

if.then:                                          ; preds = %0
  ret i32 1

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else
  br i1 true, label %if.then1, label %if.else2

if.then1:                                         ; preds = %if.end
  ret i32 2

if.else2:                                         ; preds = %if.end
  br label %if.end3

if.end3:                                          ; preds = %if.else2
  ret i32 3
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "if-no-brace.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
