
define void @_CX1N4main1fE4void1_PM3std4char(ptr %pb) #0 !dbg !4 {
  %pb1 = alloca ptr, align 8
  %b = alloca i1, align 1
  store ptr %pb, ptr %pb1, align 8
  store i1 true, ptr %b, align 1
  %pb.load = load ptr, ptr %pb1, align 8
  %pb.load.load = load i8, ptr %pb.load, align 1
  %1 = icmp ne i8 %pb.load.load, 0
  store i1 %1, ptr %b, align 1
  %b.load = load i1, ptr %b, align 1
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "conversion-to-bool.cx")
!4 = distinct !DISubprogram(name: "f", linkageName: "_CX1N4main1fE4void1_PM3std4char", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
