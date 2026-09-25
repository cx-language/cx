
%"S<A>" = type {}
%A = type {}

define i32 @main() #0 !dbg !4 {
  %x = alloca %"S<A>", align 8
  call void @_CX1N4mainM4main1SIM4main1AE4initE4void0_(ptr %x), !dbg !7
  call void @_CX1N4mainM4main1SIM4main1AE1sE4void0_(ptr %x), !dbg !8
  ret i32 0
}

define void @_CX1N4mainM4main1SIM4main1AE4initE4void0_(ptr %this) #0 !dbg !9 {
  %1 = alloca %A, align 8
  call void @_CX1N4mainM4main1A4initE4void0_(ptr %1), !dbg !10
  call void @_CX1N4mainM4main1A1hE4void0_(ptr %1), !dbg !11
  ret void
}

define void @_CX1N4mainM4main1SIM4main1AE1sE4void0_(ptr %this) #0 !dbg !12 {
  ret void
}

define void @_CX1N4mainM4main1A4initE4void0_(ptr %this) #0 !dbg !13 {
  ret void
}

define void @_CX1N4mainM4main1A1hE4void0_(ptr %this) #0 !dbg !14 {
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "generic-type-method-call.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 14, type: !5, scopeLine: 14, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 15, column: 13, scope: !4)
!8 = !DILocation(line: 16, column: 7, scope: !4)
!9 = distinct !DISubprogram(name: "init", linkageName: "_CX1N4mainM4main1SIM4main1AE4initE4void0_", scope: !3, file: !3, line: 4, type: !5, scopeLine: 4, spFlags: DISPFlagDefinition, unit: !2)
!10 = !DILocation(line: 5, column: 9, scope: !9)
!11 = !DILocation(line: 5, column: 13, scope: !9)
!12 = distinct !DISubprogram(name: "s", linkageName: "_CX1N4mainM4main1SIM4main1AE1sE4void0_", scope: !3, file: !3, line: 7, type: !5, scopeLine: 7, spFlags: DISPFlagDefinition, unit: !2)
!13 = distinct !DISubprogram(name: "init", linkageName: "_CX1N4mainM4main1A4initE4void0_", scope: !3, file: !3, line: 10, type: !5, scopeLine: 10, spFlags: DISPFlagDefinition, unit: !2)
!14 = distinct !DISubprogram(name: "h", linkageName: "_CX1N4mainM4main1A1hE4void0_", scope: !3, file: !3, line: 11, type: !5, scopeLine: 11, spFlags: DISPFlagDefinition, unit: !2)
