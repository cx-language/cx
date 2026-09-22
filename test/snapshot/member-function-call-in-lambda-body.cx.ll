
define i32 @main() #0 !dbg !4 {
  %lambda = alloca ptr, align 8
  store ptr @_EN4main9__lambda0EP1X, ptr %lambda, align 8
  ret i32 0
}

define i32 @_EN4main9__lambda0EP1X(ptr %x) #0 !dbg !7 {
  %x1 = alloca ptr, align 8
  store ptr %x, ptr %x1, align 8
  %x.load = load ptr, ptr %x1, align 8
  %1 = call i32 @_EN4main1X3fooE(ptr %x.load), !dbg !8
  ret i32 %1
}

define i32 @_EN4main1X3fooE(ptr %this) #0 !dbg !9 {
  ret i32 42
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "member-function-call-in-lambda-body.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 9, type: !5, scopeLine: 9, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = distinct !DISubprogram(name: "__lambda0", linkageName: "_EN4main9__lambda0EP1X", scope: !3, file: !3, line: 10, type: !5, scopeLine: 10, spFlags: DISPFlagDefinition, unit: !2)
!8 = !DILocation(line: 10, column: 30, scope: !7)
!9 = distinct !DISubprogram(name: "foo", linkageName: "_EN4main1X3fooE", scope: !3, file: !3, line: 4, type: !5, scopeLine: 4, spFlags: DISPFlagDefinition, unit: !2)
