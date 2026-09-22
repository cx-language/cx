
%S = type {}

define void @_EN4maineqE1S1S(%S %a, %S %b) #0 !dbg !4 {
  %a1 = alloca %S, align 8
  %b2 = alloca %S, align 8
  store %S %a, ptr %a1, align 1
  store %S %b, ptr %b2, align 1
  ret void
}

define void @_EN4main1fE1S(%S %s) #0 !dbg !7 {
  %s1 = alloca %S, align 8
  store %S %s, ptr %s1, align 1
  %s.load = load %S, ptr %s1, align 1
  %s.load2 = load %S, ptr %s1, align 1
  call void @_EN4maineqE1S1S(%S %s.load, %S %s.load2), !dbg !8
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "overloaded-generic-operator-function.cx")
!4 = distinct !DISubprogram(name: "==", linkageName: "_EN4maineqE1S1S", scope: !3, file: !3, line: 11, type: !5, scopeLine: 11, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = distinct !DISubprogram(name: "f", linkageName: "_EN4main1fE1S", scope: !3, file: !3, line: 13, type: !5, scopeLine: 13, spFlags: DISPFlagDefinition, unit: !2)
!8 = !DILocation(line: 14, column: 11, scope: !7)
