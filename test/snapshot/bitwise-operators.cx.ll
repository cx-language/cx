
define i32 @main() #0 !dbg !4 {
  %a = alloca i32, align 4
  %b = alloca i32, align 4
  %c = alloca i32, align 4
  %d = alloca i32, align 4
  %a.load = load i32, ptr %a, align 4
  %b.load = load i32, ptr %b, align 4
  %1 = and i32 %a.load, %b.load
  %a.load1 = load i32, ptr %a, align 4
  %b.load2 = load i32, ptr %b, align 4
  %2 = or i32 %a.load1, %b.load2
  %a.load3 = load i32, ptr %a, align 4
  %b.load4 = load i32, ptr %b, align 4
  %3 = xor i32 %a.load3, %b.load4
  %a.load5 = load i32, ptr %a, align 4
  %b.load6 = load i32, ptr %b, align 4
  %4 = shl i32 %a.load5, %b.load6
  %a.load7 = load i32, ptr %a, align 4
  %b.load8 = load i32, ptr %b, align 4
  %5 = ashr i32 %a.load7, %b.load8
  %c.load = load i32, ptr %c, align 4
  %d.load = load i32, ptr %d, align 4
  %6 = lshr i32 %c.load, %d.load
  %a.load9 = load i32, ptr %a, align 4
  %7 = xor i32 %a.load9, -1
  ret i32 0
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "bitwise-operators.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
