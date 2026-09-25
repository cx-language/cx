
%never = type {}

@0 = private unnamed_addr constant [35 x i8] c"Assertion failed at assert.cx:6:5\0A\00", align 1

declare i1 @b() #0

define i32 @main() #0 !dbg !4 {
  %1 = call i1 @b(), !dbg !7
  %assert.condition = icmp eq i1 %1, false
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %0
  %2 = call %never @_CX1N3std10assertFailEM3std5never1_PKM3std4char(ptr @0), !dbg !8
  unreachable

assert.success:                                   ; preds = %0
  ret i32 0
}

declare %never @_CX1N3std10assertFailEM3std5never1_PKM3std4char(ptr) #0

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "assert.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 5, type: !5, scopeLine: 5, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 6, column: 12, scope: !4)
!8 = !DILocation(line: 5, column: 6, scope: !4)
