
define i32 @main() #0 !dbg !4 {
  call void @_EN4main1gI5int32EE5int32(i32 42), !dbg !7
  call void @_EN4main1gI4boolEE4bool(i1 false), !dbg !8
  ret i32 0
}

define void @_EN4main1gI5int32EE5int32(i32 %t) #0 !dbg !9 {
  %t1 = alloca i32, align 4
  store i32 %t, ptr %t1, align 4
  %t.load = load i32, ptr %t1, align 4
  call void @_EN4main1fI5int32EE5int32(i32 %t.load), !dbg !10
  ret void
}

define void @_EN4main1gI4boolEE4bool(i1 %t) #0 !dbg !11 {
  %t1 = alloca i1, align 1
  store i1 %t, ptr %t1, align 1
  %t.load = load i1, ptr %t1, align 1
  call void @_EN4main1fI4boolEE4bool(i1 %t.load), !dbg !12
  ret void
}

define void @_EN4main1fI5int32EE5int32(i32 %t) #0 !dbg !13 {
  %t1 = alloca i32, align 4
  store i32 %t, ptr %t1, align 4
  ret void
}

define void @_EN4main1fI4boolEE4bool(i1 %t) #0 !dbg !14 {
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
!9 = distinct !DISubprogram(name: "g", linkageName: "_EN4main1gI5int32EE5int32", scope: !3, file: !3, line: 8, type: !5, scopeLine: 8, spFlags: DISPFlagDefinition, unit: !2)
!10 = !DILocation(line: 9, column: 5, scope: !9)
!11 = distinct !DISubprogram(name: "g", linkageName: "_EN4main1gI4boolEE4bool", scope: !3, file: !3, line: 8, type: !5, scopeLine: 8, spFlags: DISPFlagDefinition, unit: !2)
!12 = !DILocation(line: 9, column: 5, scope: !11)
!13 = distinct !DISubprogram(name: "f", linkageName: "_EN4main1fI5int32EE5int32", scope: !3, file: !3, line: 12, type: !5, scopeLine: 12, spFlags: DISPFlagDefinition, unit: !2)
!14 = distinct !DISubprogram(name: "f", linkageName: "_EN4main1fI4boolEE4bool", scope: !3, file: !3, line: 12, type: !5, scopeLine: 12, spFlags: DISPFlagDefinition, unit: !2)
