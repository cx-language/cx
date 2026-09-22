
define void @_EN4main3bazE4bool(i1 %foo) #0 !dbg !4 {
  %foo1 = alloca i1, align 1
  store i1 %foo, ptr %foo1, align 1
  br label %loop.condition

loop.condition:                                   ; preds = %if.end, %if.then, %0
  %foo.load = load i1, ptr %foo1, align 1
  br i1 %foo.load, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %foo.load2 = load i1, ptr %foo1, align 1
  br i1 %foo.load2, label %if.then, label %if.else

loop.end:                                         ; preds = %loop.condition
  ret void

if.then:                                          ; preds = %loop.body
  %foo.load3 = load i1, ptr %foo1, align 1
  call void @_EN4main3bazE4bool(i1 %foo.load3), !dbg !7
  br label %loop.condition

if.else:                                          ; preds = %loop.body
  br label %if.end

if.end:                                           ; preds = %if.else
  br label %loop.condition
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "continue.cx")
!4 = distinct !DISubprogram(name: "baz", linkageName: "_EN4main3bazE4bool", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 6, column: 13, scope: !4)
