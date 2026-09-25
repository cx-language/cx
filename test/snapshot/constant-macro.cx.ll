
define void @_CX1N4main3fooE4void1_M3std5int32(i32 %i) #0 !dbg !4 {
  %i1 = alloca i32, align 4
  store i32 %i, ptr %i1, align 4
  %i.load = load i32, ptr %i1, align 4
  switch i32 %i.load, label %switch.default [
    i32 42, label %switch.case.0
  ]

switch.case.0:                                    ; preds = %0
  ret void

switch.default:                                   ; preds = %0
  br label %switch.end

switch.end:                                       ; preds = %switch.default
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "constant-macro.cx")
!4 = distinct !DISubprogram(name: "foo", linkageName: "_CX1N4main3fooE4void1_M3std5int32", scope: !3, file: !3, line: 5, type: !5, scopeLine: 5, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
