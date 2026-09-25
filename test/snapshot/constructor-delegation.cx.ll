
%X = type {}

define i32 @main() #0 !dbg !4 {
  %x = alloca %X, align 8
  call void @_CX1N4mainM4main1X4initE4void2_M3std5int32M3std5int32(ptr %x, i32 4, i32 2), !dbg !7
  call void @_CX1N3std10checkLeaksE4void0_(), !dbg !8
  ret i32 0
}

define void @_CX1N4mainM4main1X4initE4void2_M3std5int32M3std5int32(ptr %this, i32 %a, i32 %b) #0 !dbg !9 {
  %a1 = alloca i32, align 4
  %b2 = alloca i32, align 4
  store i32 %a, ptr %a1, align 4
  store i32 %b, ptr %b2, align 4
  %a.load = load i32, ptr %a1, align 4
  call void @_CX1N4mainM4main1X4initE4void1_M3std5int32(ptr %this, i32 %a.load), !dbg !10
  ret void
}

declare void @_CX1N3std10checkLeaksE4void0_() #0

define void @_CX1N4mainM4main1X4initE4void1_M3std5int32(ptr %this, i32 %a) #0 !dbg !11 {
  %a1 = alloca i32, align 4
  store i32 %a, ptr %a1, align 4
  call void @_CX1N4mainM4main1X4initE4void0_(ptr %this), !dbg !12
  ret void
}

define void @_CX1N4mainM4main1X4initE4void0_(ptr %this) #0 !dbg !13 {
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
!8 = !DILocation(line: 3, column: 6, scope: !4)
!9 = distinct !DISubprogram(name: "init", linkageName: "_CX1N4mainM4main1X4initE4void2_M3std5int32M3std5int32", scope: !3, file: !3, line: 8, type: !5, scopeLine: 8, spFlags: DISPFlagDefinition, unit: !2)
!10 = !DILocation(line: 9, column: 9, scope: !9)
!11 = distinct !DISubprogram(name: "init", linkageName: "_CX1N4mainM4main1X4initE4void1_M3std5int32", scope: !3, file: !3, line: 12, type: !5, scopeLine: 12, spFlags: DISPFlagDefinition, unit: !2)
!12 = !DILocation(line: 13, column: 9, scope: !11)
!13 = distinct !DISubprogram(name: "init", linkageName: "_CX1N4mainM4main1X4initE4void0_", scope: !3, file: !3, line: 16, type: !5, scopeLine: 16, spFlags: DISPFlagDefinition, unit: !2)
