
@foo = private global [3 x i32] [i32 0, i32 1, i32 2]

define i32 @main() #0 !dbg !4 {
  %qux = alloca [1 x i32], align 4
  store [1 x i32] [i32 42], ptr %qux, align 4
  store i32 3, ptr @foo, align 4
  %1 = call i64 @_EN3std3int4hashE(ptr getelementptr inbounds ([3 x i32], ptr @foo, i32 0, i32 1)), !dbg !7
  ret i32 0
}

define i64 @_EN3std3int4hashE(ptr %this) #0 !dbg !8 {
  %this.load = load i32, ptr %this, align 4
  %1 = sext i32 %this.load to i64
  ret i64 %1
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "variable-of-array-type.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 5, type: !5, scopeLine: 5, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 8, column: 12, scope: !4)
!8 = distinct !DISubprogram(name: "hash", linkageName: "_EN3std3int4hashE", scope: !9, file: !9, line: 52, type: !5, scopeLine: 52, spFlags: DISPFlagDefinition, unit: !2)
!9 = !DIFile(filename: "integers.cx")
