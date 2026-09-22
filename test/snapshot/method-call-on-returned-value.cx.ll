
%S = type { i32 }
%C = type { i32 }

define %S @_EN4main1sE() #0 !dbg !4 {
  %1 = alloca %S, align 8
  call void @_EN4main1S4initE(ptr %1), !dbg !7
  %.load = load %S, ptr %1, align 4
  ret %S %.load
}

define void @_EN4main1S4initE(ptr %this) #0 !dbg !8 {
  %i = getelementptr inbounds %S, ptr %this, i32 0, i32 0
  store i32 0, ptr %i, align 4
  ret void
}

define %C @_EN4main1cE() #0 !dbg !9 {
  %1 = alloca %C, align 8
  call void @_EN4main1C4initE(ptr %1), !dbg !10
  %.load = load %C, ptr %1, align 4
  ret %C %.load
}

define void @_EN4main1C4initE(ptr %this) #0 !dbg !11 {
  %i = getelementptr inbounds %C, ptr %this, i32 0, i32 0
  store i32 0, ptr %i, align 4
  ret void
}

define i32 @main() #0 !dbg !12 {
  %1 = alloca %S, align 8
  %2 = alloca %C, align 8
  %3 = alloca %S, align 8
  %4 = alloca %C, align 8
  %5 = call %S @_EN4main1sE(), !dbg !13
  store %S %5, ptr %1, align 4
  call void @_EN4main1S1fE(ptr %1), !dbg !14
  %6 = call %C @_EN4main1cE(), !dbg !15
  store %C %6, ptr %2, align 4
  call void @_EN4main1C1fE(ptr %2), !dbg !16
  call void @_EN4main1S4initE(ptr %3), !dbg !17
  call void @_EN4main1S1fE(ptr %3), !dbg !18
  call void @_EN4main1C4initE(ptr %4), !dbg !19
  call void @_EN4main1C1fE(ptr %4), !dbg !20
  ret i32 0
}

define void @_EN4main1S1fE(ptr %this) #0 !dbg !21 {
  ret void
}

define void @_EN4main1C1fE(ptr %this) #0 !dbg !22 {
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "method-call-on-returned-value.cx")
!4 = distinct !DISubprogram(name: "s", linkageName: "_EN4main1sE", scope: !3, file: !3, line: 15, type: !5, scopeLine: 15, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 15, column: 16, scope: !4)
!8 = distinct !DISubprogram(name: "init", linkageName: "_EN4main1S4initE", scope: !3, file: !3, line: 5, type: !5, scopeLine: 5, spFlags: DISPFlagDefinition, unit: !2)
!9 = distinct !DISubprogram(name: "c", linkageName: "_EN4main1cE", scope: !3, file: !3, line: 16, type: !5, scopeLine: 16, spFlags: DISPFlagDefinition, unit: !2)
!10 = !DILocation(line: 16, column: 16, scope: !9)
!11 = distinct !DISubprogram(name: "init", linkageName: "_EN4main1C4initE", scope: !3, file: !3, line: 11, type: !5, scopeLine: 11, spFlags: DISPFlagDefinition, unit: !2)
!12 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 18, type: !5, scopeLine: 18, spFlags: DISPFlagDefinition, unit: !2)
!13 = !DILocation(line: 19, column: 5, scope: !12)
!14 = !DILocation(line: 19, column: 9, scope: !12)
!15 = !DILocation(line: 20, column: 5, scope: !12)
!16 = !DILocation(line: 20, column: 9, scope: !12)
!17 = !DILocation(line: 21, column: 5, scope: !12)
!18 = !DILocation(line: 21, column: 9, scope: !12)
!19 = !DILocation(line: 22, column: 5, scope: !12)
!20 = !DILocation(line: 22, column: 9, scope: !12)
!21 = distinct !DISubprogram(name: "f", linkageName: "_EN4main1S1fE", scope: !3, file: !3, line: 6, type: !5, scopeLine: 6, spFlags: DISPFlagDefinition, unit: !2)
!22 = distinct !DISubprogram(name: "f", linkageName: "_EN4main1C1fE", scope: !3, file: !3, line: 12, type: !5, scopeLine: 12, spFlags: DISPFlagDefinition, unit: !2)
