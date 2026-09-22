
@0 = private unnamed_addr constant [6 x i8] c"%d %f\00", align 1

define i32 @main() #0 !dbg !4 {
  %f = alloca float, align 4
  store float 4.200000e+00, ptr %f, align 4
  %f.load = load float, ptr %f, align 4
  %1 = call i32 (ptr, ...) @printf(ptr @0, i32 -3, float %f.load), !dbg !7
  ret i32 0
}

declare i32 @printf(ptr, ...) #0

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "variadic-c-function.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 5, column: 5, scope: !4)
