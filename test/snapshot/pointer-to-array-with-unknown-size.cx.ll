
define i32 @main() #0 !dbg !4 {
  %a = alloca [3 x i32], align 4
  %p = alloca ptr, align 8
  %t = alloca i32, align 4
  store [3 x i32] [i32 1, i32 2, i32 3], ptr %a, align 4
  %1 = getelementptr inbounds [3 x i32], ptr %a, i32 0, i32 0
  store ptr %1, ptr %p, align 8
  store ptr %a, ptr %p, align 8
  store ptr %a, ptr %p, align 8
  %p.load = load ptr, ptr %p, align 8
  %2 = getelementptr inbounds i32, ptr %p.load, i32 1
  %.load = load i32, ptr %2, align 4
  store i32 %.load, ptr %t, align 4
  ret i32 0
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "pointer-to-array-with-unknown-size.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
