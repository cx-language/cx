
define i32 @main() #0 !dbg !4 {
  br label %loop.condition

loop.condition:                                   ; preds = %0
  br i1 true, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  br i1 true, label %if.then, label %if.else

loop.end:                                         ; preds = %if.end, %if.then, %loop.condition
  switch i32 1, label %switch.default [
  ]

if.then:                                          ; preds = %loop.body
  br label %loop.end

if.else:                                          ; preds = %loop.body
  br label %if.end

if.end:                                           ; preds = %if.else
  br label %loop.end

switch.default:                                   ; preds = %loop.end
  br label %switch.end

switch.end:                                       ; preds = %switch.default
  ret i32 0
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "break.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
