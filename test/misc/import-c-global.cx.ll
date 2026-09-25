
@my_global = external global i32

define i32 @main() #0 !dbg !4 {
  %my_global.load = load i32, ptr @my_global, align 4
  call void @_CX1N3std5printIM3std5int32EE4void1_M3std5int32(i32 %my_global.load), !dbg !7
  ret i32 0
}

declare void @_CX1N3std5printIM3std5int32EE4void1_M3std5int32(i32) #0

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "import-c-global.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 5, type: !5, scopeLine: 5, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 6, column: 5, scope: !4)
