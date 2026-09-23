
define void @_EN4main1fEP3int3int(ptr %foo, i32 %bar) #0 !dbg !4 {
  %foo1 = alloca ptr, align 8
  %bar2 = alloca i32, align 4
  store ptr %foo, ptr %foo1, align 8
  store i32 %bar, ptr %bar2, align 4
  %foo.load = load ptr, ptr %foo1, align 8
  %foo.load.load = load i32, ptr %foo.load, align 4
  %bar.load = load i32, ptr %bar2, align 4
  %1 = icmp slt i32 %foo.load.load, %bar.load
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "compare-pointer-to-integer.cx")
!4 = distinct !DISubprogram(name: "f", linkageName: "_EN4main1fEP3int3int", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
