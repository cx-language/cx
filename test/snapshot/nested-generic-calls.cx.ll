
define i32 @main() #0 !dbg !4 {
  call void @_CX1N4main1gIM3std5int32EE4void1_M3std5int32(i32 42), !dbg !7
  call void @_CX1N4main1gIM3std4boolEE4void1_M3std4bool(i1 false), !dbg !8
  call void @_CX1N3std10checkLeaksE4void0_(), !dbg !9
  ret i32 0
}

define void @_CX1N4main1gIM3std5int32EE4void1_M3std5int32(i32 %t) #0 !dbg !10 {
  %t1 = alloca i32, align 4
  store i32 %t, ptr %t1, align 4
  %t.load = load i32, ptr %t1, align 4
  call void @_CX1N4main1fIM3std5int32EE4void1_M3std5int32(i32 %t.load), !dbg !11
  ret void
}

define void @_CX1N4main1gIM3std4boolEE4void1_M3std4bool(i1 %t) #0 !dbg !12 {
  %t1 = alloca i1, align 1
  store i1 %t, ptr %t1, align 1
  %t.load = load i1, ptr %t1, align 1
  call void @_CX1N4main1fIM3std4boolEE4void1_M3std4bool(i1 %t.load), !dbg !13
  ret void
}

declare void @_CX1N3std10checkLeaksE4void0_() #0

define void @_CX1N4main1fIM3std5int32EE4void1_M3std5int32(i32 %t) #0 !dbg !14 {
  %t1 = alloca i32, align 4
  store i32 %t, ptr %t1, align 4
  ret void
}

define void @_CX1N4main1fIM3std4boolEE4void1_M3std4bool(i1 %t) #0 !dbg !15 {
  %t1 = alloca i1, align 1
  store i1 %t, ptr %t1, align 1
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "nested-generic-calls.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 4, column: 5, scope: !4)
!8 = !DILocation(line: 5, column: 5, scope: !4)
!9 = !DILocation(line: 3, column: 6, scope: !4)
!10 = distinct !DISubprogram(name: "g", linkageName: "_CX1N4main1gIM3std5int32EE4void1_M3std5int32", scope: !3, file: !3, line: 8, type: !5, scopeLine: 8, spFlags: DISPFlagDefinition, unit: !2)
!11 = !DILocation(line: 9, column: 5, scope: !10)
!12 = distinct !DISubprogram(name: "g", linkageName: "_CX1N4main1gIM3std4boolEE4void1_M3std4bool", scope: !3, file: !3, line: 8, type: !5, scopeLine: 8, spFlags: DISPFlagDefinition, unit: !2)
!13 = !DILocation(line: 9, column: 5, scope: !12)
!14 = distinct !DISubprogram(name: "f", linkageName: "_CX1N4main1fIM3std5int32EE4void1_M3std5int32", scope: !3, file: !3, line: 12, type: !5, scopeLine: 12, spFlags: DISPFlagDefinition, unit: !2)
!15 = distinct !DISubprogram(name: "f", linkageName: "_CX1N4main1fIM3std4boolEE4void1_M3std4bool", scope: !3, file: !3, line: 12, type: !5, scopeLine: 12, spFlags: DISPFlagDefinition, unit: !2)
