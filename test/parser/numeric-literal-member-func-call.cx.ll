
define i32 @main() #0 !dbg !4 {
  %foo = alloca i64, align 8
  %1 = alloca i32, align 4
  store i32 0, ptr %1, align 4
  %2 = call i64 @_EN3std3int4hashE(ptr %1), !dbg !7
  store i64 %2, ptr %foo, align 8
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
!3 = !DIFile(filename: "numeric-literal-member-func-call.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 4, column: 17, scope: !4)
!8 = distinct !DISubprogram(name: "hash", linkageName: "_EN3std3int4hashE", scope: !9, file: !9, line: 96, type: !5, scopeLine: 96, spFlags: DISPFlagDefinition, unit: !2)
!9 = !DIFile(filename: "integers.cx")
