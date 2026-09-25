
define void @_CX1N4main3fooE4void1_RM3std5int32(ptr %ref_i) #0 !dbg !4 {
  %ref_i1 = alloca ptr, align 8
  store ptr %ref_i, ptr %ref_i1, align 8
  ret void
}

define i32 @main() #0 !dbg !7 {
  %bar = alloca i32, align 4
  store i32 42, ptr %bar, align 4
  call void @_CX1N4main3fooE4void1_RM3std5int32(ptr %bar), !dbg !8
  call void @_CX1N3std10checkLeaksE4void0_(), !dbg !9
  ret i32 0
}

declare void @_CX1N3std10checkLeaksE4void0_() #0

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "builtin-type-auto-reference.cx")
!4 = distinct !DISubprogram(name: "foo", linkageName: "_CX1N4main3fooE4void1_RM3std5int32", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 5, type: !5, scopeLine: 5, spFlags: DISPFlagDefinition, unit: !2)
!8 = !DILocation(line: 7, column: 5, scope: !7)
!9 = !DILocation(line: 5, column: 6, scope: !7)
