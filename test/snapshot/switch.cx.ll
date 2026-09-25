
%S = type { i32 }

define void @_CX1N4main3fooE4void1_M3std5int32(i32 %i) #0 !dbg !4 {
  %i1 = alloca i32, align 4
  store i32 %i, ptr %i1, align 4
  %i.load = load i32, ptr %i1, align 4
  switch i32 %i.load, label %switch.default [
    i32 0, label %switch.case.0
    i32 -1, label %switch.case.1
    i32 42, label %switch.case.2
  ]

switch.case.0:                                    ; preds = %0
  br label %switch.end

switch.case.1:                                    ; preds = %0
  %i.load2 = load i32, ptr %i1, align 4
  call void @_CX1N4main3fooE4void1_M3std5int32(i32 %i.load2), !dbg !7
  %i.load3 = load i32, ptr %i1, align 4
  call void @_CX1N4main3fooE4void1_M3std5int32(i32 %i.load3), !dbg !8
  br label %switch.end

switch.case.2:                                    ; preds = %0
  br label %switch.end

switch.default:                                   ; preds = %0
  br label %switch.end

switch.end:                                       ; preds = %switch.default, %switch.case.2, %switch.case.1, %switch.case.0
  %i.load4 = load i32, ptr %i1, align 4
  switch i32 %i.load4, label %switch.default6 [
    i32 0, label %switch.case.05
  ]

switch.case.05:                                   ; preds = %switch.end
  br label %switch.end8

switch.default6:                                  ; preds = %switch.end
  %i.load7 = load i32, ptr %i1, align 4
  call void @_CX1N4main3fooE4void1_M3std5int32(i32 %i.load7), !dbg !9
  br label %switch.end8

switch.end8:                                      ; preds = %switch.default6, %switch.case.05
  ret void
}

define i32 @_CX1N4main1fEM3std5int321_PM4main1S(ptr %s) #0 !dbg !10 {
  %s1 = alloca ptr, align 8
  store ptr %s, ptr %s1, align 8
  %s.load = load ptr, ptr %s1, align 8
  %i = getelementptr inbounds %S, ptr %s.load, i32 0, i32 0
  %i.load = load i32, ptr %i, align 4
  switch i32 %i.load, label %switch.default [
    i32 0, label %switch.case.0
  ]

switch.case.0:                                    ; preds = %0
  ret i32 0

switch.default:                                   ; preds = %0
  ret i32 1
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "switch.cx")
!4 = distinct !DISubprogram(name: "foo", linkageName: "_CX1N4main3fooE4void1_M3std5int32", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 8, column: 13, scope: !4)
!8 = !DILocation(line: 9, column: 13, scope: !4)
!9 = !DILocation(line: 17, column: 13, scope: !4)
!10 = distinct !DISubprogram(name: "f", linkageName: "_CX1N4main1fEM3std5int321_PM4main1S", scope: !3, file: !3, line: 25, type: !5, scopeLine: 25, spFlags: DISPFlagDefinition, unit: !2)
