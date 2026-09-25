
%S = type { ptr }

define void @_CX1N4main1fE4void1_M4main1S(%S %s) #0 !dbg !4 {
  %s1 = alloca %S, align 8
  store %S %s, ptr %s1, align 8
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "recursive-type.cx")
!4 = distinct !DISubprogram(name: "f", linkageName: "_CX1N4main1fE4void1_M4main1S", scope: !3, file: !3, line: 7, type: !5, scopeLine: 7, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
