
%"F<int32, bool>" = type { i32, i1 }

define i32 @main() #0 !dbg !4 {
  %f = alloca %"F<int32, bool>", align 8
  call void @_EN4main1FI5int324boolE4initE(ptr %f), !dbg !7
  call void @_EN4main1FI5int324boolE3fooE(ptr %f), !dbg !8
  ret i32 0
}

define void @_EN4main1FI5int324boolE4initE(ptr %this) #0 !dbg !9 {
  ret void
}

define void @_EN4main1FI5int324boolE3fooE(ptr %this) #0 !dbg !10 {
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "generic-type-member-func.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 14, type: !5, scopeLine: 14, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 15, column: 13, scope: !4)
!8 = !DILocation(line: 16, column: 7, scope: !4)
!9 = distinct !DISubprogram(name: "init", linkageName: "_EN4main1FI5int324boolE4initE", scope: !3, file: !3, line: 6, type: !5, scopeLine: 6, spFlags: DISPFlagDefinition, unit: !2)
!10 = distinct !DISubprogram(name: "foo", linkageName: "_EN4main1FI5int324boolE3fooE", scope: !3, file: !3, line: 10, type: !5, scopeLine: 10, spFlags: DISPFlagDefinition, unit: !2)
