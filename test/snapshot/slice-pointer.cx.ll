
define void @_EN4main3fooEP5SliceI5int32E(ptr %a) #0 !dbg !4 {
  %a1 = alloca ptr, align 8
  store ptr %a, ptr %a1, align 8
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "slice-pointer.cx")
!4 = distinct !DISubprogram(name: "foo", linkageName: "_EN4main3fooEP5SliceI5int32E", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
