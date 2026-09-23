
define i32 @main() #0 !dbg !4 {
  %i = alloca i16, align 2
  %f = alloca float, align 4
  %u = alloca i64, align 8
  %s = alloca i32, align 4
  store i16 42, ptr %i, align 2
  %i.load = load i16, ptr %i, align 2
  %1 = uitofp i16 %i.load to float
  store float %1, ptr %f, align 4
  %f.load = load float, ptr %f, align 4
  %2 = fptosi float %f.load to i64
  store i64 %2, ptr %u, align 8
  store i32 1, ptr %s, align 4
  ret i32 0
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "numeric-conversion.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
