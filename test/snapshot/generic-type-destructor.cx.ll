
%"C<int>" = type {}
%"C<bool>" = type {}

define i32 @main() #0 !dbg !4 {
  %i = alloca %"C<int>", align 8
  %b = alloca %"C<bool>", align 8
  call void @_EN4main1CI3intE4initE(ptr %i), !dbg !7
  call void @_EN4main1CI4boolE4initE(ptr %b), !dbg !8
  call void @_EN4main1CI4boolE6deinitE(ptr %b), !dbg !9
  call void @_EN4main1CI3intE6deinitE(ptr %i), !dbg !9
  ret i32 0
}

define void @_EN4main1CI3intE4initE(ptr %this) #0 !dbg !10 {
  ret void
}

define void @_EN4main1CI4boolE4initE(ptr %this) #0 !dbg !11 {
  ret void
}

define void @_EN4main1CI4boolE6deinitE(ptr %this) #0 !dbg !12 {
  ret void
}

define void @_EN4main1CI3intE6deinitE(ptr %this) #0 !dbg !13 {
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "generic-type-destructor.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 7, type: !5, scopeLine: 7, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 8, column: 13, scope: !4)
!8 = !DILocation(line: 9, column: 13, scope: !4)
!9 = !DILocation(line: 7, column: 6, scope: !4)
!10 = distinct !DISubprogram(name: "init", linkageName: "_EN4main1CI3intE4initE", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!11 = distinct !DISubprogram(name: "init", linkageName: "_EN4main1CI4boolE4initE", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!12 = distinct !DISubprogram(name: "deinit", linkageName: "_EN4main1CI4boolE6deinitE", scope: !3, file: !3, line: 4, type: !5, scopeLine: 4, spFlags: DISPFlagDefinition, unit: !2)
!13 = distinct !DISubprogram(name: "deinit", linkageName: "_EN4main1CI3intE6deinitE", scope: !3, file: !3, line: 4, type: !5, scopeLine: 4, spFlags: DISPFlagDefinition, unit: !2)
