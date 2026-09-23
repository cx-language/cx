
declare i1 @a() #0

declare i1 @b() #0

declare i1 @c() #0

define i32 @main() #0 !dbg !4 {
  %and = alloca i1, align 1
  %and2 = alloca i1, align 1
  %or = alloca i1, align 1
  %or2 = alloca i1, align 1
  %1 = call i1 @a(), !dbg !7
  br i1 %1, label %and.rhs, label %and.end

and.rhs:                                          ; preds = %0
  %2 = call i1 @b(), !dbg !8
  br label %and.end

and.end:                                          ; preds = %and.rhs, %0
  %and1 = phi i1 [ %1, %0 ], [ %2, %and.rhs ]
  store i1 %and1, ptr %and, align 1
  %3 = call i1 @a(), !dbg !9
  br i1 %3, label %and.rhs3, label %and.end4

and.rhs2:                                         ; preds = %and.end4
  %4 = call i1 @c(), !dbg !10
  br label %and.end6

and.rhs3:                                         ; preds = %and.end
  %5 = call i1 @b(), !dbg !11
  br label %and.end4

and.end4:                                         ; preds = %and.rhs3, %and.end
  %and5 = phi i1 [ %3, %and.end ], [ %5, %and.rhs3 ]
  br i1 %and5, label %and.rhs2, label %and.end6

and.end6:                                         ; preds = %and.end4, %and.rhs2
  %and7 = phi i1 [ %and5, %and.end4 ], [ %4, %and.rhs2 ]
  store i1 %and7, ptr %and2, align 1
  %6 = call i1 @a(), !dbg !12
  br i1 %6, label %or.end, label %or.rhs

or.rhs:                                           ; preds = %and.end6
  %7 = call i1 @b(), !dbg !13
  br label %or.end

or.end:                                           ; preds = %or.rhs, %and.end6
  %or8 = phi i1 [ %6, %and.end6 ], [ %7, %or.rhs ]
  store i1 %or8, ptr %or, align 1
  %8 = call i1 @a(), !dbg !14
  br i1 %8, label %or.end11, label %or.rhs10

or.rhs9:                                          ; preds = %or.end11
  %9 = call i1 @c(), !dbg !15
  br label %or.end13

or.rhs10:                                         ; preds = %or.end
  %10 = call i1 @b(), !dbg !16
  br label %or.end11

or.end11:                                         ; preds = %or.rhs10, %or.end
  %or12 = phi i1 [ %8, %or.end ], [ %10, %or.rhs10 ]
  br i1 %or12, label %or.end13, label %or.rhs9

or.end13:                                         ; preds = %or.end11, %or.rhs9
  %or14 = phi i1 [ %or12, %or.end11 ], [ %9, %or.rhs9 ]
  store i1 %or14, ptr %or2, align 1
  ret i32 0
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "and-or.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 7, type: !5, scopeLine: 7, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 8, column: 15, scope: !4)
!8 = !DILocation(line: 8, column: 22, scope: !4)
!9 = !DILocation(line: 9, column: 16, scope: !4)
!10 = !DILocation(line: 9, column: 30, scope: !4)
!11 = !DILocation(line: 9, column: 23, scope: !4)
!12 = !DILocation(line: 10, column: 14, scope: !4)
!13 = !DILocation(line: 10, column: 21, scope: !4)
!14 = !DILocation(line: 11, column: 15, scope: !4)
!15 = !DILocation(line: 11, column: 29, scope: !4)
!16 = !DILocation(line: 11, column: 22, scope: !4)
