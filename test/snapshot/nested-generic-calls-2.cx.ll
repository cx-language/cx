
%"M<S>" = type { %S }
%S = type { i32 }
%"A<S>" = type {}

define i32 @main() #0 !dbg !4 {
  %m = alloca %"M<S>", align 8
  call void @_EN4main1MI1SE4initE(ptr %m), !dbg !7
  call void @_EN4main1MI1SE1fE(ptr %m), !dbg !8
  ret i32 0
}

define void @_EN4main1MI1SE4initE(ptr %this) #0 !dbg !9 {
  ret void
}

define void @_EN4main1MI1SE1fE(ptr %this) #0 !dbg !10 {
  %a = alloca %"A<S>", align 8
  %1 = alloca %S, align 8
  call void @_EN4main1AI1SE4initE(ptr %a), !dbg !11
  %2 = call %S @_EN4main1AI1SE1aE(ptr %a), !dbg !12
  store %S %2, ptr %1, align 4
  call void @_EN4main1S1iE(ptr %1), !dbg !13
  ret void
}

define void @_EN4main1AI1SE4initE(ptr %this) #0 !dbg !14 {
  ret void
}

define %S @_EN4main1AI1SE1aE(ptr %this) #0 !dbg !15 {
  %1 = alloca %S, align 8
  call void @_EN4main1S4initE(ptr %1), !dbg !16
  %.load = load %S, ptr %1, align 4
  ret %S %.load
}

define void @_EN4main1S1iE(ptr %this) #0 !dbg !17 {
  ret void
}

define void @_EN4main1S4initE(ptr %this) #0 !dbg !18 {
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "nested-generic-calls-2.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 30, type: !5, scopeLine: 30, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 31, column: 13, scope: !4)
!8 = !DILocation(line: 32, column: 7, scope: !4)
!9 = distinct !DISubprogram(name: "init", linkageName: "_EN4main1MI1SE4initE", scope: !3, file: !3, line: 22, type: !5, scopeLine: 22, spFlags: DISPFlagDefinition, unit: !2)
!10 = distinct !DISubprogram(name: "f", linkageName: "_EN4main1MI1SE1fE", scope: !3, file: !3, line: 24, type: !5, scopeLine: 24, spFlags: DISPFlagDefinition, unit: !2)
!11 = !DILocation(line: 25, column: 17, scope: !10)
!12 = !DILocation(line: 26, column: 11, scope: !10)
!13 = !DILocation(line: 26, column: 15, scope: !10)
!14 = distinct !DISubprogram(name: "init", linkageName: "_EN4main1AI1SE4initE", scope: !3, file: !3, line: 13, type: !5, scopeLine: 13, spFlags: DISPFlagDefinition, unit: !2)
!15 = distinct !DISubprogram(name: "a", linkageName: "_EN4main1AI1SE1aE", scope: !3, file: !3, line: 14, type: !5, scopeLine: 14, spFlags: DISPFlagDefinition, unit: !2)
!16 = !DILocation(line: 15, column: 16, scope: !15)
!17 = distinct !DISubprogram(name: "i", linkageName: "_EN4main1S1iE", scope: !3, file: !3, line: 10, type: !5, scopeLine: 10, spFlags: DISPFlagDefinition, unit: !2)
!18 = distinct !DISubprogram(name: "init", linkageName: "_EN4main1S4initE", scope: !3, file: !3, line: 9, type: !5, scopeLine: 9, spFlags: DISPFlagDefinition, unit: !2)
