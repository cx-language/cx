
define i32 @main() #0 !dbg !4 {
  %1 = call i32 @plain(i32 41), !dbg !7
  call void @_EN3std7printlnI3intEE3int(i32 %1), !dbg !8
  ret i32 0
}

declare i32 @plain(i32) #0

define void @_EN3std7printlnI3intEE3int(i32 %value) #0 !dbg !9 {
  %value1 = alloca i32, align 4
  store i32 %value, ptr %value1, align 4
  %value.load = load i32, ptr %value1, align 4
  call void @_EN3std5printI3intEE3int(i32 %value.load), !dbg !11
  call void @_EN3std5printI4charEE4char(i8 10), !dbg !12
  ret void
}

declare void @_EN3std5printI3intEE3int(i32) #0

declare void @_EN3std5printI4charEE4char(i8) #0

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "import-c-float16.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 5, type: !5, scopeLine: 5, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 6, column: 13, scope: !4)
!8 = !DILocation(line: 6, column: 5, scope: !4)
!9 = distinct !DISubprogram(name: "println", linkageName: "_EN3std7printlnI3intEE3int", scope: !10, file: !10, line: 8, type: !5, scopeLine: 8, spFlags: DISPFlagDefinition, unit: !2)
!10 = !DIFile(filename: "stdio.cx")
!11 = !DILocation(line: 9, column: 5, scope: !9)
!12 = !DILocation(line: 10, column: 5, scope: !9)
