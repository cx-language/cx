
%B = type {}

define i32 @main() #0 !dbg !4 {
  %1 = alloca %B, align 8
  call void @_EN4main1B4initE(ptr %1), !dbg !7
  call void @_EN4main1B1fE(ptr %1), !dbg !8
  ret i32 0
}

define void @_EN4main1B4initE(ptr %this) #0 !dbg !9 {
  ret void
}

define void @_EN4main1B1fE(ptr %this) #0 !dbg !11 {
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "a.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 1, type: !5, scopeLine: 1, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 2, column: 5, scope: !4)
!8 = !DILocation(line: 2, column: 9, scope: !4)
!9 = distinct !DISubprogram(name: "init", linkageName: "_EN4main1B4initE", scope: !10, file: !10, line: 1, type: !5, scopeLine: 1, spFlags: DISPFlagDefinition, unit: !2)
!10 = !DIFile(filename: "b.cx")
!11 = distinct !DISubprogram(name: "f", linkageName: "_EN4main1B1fE", scope: !10, file: !10, line: 2, type: !5, scopeLine: 2, spFlags: DISPFlagDefinition, unit: !2)
