
define i32 @main() #0 !dbg !4 {
  %foo = alloca i32, align 4
  %bar = alloca i32, align 4
  %qux = alloca i64, align 8
  %qux2 = alloca i8, align 1
  %qux3 = alloca i32, align 4
  %qux4 = alloca i32, align 4
  store i32 -1, ptr %foo, align 4
  store i32 0, ptr %bar, align 4
  store i64 0, ptr %qux, align 8
  store i8 0, ptr %qux2, align 1
  store i32 -1, ptr %qux3, align 4
  %qux3.load = load i32, ptr %qux3, align 4
  store i32 %qux3.load, ptr %qux4, align 4
  ret i32 0
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "import-c-enum.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 5, type: !5, scopeLine: 5, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
