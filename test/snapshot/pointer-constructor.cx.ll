
define void @_EN4main3fooEPP4charP4char(ptr %p, ptr %x) #0 !dbg !4 {
  %p1 = alloca ptr, align 8
  %x2 = alloca ptr, align 8
  %pp = alloca ptr, align 8
  store ptr %p, ptr %p1, align 8
  store ptr %x, ptr %x2, align 8
  %p.load = load ptr, ptr %p1, align 8
  store ptr %p.load, ptr %pp, align 8
  %pp.load = load ptr, ptr %pp, align 8
  %x.load = load ptr, ptr %x2, align 8
  store ptr %x.load, ptr %pp.load, align 8
  %pp.load3 = load ptr, ptr %pp, align 8
  %pp.load.load = load ptr, ptr %pp.load3, align 8
  store i8 120, ptr %pp.load.load, align 1
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "pointer-constructor.cx")
!4 = distinct !DISubprogram(name: "foo", linkageName: "_EN4main3fooEPP4charP4char", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
