
define i32 @main() #0 !dbg !4 {
  %1 = call float @_CX1N4main1fIM3std7float32EEM3std7float321_M3std7float32(float 0.000000e+00), !dbg !7
  ret i32 0
}

define float @_CX1N4main1fIM3std7float32EEM3std7float321_M3std7float32(float %a) #0 !dbg !8 {
  %a1 = alloca float, align 4
  store float %a, ptr %a1, align 4
  %a.load = load float, ptr %a1, align 4
  %1 = fneg float %a.load
  ret float %1
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "negate-generic-parameter.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 7, type: !5, scopeLine: 7, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 8, column: 5, scope: !4)
!8 = distinct !DISubprogram(name: "f", linkageName: "_CX1N4main1fIM3std7float32EEM3std7float321_M3std7float32", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
