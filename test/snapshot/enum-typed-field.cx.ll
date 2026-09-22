
%S = type { i32, i32 }

define i32 @_EN4main1fE1S(%S %s) #0 !dbg !4 {
  %s1 = alloca %S, align 8
  store %S %s, ptr %s1, align 4
  %f = getelementptr inbounds %S, ptr %s1, i32 0, i32 1
  %f.load = load i32, ptr %f, align 4
  ret i32 %f.load
}

define i32 @_EN4main1gEP1S(ptr %s) #0 !dbg !7 {
  %s1 = alloca ptr, align 8
  store ptr %s, ptr %s1, align 8
  %s.load = load ptr, ptr %s1, align 8
  %f = getelementptr inbounds %S, ptr %s.load, i32 0, i32 1
  %f.load = load i32, ptr %f, align 4
  ret i32 %f.load
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "enum-typed-field.cx")
!4 = distinct !DISubprogram(name: "f", linkageName: "_EN4main1fE1S", scope: !3, file: !3, line: 10, type: !5, scopeLine: 10, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = distinct !DISubprogram(name: "g", linkageName: "_EN4main1gEP1S", scope: !3, file: !3, line: 14, type: !5, scopeLine: 14, spFlags: DISPFlagDefinition, unit: !2)
