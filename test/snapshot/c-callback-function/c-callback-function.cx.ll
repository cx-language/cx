
define i32 @_CX1N4main1fEM3std5int321_M3std5int32(i32 %a) #0 !dbg !4 {
  %a1 = alloca i32, align 4
  store i32 %a, ptr %a1, align 4
  %a.load = load i32, ptr %a1, align 4
  ret i32 %a.load
}

define i32 @main() #0 !dbg !7 {
  call void @foo(ptr @_CX1N4main1fEM3std5int321_M3std5int32), !dbg !8
  call void @bar(ptr @_CX1N4main1fEM3std5int321_M3std5int32), !dbg !9
  call void @baz(ptr @_CX1N4main1fEM3std5int321_M3std5int32), !dbg !10
  ret i32 0
}

declare void @foo(ptr) #0

declare void @bar(ptr) #0

declare void @baz(ptr) #0

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "c-callback-function.cx")
!4 = distinct !DISubprogram(name: "f", linkageName: "_CX1N4main1fEM3std5int321_M3std5int32", scope: !3, file: !3, line: 5, type: !5, scopeLine: 5, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 9, type: !5, scopeLine: 9, spFlags: DISPFlagDefinition, unit: !2)
!8 = !DILocation(line: 10, column: 5, scope: !7)
!9 = !DILocation(line: 11, column: 5, scope: !7)
!10 = !DILocation(line: 12, column: 5, scope: !7)
