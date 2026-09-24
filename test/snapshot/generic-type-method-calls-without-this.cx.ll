
%"S<int32>" = type {}
%"S<float32>" = type {}

define i32 @main() #0 !dbg !4 {
  %1 = alloca %"S<int32>", align 8
  %2 = alloca %"S<float32>", align 8
  call void @_EN4main1SI5int32E4initE(ptr %1), !dbg !7
  call void @_EN4main1SI5int32E1fE(ptr %1), !dbg !8
  call void @_EN4main1SI7float32E4initE(ptr %2), !dbg !9
  call void @_EN4main1SI7float32E1fE(ptr %2), !dbg !10
  ret i32 0
}

define void @_EN4main1SI5int32E4initE(ptr %this) #0 !dbg !11 {
  ret void
}

define void @_EN4main1SI5int32E1fE(ptr %this) #0 !dbg !12 {
  call void @_EN4main1SI5int32E1gE(ptr %this), !dbg !13
  ret void
}

define void @_EN4main1SI7float32E4initE(ptr %this) #0 !dbg !14 {
  ret void
}

define void @_EN4main1SI7float32E1fE(ptr %this) #0 !dbg !15 {
  call void @_EN4main1SI7float32E1gE(ptr %this), !dbg !16
  ret void
}

define void @_EN4main1SI5int32E1gE(ptr %this) #0 !dbg !17 {
  ret void
}

define void @_EN4main1SI7float32E1gE(ptr %this) #0 !dbg !18 {
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "generic-type-method-calls-without-this.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 8, type: !5, scopeLine: 8, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 9, column: 5, scope: !4)
!8 = !DILocation(line: 9, column: 14, scope: !4)
!9 = !DILocation(line: 10, column: 5, scope: !4)
!10 = !DILocation(line: 10, column: 16, scope: !4)
!11 = distinct !DISubprogram(name: "init", linkageName: "_EN4main1SI5int32E4initE", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!12 = distinct !DISubprogram(name: "f", linkageName: "_EN4main1SI5int32E1fE", scope: !3, file: !3, line: 4, type: !5, scopeLine: 4, spFlags: DISPFlagDefinition, unit: !2)
!13 = !DILocation(line: 4, column: 16, scope: !12)
!14 = distinct !DISubprogram(name: "init", linkageName: "_EN4main1SI7float32E4initE", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!15 = distinct !DISubprogram(name: "f", linkageName: "_EN4main1SI7float32E1fE", scope: !3, file: !3, line: 4, type: !5, scopeLine: 4, spFlags: DISPFlagDefinition, unit: !2)
!16 = !DILocation(line: 4, column: 16, scope: !15)
!17 = distinct !DISubprogram(name: "g", linkageName: "_EN4main1SI5int32E1gE", scope: !3, file: !3, line: 5, type: !5, scopeLine: 5, spFlags: DISPFlagDefinition, unit: !2)
!18 = distinct !DISubprogram(name: "g", linkageName: "_EN4main1SI7float32E1gE", scope: !3, file: !3, line: 5, type: !5, scopeLine: 5, spFlags: DISPFlagDefinition, unit: !2)
