
define void @_CX1N4main3fooE4void2_M3std5int32M3std6uint32(i32 %i, i32 %u) #0 !dbg !4 {
  %i1 = alloca i32, align 4
  %u2 = alloca i32, align 4
  %a = alloca i1, align 1
  %b = alloca i32, align 4
  %c = alloca i32, align 4
  store i32 %i, ptr %i1, align 4
  store i32 %u, ptr %u2, align 4
  %i.load = load i32, ptr %i1, align 4
  %i.load3 = load i32, ptr %i1, align 4
  %1 = icmp slt i32 %i.load, %i.load3
  store i1 %1, ptr %a, align 1
  %u.load = load i32, ptr %u2, align 4
  %u.load4 = load i32, ptr %u2, align 4
  %2 = icmp ult i32 %u.load, %u.load4
  store i1 %2, ptr %a, align 1
  %i.load5 = load i32, ptr %i1, align 4
  %i.load6 = load i32, ptr %i1, align 4
  %3 = icmp sgt i32 %i.load5, %i.load6
  store i1 %3, ptr %a, align 1
  %u.load7 = load i32, ptr %u2, align 4
  %u.load8 = load i32, ptr %u2, align 4
  %4 = icmp ugt i32 %u.load7, %u.load8
  store i1 %4, ptr %a, align 1
  %i.load9 = load i32, ptr %i1, align 4
  %i.load10 = load i32, ptr %i1, align 4
  %5 = icmp sle i32 %i.load9, %i.load10
  store i1 %5, ptr %a, align 1
  %u.load11 = load i32, ptr %u2, align 4
  %u.load12 = load i32, ptr %u2, align 4
  %6 = icmp ule i32 %u.load11, %u.load12
  store i1 %6, ptr %a, align 1
  %i.load13 = load i32, ptr %i1, align 4
  %i.load14 = load i32, ptr %i1, align 4
  %7 = icmp sge i32 %i.load13, %i.load14
  store i1 %7, ptr %a, align 1
  %u.load15 = load i32, ptr %u2, align 4
  %u.load16 = load i32, ptr %u2, align 4
  %8 = icmp uge i32 %u.load15, %u.load16
  store i1 %8, ptr %a, align 1
  %i.load17 = load i32, ptr %i1, align 4
  %i.load18 = load i32, ptr %i1, align 4
  %9 = sdiv i32 %i.load17, %i.load18
  store i32 %9, ptr %b, align 4
  %u.load19 = load i32, ptr %u2, align 4
  %u.load20 = load i32, ptr %u2, align 4
  %10 = udiv i32 %u.load19, %u.load20
  store i32 %10, ptr %c, align 4
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "signed-vs-unsigned-arithmetic.cx")
!4 = distinct !DISubprogram(name: "foo", linkageName: "_CX1N4main3fooE4void2_M3std5int32M3std6uint32", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
