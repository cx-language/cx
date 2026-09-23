
%X = type {}

define i32 @main() #0 !dbg !4 {
  %x = alloca %X, align 8
  call void @_EN4main1X4initE3int3int(ptr %x, i32 4, i32 2), !dbg !7
  ret i32 0
}

define void @_EN4main1X4initE3int3int(ptr %this, i32 %a, i32 %b) #0 !dbg !8 {
  %a1 = alloca i32, align 4
  %b2 = alloca i32, align 4
  store i32 %a, ptr %a1, align 4
  store i32 %b, ptr %b2, align 4
  %a.load = load i32, ptr %a1, align 4
  call void @_EN4main1X4initE3int(ptr %this, i32 %a.load), !dbg !9
  ret void
}

define void @_EN4main1X4initE3int(ptr %this, i32 %a) #0 !dbg !10 {
  %a1 = alloca i32, align 4
  store i32 %a, ptr %a1, align 4
  call void @_EN4main1X4initE(ptr %this), !dbg !11
  ret void
}

define void @_EN4main1X4initE(ptr %this) #0 !dbg !12 {
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "constructor-delegation.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 4, column: 13, scope: !4)
!8 = distinct !DISubprogram(name: "init", linkageName: "_EN4main1X4initE3int3int", scope: !3, file: !3, line: 8, type: !5, scopeLine: 8, spFlags: DISPFlagDefinition, unit: !2)
!9 = !DILocation(line: 9, column: 9, scope: !8)
!10 = distinct !DISubprogram(name: "init", linkageName: "_EN4main1X4initE3int", scope: !3, file: !3, line: 12, type: !5, scopeLine: 12, spFlags: DISPFlagDefinition, unit: !2)
!11 = !DILocation(line: 13, column: 9, scope: !10)
!12 = distinct !DISubprogram(name: "init", linkageName: "_EN4main1X4initE", scope: !3, file: !3, line: 16, type: !5, scopeLine: 16, spFlags: DISPFlagDefinition, unit: !2)
