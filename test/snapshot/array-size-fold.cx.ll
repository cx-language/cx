
define i32 @main() #0 !dbg !4 {
  %a = alloca [3 x i32], align 4
  store [3 x i32] [i32 1, i32 2, i32 3], ptr %a, align 4
  call void @_CX1N3std7printlnIM3std5int32EE4void1_M3std5int32(i32 3), !dbg !7
  call void @_CX1N3std10checkLeaksE4void0_(), !dbg !8
  ret i32 0
}

define void @_CX1N3std7printlnIM3std5int32EE4void1_M3std5int32(i32 %value) #0 !dbg !9 {
  %value1 = alloca i32, align 4
  store i32 %value, ptr %value1, align 4
  %value.load = load i32, ptr %value1, align 4
  call void @_CX1N3std5printIM3std5int32EE4void1_M3std5int32(i32 %value.load), !dbg !11
  call void @_CX1N3std5printIM3std4charEE4void1_M3std4char(i8 10), !dbg !12
  ret void
}

declare void @_CX1N3std10checkLeaksE4void0_() #0

declare void @_CX1N3std5printIM3std5int32EE4void1_M3std5int32(i32) #0

declare void @_CX1N3std5printIM3std4charEE4void1_M3std4char(i8) #0

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "array-size-fold.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 5, column: 5, scope: !4)
!8 = !DILocation(line: 3, column: 6, scope: !4)
!9 = distinct !DISubprogram(name: "println", linkageName: "_CX1N3std7printlnIM3std5int32EE4void1_M3std5int32", scope: !10, file: !10, line: 8, type: !5, scopeLine: 8, spFlags: DISPFlagDefinition, unit: !2)
!10 = !DIFile(filename: "stdio.cx")
!11 = !DILocation(line: 9, column: 5, scope: !9)
!12 = !DILocation(line: 10, column: 5, scope: !9)
