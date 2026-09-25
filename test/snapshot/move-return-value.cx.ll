
%X = type {}

define %X @_CX1N4main1fEM4main1X0_() #0 !dbg !4 {
  %x = alloca %X, align 8
  call void @_CX1N4mainM4main1X4initE4void0_(ptr %x), !dbg !7
  %x.load = load %X, ptr %x, align 1
  ret %X %x.load
}

define void @_CX1N4mainM4main1X4initE4void0_(ptr %this) #0 !dbg !8 {
  ret void
}

define void @_CX1N4mainM4main1X6deinitE4void0_(ptr %this) #0 !dbg !9 {
  ret void
}

define i32 @main() #0 !dbg !10 {
  %a = alloca %X, align 8
  %1 = call %X @_CX1N4main1fEM4main1X0_(), !dbg !11
  store %X %1, ptr %a, align 1
  call void @_CX1N4mainM4main1X6deinitE4void0_(ptr %a), !dbg !12
  ret i32 0
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "move-return-value.cx")
!4 = distinct !DISubprogram(name: "f", linkageName: "_CX1N4main1fEM4main1X0_", scope: !3, file: !3, line: 7, type: !5, scopeLine: 7, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 8, column: 13, scope: !4)
!8 = distinct !DISubprogram(name: "init", linkageName: "_CX1N4mainM4main1X4initE4void0_", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!9 = distinct !DISubprogram(name: "deinit", linkageName: "_CX1N4mainM4main1X6deinitE4void0_", scope: !3, file: !3, line: 4, type: !5, scopeLine: 4, spFlags: DISPFlagDefinition, unit: !2)
!10 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 12, type: !5, scopeLine: 12, spFlags: DISPFlagDefinition, unit: !2)
!11 = !DILocation(line: 13, column: 13, scope: !10)
!12 = !DILocation(line: 12, column: 6, scope: !10)
