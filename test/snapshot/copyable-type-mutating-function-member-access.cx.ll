
%S = type { i32 }

define i32 @main() #0 !dbg !4 {
  %1 = alloca %S, align 8
  call void @_CX1N4mainM4main1S4initE4void1_M3std5int32(ptr %1, i32 1), !dbg !7
  call void @_CX1N4mainM4main1S3fooE4void0_(ptr %1), !dbg !8
  call void @_CX1N3std10checkLeaksE4void0_(), !dbg !9
  ret i32 0
}

define void @_CX1N4mainM4main1S4initE4void1_M3std5int32(ptr %this, i32 %a) #0 !dbg !10 {
  %a1 = alloca i32, align 4
  store i32 %a, ptr %a1, align 4
  %a2 = getelementptr inbounds %S, ptr %this, i32 0, i32 0
  %a.load = load i32, ptr %a1, align 4
  store i32 %a.load, ptr %a2, align 4
  ret void
}

define void @_CX1N4mainM4main1S3fooE4void0_(ptr %this) #0 !dbg !11 {
  %b = alloca i32, align 4
  %a = getelementptr inbounds %S, ptr %this, i32 0, i32 0
  %a.load = load i32, ptr %a, align 4
  store i32 %a.load, ptr %b, align 4
  ret void
}

declare void @_CX1N3std10checkLeaksE4void0_() #0

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "copyable-type-mutating-function-member-access.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 11, type: !5, scopeLine: 11, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 12, column: 5, scope: !4)
!8 = !DILocation(line: 12, column: 10, scope: !4)
!9 = !DILocation(line: 11, column: 6, scope: !4)
!10 = distinct !DISubprogram(name: "init", linkageName: "_CX1N4mainM4main1S4initE4void1_M3std5int32", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!11 = distinct !DISubprogram(name: "foo", linkageName: "_CX1N4mainM4main1S3fooE4void0_", scope: !3, file: !3, line: 6, type: !5, scopeLine: 6, spFlags: DISPFlagDefinition, unit: !2)
