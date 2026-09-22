
define i32 @main() #0 !dbg !4 {
  %foo = alloca i32, align 4
  store i32 0, ptr %foo, align 4
  %foo.load = load i32, ptr %foo, align 4
  %1 = add i32 %foo.load, -1
  store i32 %1, ptr %foo, align 4
  store i32 666, ptr %foo, align 4
  %foo.load1 = load i32, ptr %foo, align 4
  %2 = add i32 %foo.load1, 1
  store i32 %2, ptr %foo, align 4
  ret i32 0
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "assignment.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
