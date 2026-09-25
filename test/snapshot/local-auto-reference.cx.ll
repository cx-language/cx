
%Foo = type { i32 }

define i32 @main() #0 !dbg !4 {
  %f = alloca %Foo, align 8
  %rf = alloca ptr, align 8
  %pf = alloca ptr, align 8
  store ptr %f, ptr %rf, align 8
  store ptr %f, ptr %pf, align 8
  call void @_CX1N3std10checkLeaksE4void0_(), !dbg !7
  ret i32 0
}

declare void @_CX1N3std10checkLeaksE4void0_() #0

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "local-auto-reference.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 4, type: !5, scopeLine: 4, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 4, column: 6, scope: !4)
