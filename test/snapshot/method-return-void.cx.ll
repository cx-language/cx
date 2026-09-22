
%C = type {}

define i32 @main() #0 !dbg !4 {
  %c = alloca %C, align 8
  call void @_EN4main1C4initE(ptr %c), !dbg !7
  call void @_EN4main1C1fE(ptr %c), !dbg !8
  ret i32 0
}

define void @_EN4main1C4initE(ptr %this) #0 !dbg !9 {
  ret void
}

define void @_EN4main1C1fE(ptr %this) #0 !dbg !10 {
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "method-return-void.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 9, type: !5, scopeLine: 9, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 10, column: 13, scope: !4)
!8 = !DILocation(line: 11, column: 7, scope: !4)
!9 = distinct !DISubprogram(name: "init", linkageName: "_EN4main1C4initE", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!10 = distinct !DISubprogram(name: "f", linkageName: "_EN4main1C1fE", scope: !3, file: !3, line: 4, type: !5, scopeLine: 4, spFlags: DISPFlagDefinition, unit: !2)
