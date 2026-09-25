
%S = type { ptr }

define i32 @main() #0 !dbg !4 {
  %s = alloca %S, align 8
  call void @_CX1N19opaque_2Dpointer__hM19opaque_2Dpointer__h1S4initE4void1_OPM19opaque_2Dpointer__h6Opaque(ptr %s, ptr null), !dbg !7
  ret i32 0
}

declare void @_CX1N19opaque_2Dpointer__hM19opaque_2Dpointer__h1S4initE4void1_OPM19opaque_2Dpointer__h6Opaque(ptr, ptr) #0

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "opaque-pointer.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 5, type: !5, scopeLine: 5, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 6, column: 13, scope: !4)
