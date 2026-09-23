
%S = type { i32 }

define i32 @main() #0 !dbg !4 {
  %1 = alloca %S, align 8
  call void @_EN4main1S4initE3int(ptr %1, i32 1), !dbg !7
  call void @_EN4main1S1sE(ptr %1), !dbg !8
  ret i32 0
}

define void @_EN4main1S4initE3int(ptr %this, i32 %i) #0 !dbg !9 {
  %i1 = alloca i32, align 4
  store i32 %i, ptr %i1, align 4
  %i2 = getelementptr inbounds %S, ptr %this, i32 0, i32 0
  %i.load = load i32, ptr %i1, align 4
  store i32 %i.load, ptr %i2, align 4
  ret void
}

define void @_EN4main1S1sE(ptr %this) #0 !dbg !10 {
  %i = getelementptr inbounds %S, ptr %this, i32 0, i32 0
  store i32 2, ptr %i, align 4
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "copyable-type-mutate-this.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 11, type: !5, scopeLine: 11, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 12, column: 5, scope: !4)
!8 = !DILocation(line: 12, column: 10, scope: !4)
!9 = distinct !DISubprogram(name: "init", linkageName: "_EN4main1S4initE3int", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!10 = distinct !DISubprogram(name: "s", linkageName: "_EN4main1S1sE", scope: !3, file: !3, line: 6, type: !5, scopeLine: 6, spFlags: DISPFlagDefinition, unit: !2)
