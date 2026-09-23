
%never = type {}

define %never @_EN4main1fE() #0 !dbg !4 {
  %1 = call %never @_EN4main1fE(), !dbg !7
  unreachable
}

define %never @_EN4main1gE() #0 !dbg !8 {
  %1 = call %never @_EN3std12abortWrapperE(), !dbg !9
  unreachable
}

declare %never @_EN3std12abortWrapperE() #0

define %never @_EN4main1hE() #0 !dbg !10 {
  unreachable
}

define void @_EN4main1iE() #0 !dbg !11 {
  %1 = call %never @_EN4main1fE(), !dbg !12
  ret void
}

define i32 @_EN4main1iE3int(i32 %x) #0 !dbg !13 {
  %x1 = alloca i32, align 4
  store i32 %x, ptr %x1, align 4
  %x.load = load i32, ptr %x1, align 4
  switch i32 %x.load, label %switch.default [
    i32 0, label %switch.case.0
  ]

switch.case.0:                                    ; preds = %0
  ret i32 1

switch.default:                                   ; preds = %0
  %1 = call %never @_EN4main1fE(), !dbg !14
  br label %switch.end

switch.end:                                       ; preds = %switch.default
  unreachable
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "never.cx")
!4 = distinct !DISubprogram(name: "f", linkageName: "_EN4main1fE", scope: !3, file: !3, line: 5, type: !5, scopeLine: 5, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 5, column: 13, scope: !4)
!8 = distinct !DISubprogram(name: "g", linkageName: "_EN4main1gE", scope: !3, file: !3, line: 8, type: !5, scopeLine: 8, spFlags: DISPFlagDefinition, unit: !2)
!9 = !DILocation(line: 8, column: 13, scope: !8)
!10 = distinct !DISubprogram(name: "h", linkageName: "_EN4main1hE", scope: !3, file: !3, line: 11, type: !5, scopeLine: 11, spFlags: DISPFlagDefinition, unit: !2)
!11 = distinct !DISubprogram(name: "i", linkageName: "_EN4main1iE", scope: !3, file: !3, line: 14, type: !5, scopeLine: 14, spFlags: DISPFlagDefinition, unit: !2)
!12 = !DILocation(line: 14, column: 12, scope: !11)
!13 = distinct !DISubprogram(name: "i", linkageName: "_EN4main1iE3int", scope: !3, file: !3, line: 17, type: !5, scopeLine: 17, spFlags: DISPFlagDefinition, unit: !2)
!14 = !DILocation(line: 20, column: 18, scope: !13)
