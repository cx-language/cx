
%S = type { i32 }

define i32 @main() #0 !dbg !4 {
  %s = alloca %S, align 8
  call void @_CX1N4mainM4main1S4initE4void0_(ptr %s), !dbg !7
  call void @_CX1N4mainM4main1S3fooE4void1_M3std5int32(ptr %s, i32 30), !dbg !8
  call void @_CX1N3std10checkLeaksE4void0_(), !dbg !9
  ret i32 0
}

define void @_CX1N4mainM4main1S4initE4void0_(ptr %this) #0 !dbg !10 {
  ret void
}

define void @_CX1N4mainM4main1S3fooE4void1_M3std5int32(ptr %this, i32 %bar) #0 !dbg !11 {
  %bar1 = alloca i32, align 4
  %a = alloca i32, align 4
  store i32 %bar, ptr %bar1, align 4
  %bar.load = load i32, ptr %bar1, align 4
  store i32 %bar.load, ptr %a, align 4
  call void @_CX1N4mainM4main1S3barE4void0_(ptr %this), !dbg !12
  ret void
}

declare void @_CX1N3std10checkLeaksE4void0_() #0

define void @_CX1N4mainM4main1S3barE4void0_(ptr %this) #0 !dbg !13 {
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "parameter-shadows-member-function.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 13, type: !5, scopeLine: 13, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 14, column: 13, scope: !4)
!8 = !DILocation(line: 15, column: 7, scope: !4)
!9 = !DILocation(line: 13, column: 6, scope: !4)
!10 = distinct !DISubprogram(name: "init", linkageName: "_CX1N4mainM4main1S4initE4void0_", scope: !3, file: !3, line: 5, type: !5, scopeLine: 5, spFlags: DISPFlagDefinition, unit: !2)
!11 = distinct !DISubprogram(name: "foo", linkageName: "_CX1N4mainM4main1S3fooE4void1_M3std5int32", scope: !3, file: !3, line: 7, type: !5, scopeLine: 7, spFlags: DISPFlagDefinition, unit: !2)
!12 = !DILocation(line: 9, column: 9, scope: !11)
!13 = distinct !DISubprogram(name: "bar", linkageName: "_CX1N4mainM4main1S3barE4void0_", scope: !3, file: !3, line: 6, type: !5, scopeLine: 6, spFlags: DISPFlagDefinition, unit: !2)
