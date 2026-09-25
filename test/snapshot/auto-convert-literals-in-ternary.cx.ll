
define i8 @_CX1N4main3fooEM3std5uint81_M3std4bool(i1 %b) #0 !dbg !4 {
  %b1 = alloca i1, align 1
  store i1 %b, ptr %b1, align 1
  %b.load = load i1, ptr %b1, align 1
  br i1 %b.load, label %if.then, label %if.else

if.then:                                          ; preds = %0
  br label %if.end

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %if.result = phi i8 [ -1, %if.then ], [ -18, %if.else ]
  ret i8 %if.result
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "auto-convert-literals-in-ternary.cx")
!4 = distinct !DISubprogram(name: "foo", linkageName: "_CX1N4main3fooEM3std5uint81_M3std4bool", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
