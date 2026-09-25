
define i1 @_CX1N4maino2ltEM3std4bool2_RM4main1XRM4main1X(ptr %a, ptr %b) #0 !dbg !4 {
  %a1 = alloca ptr, align 8
  %b2 = alloca ptr, align 8
  store ptr %a, ptr %a1, align 8
  store ptr %b, ptr %b2, align 8
  ret i1 true
}

define void @_CX1N4main2fxE4void4_PM4main1XPM4main1XP4voidP4void(ptr %a, ptr %b, ptr %v1, ptr %v2) #0 !dbg !7 {
  %a1 = alloca ptr, align 8
  %b2 = alloca ptr, align 8
  %v13 = alloca ptr, align 8
  %v24 = alloca ptr, align 8
  store ptr %a, ptr %a1, align 8
  store ptr %b, ptr %b2, align 8
  store ptr %v1, ptr %v13, align 8
  store ptr %v2, ptr %v24, align 8
  %a.load = load ptr, ptr %a1, align 8
  %b.load = load ptr, ptr %b2, align 8
  %1 = icmp eq ptr %a.load, %b.load
  %a.load5 = load ptr, ptr %a1, align 8
  %b.load6 = load ptr, ptr %b2, align 8
  %2 = icmp ult ptr %a.load5, %b.load6
  %a.load7 = load ptr, ptr %a1, align 8
  %b.load8 = load ptr, ptr %b2, align 8
  %3 = call i1 @_CX1N4maino2ltEM3std4bool2_RM4main1XRM4main1X(ptr %a.load7, ptr %b.load8), !dbg !8
  %v1.load = load ptr, ptr %v13, align 8
  %v2.load = load ptr, ptr %v24, align 8
  %4 = icmp ne ptr %v1.load, %v2.load
  %v1.load9 = load ptr, ptr %v13, align 8
  %v2.load10 = load ptr, ptr %v24, align 8
  %5 = icmp uge ptr %v1.load9, %v2.load10
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "pointer-comparison.cx")
!4 = distinct !DISubprogram(name: "<", linkageName: "_CX1N4maino2ltEM3std4bool2_RM4main1XRM4main1X", scope: !3, file: !3, line: 5, type: !5, scopeLine: 5, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = distinct !DISubprogram(name: "fx", linkageName: "_CX1N4main2fxE4void4_PM4main1XPM4main1XP4voidP4void", scope: !3, file: !3, line: 17, type: !5, scopeLine: 17, spFlags: DISPFlagDefinition, unit: !2)
!8 = !DILocation(line: 20, column: 8, scope: !7)
