
declare i1 @foo() #0

define void @_CX1N4main3barE4void0_() #0 !dbg !4 {
  %1 = call i1 @foo(), !dbg !7
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  ret void

if.else:                                          ; preds = %0
  ret void

if.end:                                           ; No predecessors!
  %2 = call i1 @foo(), !dbg !8
  br i1 %2, label %if.then1, label %if.else2

if.then1:                                         ; preds = %if.end
  ret void

if.else2:                                         ; preds = %if.end
  ret void

if.end3:                                          ; No predecessors!
  br label %loop.condition

loop.condition:                                   ; preds = %if.end3
  %3 = call i1 @foo(), !dbg !9
  br i1 %3, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  ret void

loop.end:                                         ; preds = %loop.condition
  br label %loop.condition4

loop.condition4:                                  ; preds = %loop.end
  %4 = call i1 @foo(), !dbg !10
  br i1 %4, label %loop.body5, label %loop.end6

loop.body5:                                       ; preds = %loop.condition4
  ret void

loop.end6:                                        ; preds = %loop.condition4
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "return-inside-if.cx")
!4 = distinct !DISubprogram(name: "bar", linkageName: "_CX1N4main3barE4void0_", scope: !3, file: !3, line: 5, type: !5, scopeLine: 5, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 6, column: 9, scope: !4)
!8 = !DILocation(line: 12, column: 9, scope: !4)
!9 = !DILocation(line: 20, column: 12, scope: !4)
!10 = !DILocation(line: 24, column: 12, scope: !4)
