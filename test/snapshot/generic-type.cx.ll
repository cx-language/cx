
%"A<int32>" = type { i32 }
%"A<A<A<bool>>>" = type { %"A<A<bool>>" }
%"A<A<bool>>" = type { %"A<bool>" }
%"A<bool>" = type { i1 }

define i32 @main() #0 !dbg !4 {
  %a = alloca %"A<int32>", align 8
  %aaa = alloca %"A<A<A<bool>>>", align 8
  ret i32 0
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "generic-type.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 7, type: !5, scopeLine: 7, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
