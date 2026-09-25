
define i32 @main() #0 !dbg !4 {
  %u = alloca i32, align 4
  %u.load = load i32, ptr %u, align 4
  call void @_CX1N4main1fIM3std6uint32EE4void2_M3std6uint32M3std6uint32(i32 0, i32 %u.load), !dbg !7
  call void @_CX1N3std10checkLeaksE4void0_(), !dbg !8
  ret i32 0
}

define void @_CX1N4main1fIM3std6uint32EE4void2_M3std6uint32M3std6uint32(i32 %a, i32 %b) #0 !dbg !9 {
  %a1 = alloca i32, align 4
  %b2 = alloca i32, align 4
  store i32 %a, ptr %a1, align 4
  store i32 %b, ptr %b2, align 4
  ret void
}

declare void @_CX1N3std10checkLeaksE4void0_() #0

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "infer-generic-args-reinterpret-int-literal.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 5, column: 5, scope: !4)
!8 = !DILocation(line: 3, column: 6, scope: !4)
!9 = distinct !DISubprogram(name: "f", linkageName: "_CX1N4main1fIM3std6uint32EE4void2_M3std6uint32M3std6uint32", scope: !3, file: !3, line: 8, type: !5, scopeLine: 8, spFlags: DISPFlagDefinition, unit: !2)
