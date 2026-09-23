
%"A<int>" = type { ptr }

define i32 @main() #0 !dbg !4 {
  %a = alloca %"A<int>", align 8
  call void @_EN4main1AI3intE4initE(ptr %a), !dbg !7
  ret i32 0
}

define void @_EN4main1AI3intE4initE(ptr %this) #0 !dbg !8 {
  %a = getelementptr inbounds %"A<int>", ptr %this, i32 0, i32 0
  store ptr null, ptr %a, align 8
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "generic-type-member-using-generic-param.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 8, type: !5, scopeLine: 8, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 8, column: 23, scope: !4)
!8 = distinct !DISubprogram(name: "init", linkageName: "_EN4main1AI3intE4initE", scope: !3, file: !3, line: 5, type: !5, scopeLine: 5, spFlags: DISPFlagDefinition, unit: !2)
