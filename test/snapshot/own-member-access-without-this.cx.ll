
%A = type { i32, i32 }
%B = type { i32, i32 }

define i32 @main() #0 !dbg !4 {
  %a = alloca %A, align 8
  %b = alloca %B, align 8
  call void @_CX1N4mainM4main1A4initE4void0_(ptr %a), !dbg !7
  %1 = call i32 @_CX1N4mainM4main1A3fooEM3std5int320_(ptr %a), !dbg !8
  call void @_CX1N4mainM4main1A3barE4void0_(ptr %a), !dbg !9
  call void @_CX1N4mainM4main1B4initE4void0_(ptr %b), !dbg !10
  %2 = call i32 @_CX1N4mainM4main1B3fooEM3std5int320_(ptr %b), !dbg !11
  call void @_CX1N4mainM4main1B3barE4void0_(ptr %b), !dbg !12
  call void @_CX1N3std10checkLeaksE4void0_(), !dbg !13
  ret i32 0
}

define void @_CX1N4mainM4main1A4initE4void0_(ptr %this) #0 !dbg !14 {
  %j = getelementptr inbounds %A, ptr %this, i32 0, i32 1
  store i32 42, ptr %j, align 4
  ret void
}

define i32 @_CX1N4mainM4main1A3fooEM3std5int320_(ptr %this) #0 !dbg !15 {
  %j = getelementptr inbounds %A, ptr %this, i32 0, i32 1
  %j.load = load i32, ptr %j, align 4
  ret i32 %j.load
}

define void @_CX1N4mainM4main1A3barE4void0_(ptr %this) #0 !dbg !16 {
  %j = getelementptr inbounds %A, ptr %this, i32 0, i32 1
  store i32 1, ptr %j, align 4
  ret void
}

define void @_CX1N4mainM4main1B4initE4void0_(ptr %this) #0 !dbg !17 {
  %j = getelementptr inbounds %B, ptr %this, i32 0, i32 1
  store i32 42, ptr %j, align 4
  ret void
}

define i32 @_CX1N4mainM4main1B3fooEM3std5int320_(ptr %this) #0 !dbg !18 {
  %j = getelementptr inbounds %B, ptr %this, i32 0, i32 1
  %j.load = load i32, ptr %j, align 4
  ret i32 %j.load
}

define void @_CX1N4mainM4main1B3barE4void0_(ptr %this) #0 !dbg !19 {
  %j = getelementptr inbounds %B, ptr %this, i32 0, i32 1
  store i32 1, ptr %j, align 4
  ret void
}

declare void @_CX1N3std10checkLeaksE4void0_() #0

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "own-member-access-without-this.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 37, type: !5, scopeLine: 37, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 38, column: 13, scope: !4)
!8 = !DILocation(line: 39, column: 7, scope: !4)
!9 = !DILocation(line: 40, column: 7, scope: !4)
!10 = !DILocation(line: 41, column: 13, scope: !4)
!11 = !DILocation(line: 42, column: 7, scope: !4)
!12 = !DILocation(line: 43, column: 7, scope: !4)
!13 = !DILocation(line: 37, column: 6, scope: !4)
!14 = distinct !DISubprogram(name: "init", linkageName: "_CX1N4mainM4main1A4initE4void0_", scope: !3, file: !3, line: 7, type: !5, scopeLine: 7, spFlags: DISPFlagDefinition, unit: !2)
!15 = distinct !DISubprogram(name: "foo", linkageName: "_CX1N4mainM4main1A3fooEM3std5int320_", scope: !3, file: !3, line: 11, type: !5, scopeLine: 11, spFlags: DISPFlagDefinition, unit: !2)
!16 = distinct !DISubprogram(name: "bar", linkageName: "_CX1N4mainM4main1A3barE4void0_", scope: !3, file: !3, line: 15, type: !5, scopeLine: 15, spFlags: DISPFlagDefinition, unit: !2)
!17 = distinct !DISubprogram(name: "init", linkageName: "_CX1N4mainM4main1B4initE4void0_", scope: !3, file: !3, line: 24, type: !5, scopeLine: 24, spFlags: DISPFlagDefinition, unit: !2)
!18 = distinct !DISubprogram(name: "foo", linkageName: "_CX1N4mainM4main1B3fooEM3std5int320_", scope: !3, file: !3, line: 28, type: !5, scopeLine: 28, spFlags: DISPFlagDefinition, unit: !2)
!19 = distinct !DISubprogram(name: "bar", linkageName: "_CX1N4mainM4main1B3barE4void0_", scope: !3, file: !3, line: 32, type: !5, scopeLine: 32, spFlags: DISPFlagDefinition, unit: !2)
