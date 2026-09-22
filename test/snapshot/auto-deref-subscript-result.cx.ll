
%"S<int>" = type { i32 }

define void @_EN4main1fEP1SI3intE(ptr %s) #0 !dbg !4 {
  %s1 = alloca ptr, align 8
  store ptr %s, ptr %s1, align 8
  %s.load = load ptr, ptr %s1, align 8
  %1 = call ptr @_EN4main1SI3intEixE3int(ptr %s.load, i32 0), !dbg !7
  %.load = load i32, ptr %1, align 4
  %2 = add i32 %.load, 1
  store i32 %2, ptr %1, align 4
  ret void
}

define ptr @_EN4main1SI3intEixE3int(ptr %this, i32 %i) #0 !dbg !8 {
  %i1 = alloca i32, align 4
  store i32 %i, ptr %i1, align 4
  %t = getelementptr inbounds %"S<int>", ptr %this, i32 0, i32 0
  ret ptr %t
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "auto-deref-subscript-result.cx")
!4 = distinct !DISubprogram(name: "f", linkageName: "_EN4main1fEP1SI3intE", scope: !3, file: !3, line: 11, type: !5, scopeLine: 11, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 12, column: 6, scope: !4)
!8 = distinct !DISubprogram(name: "[]", linkageName: "_EN4main1SI3intEixE3int", scope: !3, file: !3, line: 6, type: !5, scopeLine: 6, spFlags: DISPFlagDefinition, unit: !2)
