
define void @_CX1N4main1fE4void3_PM3std7float64M3std7float64M3std7float64(ptr %foo, double %bar, double %baz) #0 !dbg !4 {
  %foo1 = alloca ptr, align 8
  %bar2 = alloca double, align 8
  %baz3 = alloca double, align 8
  store ptr %foo, ptr %foo1, align 8
  store double %bar, ptr %bar2, align 8
  store double %baz, ptr %baz3, align 8
  %foo.load = load ptr, ptr %foo1, align 8
  %foo.load4 = load ptr, ptr %foo1, align 8
  %foo.load.load = load double, ptr %foo.load4, align 8
  %bar.load = load double, ptr %bar2, align 8
  %baz.load = load double, ptr %baz3, align 8
  %1 = fmul double %bar.load, %baz.load
  %2 = fsub double 1.000000e+00, %1
  %3 = fmul double %foo.load.load, %2
  store double %3, ptr %foo.load, align 8
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "compound-assignment-rhs-precedence.cx")
!4 = distinct !DISubprogram(name: "f", linkageName: "_CX1N4main1fE4void3_PM3std7float64M3std7float64M3std7float64", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
