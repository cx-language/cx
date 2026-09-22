
define float @_EN4main1fE5float(float %b) #0 !dbg !4 {
  %b1 = alloca float, align 4
  store float %b, ptr %b1, align 4
  %b.load = load float, ptr %b1, align 4
  %1 = fmul float %b.load, 8.000000e+00
  ret float %1
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "float-constant-auto-cast.cx")
!4 = distinct !DISubprogram(name: "f", linkageName: "_EN4main1fE5float", scope: !3, file: !3, line: 5, type: !5, scopeLine: 5, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
