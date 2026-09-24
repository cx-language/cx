
%"S<int32>" = type { ptr }
%"S<bool>" = type { ptr }

define i32 @main() #0 !dbg !4 {
  %1 = alloca %"S<int32>", align 8
  %2 = alloca %"S<bool>", align 8
  call void @_EN4main1SI5int32E4initE(ptr %1), !dbg !7
  call void @_EN4main1SI4boolE4initE(ptr %2), !dbg !8
  ret i32 0
}

define void @_EN4main1SI5int32E4initE(ptr %this) #0 !dbg !9 {
  %p = getelementptr inbounds %"S<int32>", ptr %this, i32 0, i32 0
  store ptr null, ptr %p, align 8
  ret void
}

define void @_EN4main1SI4boolE4initE(ptr %this) #0 !dbg !10 {
  %p = getelementptr inbounds %"S<bool>", ptr %this, i32 0, i32 0
  store ptr null, ptr %p, align 8
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "generic-param-as-null-type.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 11, type: !5, scopeLine: 11, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 12, column: 9, scope: !4)
!8 = !DILocation(line: 13, column: 9, scope: !4)
!9 = distinct !DISubprogram(name: "init", linkageName: "_EN4main1SI5int32E4initE", scope: !3, file: !3, line: 6, type: !5, scopeLine: 6, spFlags: DISPFlagDefinition, unit: !2)
!10 = distinct !DISubprogram(name: "init", linkageName: "_EN4main1SI4boolE4initE", scope: !3, file: !3, line: 6, type: !5, scopeLine: 6, spFlags: DISPFlagDefinition, unit: !2)
