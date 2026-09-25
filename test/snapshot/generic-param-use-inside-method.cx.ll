
%"S<int32>" = type {}

define i32 @main() #0 !dbg !4 {
  %s = alloca %"S<int32>", align 8
  call void @_CX1N4mainM4main1SIM3std5int32E4initE4void0_(ptr %s), !dbg !7
  call void @_CX1N4mainM4main1SIM3std5int32E1fE4void0_(ptr %s), !dbg !8
  call void @_CX1N3std10checkLeaksE4void0_(), !dbg !9
  ret i32 0
}

define void @_CX1N4mainM4main1SIM3std5int32E4initE4void0_(ptr %this) #0 !dbg !10 {
  ret void
}

define void @_CX1N4mainM4main1SIM3std5int32E1fE4void0_(ptr %this) #0 !dbg !11 {
  %t = alloca i32, align 4
  call void @_CX1N4mainM4main1SIM3std5int32E1gE4void0_(ptr %this), !dbg !12
  ret void
}

declare void @_CX1N3std10checkLeaksE4void0_() #0

define void @_CX1N4mainM4main1SIM3std5int32E1gE4void0_(ptr %this) #0 !dbg !13 {
  %t2 = alloca i32, align 4
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "generic-param-use-inside-method.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 14, type: !5, scopeLine: 14, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 15, column: 13, scope: !4)
!8 = !DILocation(line: 16, column: 7, scope: !4)
!9 = !DILocation(line: 14, column: 6, scope: !4)
!10 = distinct !DISubprogram(name: "init", linkageName: "_CX1N4mainM4main1SIM3std5int32E4initE4void0_", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!11 = distinct !DISubprogram(name: "f", linkageName: "_CX1N4mainM4main1SIM3std5int32E1fE4void0_", scope: !3, file: !3, line: 4, type: !5, scopeLine: 4, spFlags: DISPFlagDefinition, unit: !2)
!12 = !DILocation(line: 5, column: 9, scope: !11)
!13 = distinct !DISubprogram(name: "g", linkageName: "_CX1N4mainM4main1SIM3std5int32E1gE4void0_", scope: !3, file: !3, line: 9, type: !5, scopeLine: 9, spFlags: DISPFlagDefinition, unit: !2)
