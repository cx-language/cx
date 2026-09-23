
%"X<int>" = type {}

define void @_EN4main1fE() #0 !dbg !4 {
  %index = alloca i32, align 4
  %x = alloca %"X<int>", align 8
  %1 = alloca %"X<int>", align 8
  store i32 0, ptr %index, align 4
  %2 = call %"X<int>" @_EN4main1gE(), !dbg !7
  store %"X<int>" %2, ptr %x, align 1
  call void @_EN4main1XI3intE4initE(ptr %1), !dbg !8
  call void @_EN4main1XI3intE1gE(ptr %1), !dbg !9
  ret void
}

define %"X<int>" @_EN4main1gE() #0 !dbg !10 {
  %x = alloca %"X<int>", align 8
  call void @_EN4main1XI3intE4initE(ptr %x), !dbg !11
  %x.load = load %"X<int>", ptr %x, align 1
  ret %"X<int>" %x.load
}

define void @_EN4main1XI3intE4initE(ptr %this) #0 !dbg !12 {
  ret void
}

define void @_EN4main1XI3intE1gE(ptr %this) #0 !dbg !13 {
  %index = alloca i32, align 4
  store i32 0, ptr %index, align 4
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "reuse-variable-name-in-unrelated-scope.cx")
!4 = distinct !DISubprogram(name: "f", linkageName: "_EN4main1fE", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 5, column: 13, scope: !4)
!8 = !DILocation(line: 6, column: 5, scope: !4)
!9 = !DILocation(line: 6, column: 14, scope: !4)
!10 = distinct !DISubprogram(name: "g", linkageName: "_EN4main1gE", scope: !3, file: !3, line: 9, type: !5, scopeLine: 9, spFlags: DISPFlagDefinition, unit: !2)
!11 = !DILocation(line: 10, column: 13, scope: !10)
!12 = distinct !DISubprogram(name: "init", linkageName: "_EN4main1XI3intE4initE", scope: !3, file: !3, line: 14, type: !5, scopeLine: 14, spFlags: DISPFlagDefinition, unit: !2)
!13 = distinct !DISubprogram(name: "g", linkageName: "_EN4main1XI3intE1gE", scope: !3, file: !3, line: 15, type: !5, scopeLine: 15, spFlags: DISPFlagDefinition, unit: !2)
