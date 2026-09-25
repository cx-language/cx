
declare void @foo() #0

declare void @bar() #0

define i32 @main() #0 !dbg !4 {
  br i1 false, label %if.then, label %if.else

if.then:                                          ; preds = %0
  call void @bar(), !dbg !7
  call void @foo(), !dbg !8
  ret i32 0

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else
  br i1 false, label %if.then1, label %if.else2

if.then1:                                         ; preds = %if.end
  call void @bar(), !dbg !9
  br label %if.end3

if.else2:                                         ; preds = %if.end
  br label %if.end3

if.end3:                                          ; preds = %if.else2, %if.then1
  br label %loop.condition

loop.condition:                                   ; preds = %loop.body, %if.end3
  br i1 false, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  call void @bar(), !dbg !10
  call void @foo(), !dbg !11
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  br i1 true, label %if.then4, label %if.else5

if.then4:                                         ; preds = %loop.end
  call void @bar(), !dbg !12
  call void @foo(), !dbg !8
  ret i32 0

if.else5:                                         ; preds = %loop.end
  br label %if.end6

if.end6:                                          ; preds = %if.else5
  call void @foo(), !dbg !13
  call void @bar(), !dbg !12
  call void @foo(), !dbg !8
  ret i32 0
}

define void @_CX1N4main13shouldBeEmptyE4void0_() #0 !dbg !14 {
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "defer.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 6, type: !5, scopeLine: 6, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 9, column: 15, scope: !4)
!8 = !DILocation(line: 7, column: 11, scope: !4)
!9 = !DILocation(line: 13, column: 15, scope: !4)
!10 = !DILocation(line: 17, column: 9, scope: !4)
!11 = !DILocation(line: 16, column: 15, scope: !4)
!12 = !DILocation(line: 19, column: 11, scope: !4)
!13 = !DILocation(line: 23, column: 5, scope: !4)
!14 = distinct !DISubprogram(name: "shouldBeEmpty", linkageName: "_CX1N4main13shouldBeEmptyE4void0_", scope: !3, file: !3, line: 26, type: !5, scopeLine: 26, spFlags: DISPFlagDefinition, unit: !2)
