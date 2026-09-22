
define i32 @main() #0 !dbg !4 {
  call void @_EN4main1fI3intEE(), !dbg !7
  ret i32 0
}

define void @_EN4main1fI3intEE() #0 !dbg !8 {
  %i = alloca i32, align 4
  store i32 0, ptr %i, align 4
  %i.load = load i32, ptr %i, align 4
  %1 = add i32 %i.load, 1
  store i32 %1, ptr %i, align 4
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "mutable-local-var-in-generic-function.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 8, type: !5, scopeLine: 8, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 9, column: 5, scope: !4)
!8 = distinct !DISubprogram(name: "f", linkageName: "_EN4main1fI3intEE", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
