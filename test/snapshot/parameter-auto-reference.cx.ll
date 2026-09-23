
%S = type {}

define void @_EN4main1fE1S(%S %s) #0 !dbg !4 {
  %s1 = alloca %S, align 8
  store %S %s, ptr %s1, align 1
  call void @_EN4main1gEP1S(ptr %s1), !dbg !7
  ret void
}

define void @_EN4main1gEP1S(ptr %s) #0 !dbg !8 {
  %s1 = alloca ptr, align 8
  store ptr %s, ptr %s1, align 8
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "parameter-auto-reference.cx")
!4 = distinct !DISubprogram(name: "f", linkageName: "_EN4main1fE1S", scope: !3, file: !3, line: 5, type: !5, scopeLine: 5, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 6, column: 5, scope: !4)
!8 = distinct !DISubprogram(name: "g", linkageName: "_EN4main1gEP1S", scope: !3, file: !3, line: 9, type: !5, scopeLine: 9, spFlags: DISPFlagDefinition, unit: !2)
