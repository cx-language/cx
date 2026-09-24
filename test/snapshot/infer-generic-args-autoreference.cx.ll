
define i32 @main() #0 !dbg !4 {
  %i = alloca i32, align 4
  store i32 42, ptr %i, align 4
  call void @_EN4main1fI5int32EER5int32(ptr %i), !dbg !7
  ret i32 0
}

define void @_EN4main1fI5int32EER5int32(ptr %p) #0 !dbg !8 {
  %p1 = alloca ptr, align 8
  store ptr %p, ptr %p1, align 8
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "infer-generic-args-autoreference.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 5, type: !5, scopeLine: 5, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 7, column: 5, scope: !4)
!8 = distinct !DISubprogram(name: "f", linkageName: "_EN4main1fI5int32EER5int32", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
