
%X = type {}

define i32 @_CX1N4main1gEM3std5int320_() #0 !dbg !4 {
  ret i32 0
}

define i1 @_CX1N4main1hEM3std4bool0_() #0 !dbg !7 {
  ret i1 false
}

define i32 @main() #0 !dbg !8 {
  %x = alloca %X, align 8
  call void @_CX1N4mainM4main1X4initE4void0_(ptr %x), !dbg !9
  %1 = call i32 @_CX1N4mainM4main1X1fIM3std5int32EEM3std5int321_F0_M3std5int32(ptr %x, ptr @_CX1N4main1gEM3std5int320_), !dbg !10
  %2 = call i1 @_CX1N4mainM4main1X1fIM3std4boolEEM3std4bool1_F0_M3std4bool(ptr %x, ptr @_CX1N4main1hEM3std4bool0_), !dbg !11
  ret i32 0
}

define void @_CX1N4mainM4main1X4initE4void0_(ptr %this) #0 !dbg !12 {
  ret void
}

define i32 @_CX1N4mainM4main1X1fIM3std5int32EEM3std5int321_F0_M3std5int32(ptr %this, ptr %t) #0 !dbg !13 {
  %t1 = alloca ptr, align 8
  store ptr %t, ptr %t1, align 8
  %t.load = load ptr, ptr %t1, align 8
  %1 = call i32 %t.load(), !dbg !14
  ret i32 %1
}

define i1 @_CX1N4mainM4main1X1fIM3std4boolEEM3std4bool1_F0_M3std4bool(ptr %this, ptr %t) #0 !dbg !15 {
  %t1 = alloca ptr, align 8
  store ptr %t, ptr %t1, align 8
  %t.load = load ptr, ptr %t1, align 8
  %1 = call i1 %t.load(), !dbg !16
  ret i1 %1
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "generic-method.cx")
!4 = distinct !DISubprogram(name: "g", linkageName: "_CX1N4main1gEM3std5int320_", scope: !3, file: !3, line: 9, type: !5, scopeLine: 9, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = distinct !DISubprogram(name: "h", linkageName: "_CX1N4main1hEM3std4bool0_", scope: !3, file: !3, line: 10, type: !5, scopeLine: 10, spFlags: DISPFlagDefinition, unit: !2)
!8 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 12, type: !5, scopeLine: 12, spFlags: DISPFlagDefinition, unit: !2)
!9 = !DILocation(line: 13, column: 13, scope: !8)
!10 = !DILocation(line: 14, column: 7, scope: !8)
!11 = !DILocation(line: 15, column: 7, scope: !8)
!12 = distinct !DISubprogram(name: "init", linkageName: "_CX1N4mainM4main1X4initE4void0_", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!13 = distinct !DISubprogram(name: "f", linkageName: "_CX1N4mainM4main1X1fIM3std5int32EEM3std5int321_F0_M3std5int32", scope: !3, file: !3, line: 4, type: !5, scopeLine: 4, spFlags: DISPFlagDefinition, unit: !2)
!14 = !DILocation(line: 5, column: 16, scope: !13)
!15 = distinct !DISubprogram(name: "f", linkageName: "_CX1N4mainM4main1X1fIM3std4boolEEM3std4bool1_F0_M3std4bool", scope: !3, file: !3, line: 4, type: !5, scopeLine: 4, spFlags: DISPFlagDefinition, unit: !2)
!16 = !DILocation(line: 5, column: 16, scope: !15)
