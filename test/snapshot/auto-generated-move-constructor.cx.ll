
%X = type { i32 }

define void @_CX1N4main3fooE4void2_PM4main1XM4main1X(ptr %p, %X %x) #0 !dbg !4 {
  %p1 = alloca ptr, align 8
  %x2 = alloca %X, align 8
  store ptr %p, ptr %p1, align 8
  store %X %x, ptr %x2, align 4
  %p.load = load ptr, ptr %p1, align 8
  %x.load = load %X, ptr %x2, align 4
  store %X %x.load, ptr %p.load, align 4
  ret void
}

define void @_CX1N4mainM4main1X6deinitE4void0_(ptr %this) #0 !dbg !7 {
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "auto-generated-move-constructor.cx")
!4 = distinct !DISubprogram(name: "foo", linkageName: "_CX1N4main3fooE4void2_PM4main1XM4main1X", scope: !3, file: !3, line: 8, type: !5, scopeLine: 8, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = distinct !DISubprogram(name: "deinit", linkageName: "_CX1N4mainM4main1X6deinitE4void0_", scope: !3, file: !3, line: 5, type: !5, scopeLine: 5, spFlags: DISPFlagDefinition, unit: !2)
