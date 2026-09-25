
%S = type { i32 }
%T = type { i32 }

define void @_CX1N4main1fE4void1_M4main1S(%S %a) #0 !dbg !4 {
  %a1 = alloca %S, align 8
  store %S %a, ptr %a1, align 4
  ret void
}

define void @_CX1N4main1fE4void1_M4main1T(%T %a) #0 !dbg !7 {
  %a1 = alloca %T, align 8
  store %T %a, ptr %a1, align 4
  ret void
}

define void @_CX1N4maino2plE4void2_M4main1SM4main1S(%S %a, %S %b) #0 !dbg !8 {
  %a1 = alloca %S, align 8
  %b2 = alloca %S, align 8
  store %S %a, ptr %a1, align 4
  store %S %b, ptr %b2, align 4
  ret void
}

define void @_CX1N4maino2plE4void2_M4main1TM4main1T(%T %a, %T %b) #0 !dbg !9 {
  %a1 = alloca %T, align 8
  %b2 = alloca %T, align 8
  store %T %a, ptr %a1, align 4
  store %T %b, ptr %b2, align 4
  ret void
}

define i32 @main() #0 !dbg !10 {
  %s = alloca %S, align 8
  %t = alloca %T, align 8
  call void @_CX1N4mainM4main1S4initE4void0_(ptr %s), !dbg !11
  call void @_CX1N4mainM4main1T4initE4void0_(ptr %t), !dbg !12
  %s.load = load %S, ptr %s, align 4
  call void @_CX1N4main1fE4void1_M4main1S(%S %s.load), !dbg !13
  %t.load = load %T, ptr %t, align 4
  call void @_CX1N4main1fE4void1_M4main1T(%T %t.load), !dbg !14
  %s.load1 = load %S, ptr %s, align 4
  %s.load2 = load %S, ptr %s, align 4
  call void @_CX1N4maino2plE4void2_M4main1SM4main1S(%S %s.load1, %S %s.load2), !dbg !15
  %t.load3 = load %T, ptr %t, align 4
  %t.load4 = load %T, ptr %t, align 4
  call void @_CX1N4maino2plE4void2_M4main1TM4main1T(%T %t.load3, %T %t.load4), !dbg !16
  ret i32 0
}

define void @_CX1N4mainM4main1S4initE4void0_(ptr %this) #0 !dbg !17 {
  %i = getelementptr inbounds %S, ptr %this, i32 0, i32 0
  store i32 0, ptr %i, align 4
  ret void
}

define void @_CX1N4mainM4main1T4initE4void0_(ptr %this) #0 !dbg !18 {
  %i = getelementptr inbounds %T, ptr %this, i32 0, i32 0
  store i32 0, ptr %i, align 4
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "overloading-with-param-types.cx")
!4 = distinct !DISubprogram(name: "f", linkageName: "_CX1N4main1fE4void1_M4main1S", scope: !3, file: !3, line: 13, type: !5, scopeLine: 13, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = distinct !DISubprogram(name: "f", linkageName: "_CX1N4main1fE4void1_M4main1T", scope: !3, file: !3, line: 14, type: !5, scopeLine: 14, spFlags: DISPFlagDefinition, unit: !2)
!8 = distinct !DISubprogram(name: "+", linkageName: "_CX1N4maino2plE4void2_M4main1SM4main1S", scope: !3, file: !3, line: 15, type: !5, scopeLine: 15, spFlags: DISPFlagDefinition, unit: !2)
!9 = distinct !DISubprogram(name: "+", linkageName: "_CX1N4maino2plE4void2_M4main1TM4main1T", scope: !3, file: !3, line: 16, type: !5, scopeLine: 16, spFlags: DISPFlagDefinition, unit: !2)
!10 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 18, type: !5, scopeLine: 18, spFlags: DISPFlagDefinition, unit: !2)
!11 = !DILocation(line: 19, column: 13, scope: !10)
!12 = !DILocation(line: 20, column: 13, scope: !10)
!13 = !DILocation(line: 21, column: 5, scope: !10)
!14 = !DILocation(line: 22, column: 5, scope: !10)
!15 = !DILocation(line: 23, column: 11, scope: !10)
!16 = !DILocation(line: 24, column: 11, scope: !10)
!17 = distinct !DISubprogram(name: "init", linkageName: "_CX1N4mainM4main1S4initE4void0_", scope: !3, file: !3, line: 5, type: !5, scopeLine: 5, spFlags: DISPFlagDefinition, unit: !2)
!18 = distinct !DISubprogram(name: "init", linkageName: "_CX1N4mainM4main1T4initE4void0_", scope: !3, file: !3, line: 10, type: !5, scopeLine: 10, spFlags: DISPFlagDefinition, unit: !2)
