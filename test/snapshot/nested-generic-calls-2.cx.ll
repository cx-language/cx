
%"M<S>" = type { %S }
%S = type { i32 }
%"A<S>" = type {}

define i32 @main() #0 !dbg !4 {
  %m = alloca %"M<S>", align 8
  call void @_CX1N4mainM4main1MIM4main1SE4initE4void0_(ptr %m), !dbg !7
  call void @_CX1N4mainM4main1MIM4main1SE1fE4void0_(ptr %m), !dbg !8
  call void @_CX1N3std10checkLeaksE4void0_(), !dbg !9
  ret i32 0
}

define void @_CX1N4mainM4main1MIM4main1SE4initE4void0_(ptr %this) #0 !dbg !10 {
  ret void
}

define void @_CX1N4mainM4main1MIM4main1SE1fE4void0_(ptr %this) #0 !dbg !11 {
  %a = alloca %"A<S>", align 8
  %1 = alloca %S, align 8
  call void @_CX1N4mainM4main1AIM4main1SE4initE4void0_(ptr %a), !dbg !12
  %2 = call %S @_CX1N4mainM4main1AIM4main1SE1aEM4main1S0_(ptr %a), !dbg !13
  store %S %2, ptr %1, align 4
  call void @_CX1N4mainM4main1S1iE4void0_(ptr %1), !dbg !14
  ret void
}

declare void @_CX1N3std10checkLeaksE4void0_() #0

define void @_CX1N4mainM4main1AIM4main1SE4initE4void0_(ptr %this) #0 !dbg !15 {
  ret void
}

define %S @_CX1N4mainM4main1AIM4main1SE1aEM4main1S0_(ptr %this) #0 !dbg !16 {
  %1 = alloca %S, align 8
  call void @_CX1N4mainM4main1S4initE4void0_(ptr %1), !dbg !17
  %.load = load %S, ptr %1, align 4
  ret %S %.load
}

define void @_CX1N4mainM4main1S1iE4void0_(ptr %this) #0 !dbg !18 {
  ret void
}

define void @_CX1N4mainM4main1S4initE4void0_(ptr %this) #0 !dbg !19 {
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "nested-generic-calls-2.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 30, type: !5, scopeLine: 30, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 31, column: 13, scope: !4)
!8 = !DILocation(line: 32, column: 7, scope: !4)
!9 = !DILocation(line: 30, column: 6, scope: !4)
!10 = distinct !DISubprogram(name: "init", linkageName: "_CX1N4mainM4main1MIM4main1SE4initE4void0_", scope: !3, file: !3, line: 22, type: !5, scopeLine: 22, spFlags: DISPFlagDefinition, unit: !2)
!11 = distinct !DISubprogram(name: "f", linkageName: "_CX1N4mainM4main1MIM4main1SE1fE4void0_", scope: !3, file: !3, line: 24, type: !5, scopeLine: 24, spFlags: DISPFlagDefinition, unit: !2)
!12 = !DILocation(line: 25, column: 17, scope: !11)
!13 = !DILocation(line: 26, column: 11, scope: !11)
!14 = !DILocation(line: 26, column: 15, scope: !11)
!15 = distinct !DISubprogram(name: "init", linkageName: "_CX1N4mainM4main1AIM4main1SE4initE4void0_", scope: !3, file: !3, line: 13, type: !5, scopeLine: 13, spFlags: DISPFlagDefinition, unit: !2)
!16 = distinct !DISubprogram(name: "a", linkageName: "_CX1N4mainM4main1AIM4main1SE1aEM4main1S0_", scope: !3, file: !3, line: 14, type: !5, scopeLine: 14, spFlags: DISPFlagDefinition, unit: !2)
!17 = !DILocation(line: 15, column: 16, scope: !16)
!18 = distinct !DISubprogram(name: "i", linkageName: "_CX1N4mainM4main1S1iE4void0_", scope: !3, file: !3, line: 10, type: !5, scopeLine: 10, spFlags: DISPFlagDefinition, unit: !2)
!19 = distinct !DISubprogram(name: "init", linkageName: "_CX1N4mainM4main1S4initE4void0_", scope: !3, file: !3, line: 9, type: !5, scopeLine: 9, spFlags: DISPFlagDefinition, unit: !2)
