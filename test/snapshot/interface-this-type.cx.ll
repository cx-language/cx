
%S = type { ptr }

define i32 @main() #0 !dbg !4 {
  %a = alloca %S, align 8
  %b = alloca %S, align 8
  call void @_CX1N4mainM4main1S4initE4void0_(ptr %a), !dbg !7
  call void @_CX1N4mainM4main1S4initE4void0_(ptr %b), !dbg !8
  %1 = call ptr @_CX1N4mainM4main1S3fooEPM4main1S1_PM4main1S(ptr %a, ptr %b), !dbg !9
  call void @_CX1N4main1fIM4main1SEE4void1_PM4main1S(ptr %1), !dbg !10
  call void @_CX1N3std10checkLeaksE4void0_(), !dbg !11
  ret i32 0
}

define void @_CX1N4mainM4main1S4initE4void0_(ptr %this) #0 !dbg !12 {
  %that = getelementptr inbounds %S, ptr %this, i32 0, i32 0
  store ptr %this, ptr %that, align 8
  ret void
}

define ptr @_CX1N4mainM4main1S3fooEPM4main1S1_PM4main1S(ptr %this, ptr %a) #0 !dbg !13 {
  %a1 = alloca ptr, align 8
  store ptr %a, ptr %a1, align 8
  ret ptr %this
}

define void @_CX1N4main1fIM4main1SEE4void1_PM4main1S(ptr %a) #0 !dbg !14 {
  %a1 = alloca ptr, align 8
  store ptr %a, ptr %a1, align 8
  %a.load = load ptr, ptr %a1, align 8
  %a.load2 = load ptr, ptr %a1, align 8
  %1 = call ptr @_CX1N4mainM4main1S3barEPM4main1S0_(ptr %a.load2), !dbg !15
  %2 = call ptr @_CX1N4mainM4main1S3fooEPM4main1S1_PM4main1S(ptr %a.load, ptr %1), !dbg !16
  ret void
}

declare void @_CX1N3std10checkLeaksE4void0_() #0

define ptr @_CX1N4mainM4main1S3barEPM4main1S0_(ptr %this) #0 !dbg !17 {
  %1 = call ptr @_CX1N4mainM4main1S3fooEPM4main1S1_PM4main1S(ptr %this, ptr %this), !dbg !18
  %2 = call ptr @_CX1N4mainM4main1S3fooEPM4main1S1_PM4main1S(ptr %this, ptr %1), !dbg !19
  ret ptr %2
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "interface-this-type.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 21, type: !5, scopeLine: 21, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 22, column: 13, scope: !4)
!8 = !DILocation(line: 23, column: 13, scope: !4)
!9 = !DILocation(line: 24, column: 9, scope: !4)
!10 = !DILocation(line: 24, column: 5, scope: !4)
!11 = !DILocation(line: 21, column: 6, scope: !4)
!12 = distinct !DISubprogram(name: "init", linkageName: "_CX1N4mainM4main1S4initE4void0_", scope: !3, file: !3, line: 14, type: !5, scopeLine: 14, spFlags: DISPFlagDefinition, unit: !2)
!13 = distinct !DISubprogram(name: "foo", linkageName: "_CX1N4mainM4main1S3fooEPM4main1S1_PM4main1S", scope: !3, file: !3, line: 16, type: !5, scopeLine: 16, spFlags: DISPFlagDefinition, unit: !2)
!14 = distinct !DISubprogram(name: "f", linkageName: "_CX1N4main1fIM4main1SEE4void1_PM4main1S", scope: !3, file: !3, line: 27, type: !5, scopeLine: 27, spFlags: DISPFlagDefinition, unit: !2)
!15 = !DILocation(line: 28, column: 13, scope: !14)
!16 = !DILocation(line: 28, column: 7, scope: !14)
!17 = distinct !DISubprogram(name: "bar", linkageName: "_CX1N4mainM4main1S3barEPM4main1S0_", scope: !3, file: !3, line: 8, type: !5, scopeLine: 8, spFlags: DISPFlagDefinition, unit: !2)
!18 = !DILocation(line: 9, column: 20, scope: !17)
!19 = !DILocation(line: 9, column: 16, scope: !17)
