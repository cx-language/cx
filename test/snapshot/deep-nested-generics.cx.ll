
%"S<R>" = type { %"A<A<R>>" }
%"A<A<R>>" = type { %"A<R>" }
%"A<R>" = type { %R }
%R = type { i32 }

define i32 @main() #0 !dbg !4 {
  %s = alloca %"S<R>", align 8
  call void @_CX1N4mainM4main1SIM4main1RE4initE4void0_(ptr %s), !dbg !7
  call void @_CX1N4mainM4main1SIM4main1RE1sE4void0_(ptr %s), !dbg !8
  call void @_CX1N3std10checkLeaksE4void0_(), !dbg !9
  ret i32 0
}

define void @_CX1N4mainM4main1SIM4main1RE4initE4void0_(ptr %this) #0 !dbg !10 {
  ret void
}

define void @_CX1N4mainM4main1SIM4main1RE1sE4void0_(ptr %this) #0 !dbg !11 {
  %t = alloca %"A<R>", align 8
  %tt = alloca %R, align 8
  %a = getelementptr inbounds %"S<R>", ptr %this, i32 0, i32 0
  %1 = call %"A<R>" @_CX1N4mainM4main1AIM4main1AIM4main1REEo2ixEM4main1AIM4main1RE1_M3std5int32(ptr %a, i32 0), !dbg !12
  store %"A<R>" %1, ptr %t, align 4
  %2 = call %R @_CX1N4mainM4main1AIM4main1REo2ixEM4main1R1_M3std5int32(ptr %t, i32 0), !dbg !13
  store %R %2, ptr %tt, align 4
  %a1 = getelementptr inbounds %"S<R>", ptr %this, i32 0, i32 0
  %3 = call i32 @_CX1N4mainM4main1R1hEM3std5int320_(ptr %tt), !dbg !14
  %4 = call %"A<R>" @_CX1N4mainM4main1AIM4main1AIM4main1REEo2ixEM4main1AIM4main1RE1_M3std5int32(ptr %a1, i32 %3), !dbg !15
  ret void
}

declare void @_CX1N3std10checkLeaksE4void0_() #0

define %"A<R>" @_CX1N4mainM4main1AIM4main1AIM4main1REEo2ixEM4main1AIM4main1RE1_M3std5int32(ptr %this, i32 %i) #0 !dbg !16 {
  %i1 = alloca i32, align 4
  store i32 %i, ptr %i1, align 4
  %t = getelementptr inbounds %"A<A<R>>", ptr %this, i32 0, i32 0
  %t.load = load %"A<R>", ptr %t, align 4
  ret %"A<R>" %t.load
}

define %R @_CX1N4mainM4main1AIM4main1REo2ixEM4main1R1_M3std5int32(ptr %this, i32 %i) #0 !dbg !17 {
  %i1 = alloca i32, align 4
  store i32 %i, ptr %i1, align 4
  %t = getelementptr inbounds %"A<R>", ptr %this, i32 0, i32 0
  %t.load = load %R, ptr %t, align 4
  ret %R %t.load
}

define i32 @_CX1N4mainM4main1R1hEM3std5int320_(ptr %this) #0 !dbg !18 {
  %i = getelementptr inbounds %R, ptr %this, i32 0, i32 0
  %i.load = load i32, ptr %i, align 4
  ret i32 %i.load
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"DEBUG-FORMAT", i32 0}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "deep-nested-generics.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 41, type: !5, scopeLine: 41, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 42, column: 13, scope: !4)
!8 = !DILocation(line: 43, column: 7, scope: !4)
!9 = !DILocation(line: 41, column: 6, scope: !4)
!10 = distinct !DISubprogram(name: "init", linkageName: "_CX1N4mainM4main1SIM4main1RE4initE4void0_", scope: !3, file: !3, line: 30, type: !5, scopeLine: 30, spFlags: DISPFlagDefinition, unit: !2)
!11 = distinct !DISubprogram(name: "s", linkageName: "_CX1N4mainM4main1SIM4main1RE1sE4void0_", scope: !3, file: !3, line: 34, type: !5, scopeLine: 34, spFlags: DISPFlagDefinition, unit: !2)
!12 = !DILocation(line: 35, column: 18, scope: !11)
!13 = !DILocation(line: 36, column: 19, scope: !11)
!14 = !DILocation(line: 37, column: 18, scope: !11)
!15 = !DILocation(line: 37, column: 14, scope: !11)
!16 = distinct !DISubprogram(name: "[]", linkageName: "_CX1N4mainM4main1AIM4main1AIM4main1REEo2ixEM4main1AIM4main1RE1_M3std5int32", scope: !3, file: !3, line: 22, type: !5, scopeLine: 22, spFlags: DISPFlagDefinition, unit: !2)
!17 = distinct !DISubprogram(name: "[]", linkageName: "_CX1N4mainM4main1AIM4main1REo2ixEM4main1R1_M3std5int32", scope: !3, file: !3, line: 22, type: !5, scopeLine: 22, spFlags: DISPFlagDefinition, unit: !2)
!18 = distinct !DISubprogram(name: "h", linkageName: "_CX1N4mainM4main1R1hEM3std5int320_", scope: !3, file: !3, line: 10, type: !5, scopeLine: 10, spFlags: DISPFlagDefinition, unit: !2)
