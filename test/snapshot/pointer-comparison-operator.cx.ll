
define void @_CX1N4main3fooE4void2_OPKM3std5int32PM3std5int32(ptr %a, ptr %b) #0 !dbg !4 {
  %a1 = alloca ptr, align 8
  %b2 = alloca ptr, align 8
  store ptr %a, ptr %a1, align 8
  store ptr %b, ptr %b2, align 8
  %a.load = load ptr, ptr %a1, align 8
  %b.load = load ptr, ptr %b2, align 8
  %1 = icmp eq ptr %a.load, %b.load
  %a.load3 = load ptr, ptr %a1, align 8
  %b.load4 = load ptr, ptr %b2, align 8
  %2 = icmp ne ptr %a.load3, %b.load4
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "pointer-comparison-operator.cx")
!4 = distinct !DISubprogram(name: "foo", linkageName: "_CX1N4main3fooE4void2_OPKM3std5int32PM3std5int32", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
