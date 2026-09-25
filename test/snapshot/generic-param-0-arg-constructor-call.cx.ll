
%"A<C>" = type {}
%C = type { i32 }

define i32 @main() #0 !dbg !4 {
  %a = alloca %"A<C>", align 8
  call void @_CX1N4mainM4main1AIM4main1CE4initE4void0_(ptr %a), !dbg !7
  call void @_CX1N3std10checkLeaksE4void0_(), !dbg !8
  ret i32 0
}

define void @_CX1N4mainM4main1AIM4main1CE4initE4void0_(ptr %this) #0 !dbg !9 {
  %t = alloca %C, align 8
  call void @_CX1N4mainM4main1C4initE4void0_(ptr %t), !dbg !10
  ret void
}

declare void @_CX1N3std10checkLeaksE4void0_() #0

define void @_CX1N4mainM4main1C4initE4void0_(ptr %this) #0 !dbg !11 {
  %i = getelementptr inbounds %C, ptr %this, i32 0, i32 0
  store i32 0, ptr %i, align 4
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "generic-param-0-arg-constructor-call.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 14, type: !5, scopeLine: 14, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 15, column: 13, scope: !4)
!8 = !DILocation(line: 14, column: 6, scope: !4)
!9 = distinct !DISubprogram(name: "init", linkageName: "_CX1N4mainM4main1AIM4main1CE4initE4void0_", scope: !3, file: !3, line: 9, type: !5, scopeLine: 9, spFlags: DISPFlagDefinition, unit: !2)
!10 = !DILocation(line: 10, column: 17, scope: !9)
!11 = distinct !DISubprogram(name: "init", linkageName: "_CX1N4mainM4main1C4initE4void0_", scope: !3, file: !3, line: 5, type: !5, scopeLine: 5, spFlags: DISPFlagDefinition, unit: !2)
