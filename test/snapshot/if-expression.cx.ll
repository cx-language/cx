
declare i1 @a() #0

declare i1 @b() #0

declare i1 @c() #0

declare i1 @d() #0

declare i1 @e() #0

define i32 @main() #0 !dbg !4 {
  %x = alloca i1, align 1
  %1 = call i1 @a(), !dbg !7
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  %2 = call i1 @b(), !dbg !8
  br label %if.end3

if.else:                                          ; preds = %0
  %3 = call i1 @c(), !dbg !9
  br i1 %3, label %if.then1, label %if.else2

if.then1:                                         ; preds = %if.else
  %4 = call i1 @d(), !dbg !10
  br label %if.end

if.else2:                                         ; preds = %if.else
  %5 = call i1 @e(), !dbg !11
  br label %if.end

if.end:                                           ; preds = %if.else2, %if.then1
  %if.result = phi i1 [ %4, %if.then1 ], [ %5, %if.else2 ]
  br label %if.end3

if.end3:                                          ; preds = %if.end, %if.then
  %if.result4 = phi i1 [ %2, %if.then ], [ %if.result, %if.end ]
  store i1 %if.result4, ptr %x, align 1
  call void @_CX1N3std10checkLeaksE4void0_(), !dbg !12
  ret i32 0
}

declare void @_CX1N3std10checkLeaksE4void0_() #0

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "if-expression.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 9, type: !5, scopeLine: 9, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 10, column: 13, scope: !4)
!8 = !DILocation(line: 10, column: 19, scope: !4)
!9 = !DILocation(line: 10, column: 25, scope: !4)
!10 = !DILocation(line: 10, column: 31, scope: !4)
!11 = !DILocation(line: 10, column: 37, scope: !4)
!12 = !DILocation(line: 9, column: 6, scope: !4)
