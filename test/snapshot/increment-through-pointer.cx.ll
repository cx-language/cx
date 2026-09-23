
define void @_EN4main1fEP3int(ptr %a) #0 !dbg !4 {
  %a1 = alloca ptr, align 8
  %p = alloca ptr, align 8
  store ptr %a, ptr %a1, align 8
  %a.load = load ptr, ptr %a1, align 8
  store ptr %a.load, ptr %p, align 8
  %p.load = load ptr, ptr %p, align 8
  %p.load.load = load i32, ptr %p.load, align 4
  %1 = add i32 %p.load.load, 1
  store i32 %1, ptr %p.load, align 4
  %a.load2 = load ptr, ptr %a1, align 8
  %a.load.load = load i32, ptr %a.load2, align 4
  %2 = add i32 %a.load.load, -1
  store i32 %2, ptr %a.load2, align 4
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "increment-through-pointer.cx")
!4 = distinct !DISubprogram(name: "f", linkageName: "_EN4main1fEP3int", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
