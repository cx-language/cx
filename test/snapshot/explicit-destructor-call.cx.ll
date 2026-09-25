
define void @_CX1N4main1fE4void1_PM4main1A(ptr %a) #0 !dbg !4 {
  %a1 = alloca ptr, align 8
  store ptr %a, ptr %a1, align 8
  %a.load = load ptr, ptr %a1, align 8
  call void @_CX1N4mainM4main1A6deinitE4void0_(ptr %a.load), !dbg !7
  ret void
}

define void @_CX1N4mainM4main1A6deinitE4void0_(ptr %this) #0 !dbg !8 {
  ret void
}

define void @_CX1N4main1gE4void1_PM4main1B(ptr %b) #0 !dbg !9 {
  %b1 = alloca ptr, align 8
  store ptr %b, ptr %b1, align 8
  ret void
}

define void @_CX1N4main1hE4void1_PM3std4char(ptr %c) #0 !dbg !10 {
  %c1 = alloca ptr, align 8
  store ptr %c, ptr %c1, align 8
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "explicit-destructor-call.cx")
!4 = distinct !DISubprogram(name: "f", linkageName: "_CX1N4main1fE4void1_PM4main1A", scope: !3, file: !3, line: 7, type: !5, scopeLine: 7, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 8, column: 7, scope: !4)
!8 = distinct !DISubprogram(name: "deinit", linkageName: "_CX1N4mainM4main1A6deinitE4void0_", scope: !3, file: !3, line: 4, type: !5, scopeLine: 4, spFlags: DISPFlagDefinition, unit: !2)
!9 = distinct !DISubprogram(name: "g", linkageName: "_CX1N4main1gE4void1_PM4main1B", scope: !3, file: !3, line: 13, type: !5, scopeLine: 13, spFlags: DISPFlagDefinition, unit: !2)
!10 = distinct !DISubprogram(name: "h", linkageName: "_CX1N4main1hE4void1_PM3std4char", scope: !3, file: !3, line: 17, type: !5, scopeLine: 17, spFlags: DISPFlagDefinition, unit: !2)
