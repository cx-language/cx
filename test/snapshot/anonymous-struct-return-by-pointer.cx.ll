
%S = type { { i32 } }

define i32 @main() #0 !dbg !4 {
  %s = alloca %S, align 8
  %f = alloca ptr, align 8
  %g = alloca ptr, align 8
  call void @_CX1N4mainM4main1S4initE4void1_T1_1aM3std5int32(ptr %s, { i32 } { i32 1 }), !dbg !7
  %1 = call ptr @_CX1N4mainM4main1S1fEPT1_1aM3std5int320_(ptr %s), !dbg !8
  store ptr %1, ptr %f, align 8
  %2 = call ptr @_CX1N4mainM4main1S1gEPT1_1aM3std5int320_(ptr %s), !dbg !9
  store ptr %2, ptr %g, align 8
  ret i32 0
}

define void @_CX1N4mainM4main1S4initE4void1_T1_1aM3std5int32(ptr %this, { i32 } %t) #0 !dbg !10 {
  %t1 = alloca { i32 }, align 8
  store { i32 } %t, ptr %t1, align 4
  %t2 = getelementptr inbounds %S, ptr %this, i32 0, i32 0
  %t.load = load { i32 }, ptr %t1, align 4
  store { i32 } %t.load, ptr %t2, align 4
  ret void
}

define ptr @_CX1N4mainM4main1S1fEPT1_1aM3std5int320_(ptr %this) #0 !dbg !11 {
  %t = getelementptr inbounds %S, ptr %this, i32 0, i32 0
  ret ptr %t
}

define ptr @_CX1N4mainM4main1S1gEPT1_1aM3std5int320_(ptr %this) #0 !dbg !12 {
  %t = getelementptr inbounds %S, ptr %this, i32 0, i32 0
  ret ptr %t
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "anonymous-struct-return-by-pointer.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 15, type: !5, scopeLine: 15, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 16, column: 13, scope: !4)
!8 = !DILocation(line: 17, column: 15, scope: !4)
!9 = !DILocation(line: 18, column: 15, scope: !4)
!10 = distinct !DISubprogram(name: "init", linkageName: "_CX1N4mainM4main1S4initE4void1_T1_1aM3std5int32", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!11 = distinct !DISubprogram(name: "f", linkageName: "_CX1N4mainM4main1S1fEPT1_1aM3std5int320_", scope: !3, file: !3, line: 6, type: !5, scopeLine: 6, spFlags: DISPFlagDefinition, unit: !2)
!12 = distinct !DISubprogram(name: "g", linkageName: "_CX1N4mainM4main1S1gEPT1_1aM3std5int320_", scope: !3, file: !3, line: 10, type: !5, scopeLine: 10, spFlags: DISPFlagDefinition, unit: !2)
