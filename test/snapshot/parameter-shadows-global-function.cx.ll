
define void @_CX1N4main3barE4void0_() #0 !dbg !4 {
  ret void
}

define void @_CX1N4main3fooE4void1_M3std5int32(i32 %bar) #0 !dbg !7 {
  %bar1 = alloca i32, align 4
  store i32 %bar, ptr %bar1, align 4
  call void @_CX1N4main3barE4void0_(), !dbg !8
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "parameter-shadows-global-function.cx")
!4 = distinct !DISubprogram(name: "bar", linkageName: "_CX1N4main3barE4void0_", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = distinct !DISubprogram(name: "foo", linkageName: "_CX1N4main3fooE4void1_M3std5int32", scope: !3, file: !3, line: 4, type: !5, scopeLine: 4, spFlags: DISPFlagDefinition, unit: !2)
!8 = !DILocation(line: 5, column: 5, scope: !7)
