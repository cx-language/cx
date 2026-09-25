
%Y = type {}

define void @_CX1N4main1fE4void2_M4main1YPM4main1Y(%Y %a, ptr %b) #0 !dbg !4 {
  %a1 = alloca %Y, align 8
  %b2 = alloca ptr, align 8
  store %Y %a, ptr %a1, align 1
  store ptr %b, ptr %b2, align 8
  %b.load = load ptr, ptr %b2, align 8
  call void @_CX1N4mainM4main1Y6deinitE4void0_(ptr %b.load), !dbg !7
  %a.load = load %Y, ptr %a1, align 1
  store %Y %a.load, ptr %b.load, align 1
  ret void
}

define void @_CX1N4mainM4main1Y6deinitE4void0_(ptr %this) #0 !dbg !8 {
  ret void
}

define void @_CX1N4main1gE4void1_M4main1Y(%Y %a) #0 !dbg !9 {
  %a1 = alloca %Y, align 8
  %b = alloca %Y, align 8
  store %Y %a, ptr %a1, align 1
  %a.load = load %Y, ptr %a1, align 1
  store %Y %a.load, ptr %b, align 1
  call void @_CX1N4mainM4main1Y6deinitE4void0_(ptr %b), !dbg !10
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "move-type-with-destructor.cx")
!4 = distinct !DISubprogram(name: "f", linkageName: "_CX1N4main1fE4void2_M4main1YPM4main1Y", scope: !3, file: !3, line: 7, type: !5, scopeLine: 7, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 7, column: 6, scope: !4)
!8 = distinct !DISubprogram(name: "deinit", linkageName: "_CX1N4mainM4main1Y6deinitE4void0_", scope: !3, file: !3, line: 4, type: !5, scopeLine: 4, spFlags: DISPFlagDefinition, unit: !2)
!9 = distinct !DISubprogram(name: "g", linkageName: "_CX1N4main1gE4void1_M4main1Y", scope: !3, file: !3, line: 11, type: !5, scopeLine: 11, spFlags: DISPFlagDefinition, unit: !2)
!10 = !DILocation(line: 11, column: 6, scope: !9)
