
%S = type {}

define i32 @main() #0 !dbg !4 {
  %s = alloca %S, align 8
  %t = alloca %S, align 8
  call void @_EN4main1S4initE(ptr %s), !dbg !7
  call void @_EN4main1S4initE5int32(ptr %t, i32 1), !dbg !8
  ret i32 0
}

define void @_EN4main1S4initE(ptr %this) #0 !dbg !9 {
  %i = alloca i32, align 4
  store i32 4, ptr %i, align 4
  ret void
}

define void @_EN4main1S4initE5int32(ptr %this, i32 %foo) #0 !dbg !10 {
  %foo1 = alloca i32, align 4
  %j = alloca i32, align 4
  store i32 %foo, ptr %foo1, align 4
  store i32 5, ptr %j, align 4
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "allocas-in-multiple-constructors.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 13, type: !5, scopeLine: 13, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 14, column: 13, scope: !4)
!8 = !DILocation(line: 15, column: 13, scope: !4)
!9 = distinct !DISubprogram(name: "init", linkageName: "_EN4main1S4initE", scope: !3, file: !3, line: 4, type: !5, scopeLine: 4, spFlags: DISPFlagDefinition, unit: !2)
!10 = distinct !DISubprogram(name: "init", linkageName: "_EN4main1S4initE5int32", scope: !3, file: !3, line: 8, type: !5, scopeLine: 8, spFlags: DISPFlagDefinition, unit: !2)
