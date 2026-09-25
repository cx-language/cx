
%"X<int32>" = type {}
%"X<bool>" = type {}

define i32 @main() #0 !dbg !4 {
  %x = alloca %"X<int32>", align 8
  %a = alloca %"X<bool>", align 8
  %1 = call %"X<bool>" @_CX1N4mainM4main1XIM3std5int32E1fIM3std4boolEEM4main1XIM3std4boolE1_M3std4bool(ptr %x, i1 false), !dbg !7
  store %"X<bool>" %1, ptr %a, align 1
  call void @_CX1N4mainM4main1XIM3std4boolE1gE4void0_(ptr %a), !dbg !8
  call void @_CX1N3std10checkLeaksE4void0_(), !dbg !9
  ret i32 0
}

define %"X<bool>" @_CX1N4mainM4main1XIM3std5int32E1fIM3std4boolEEM4main1XIM3std4boolE1_M3std4bool(ptr %this, i1 %u) #0 !dbg !10 {
  %u1 = alloca i1, align 1
  %x = alloca %"X<bool>", align 8
  store i1 %u, ptr %u1, align 1
  %x.load = load %"X<bool>", ptr %x, align 1
  ret %"X<bool>" %x.load
}

define void @_CX1N4mainM4main1XIM3std4boolE1gE4void0_(ptr %this) #0 !dbg !11 {
  ret void
}

declare void @_CX1N3std10checkLeaksE4void0_() #0

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "generic-method-use-return-type.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 12, type: !5, scopeLine: 12, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 14, column: 15, scope: !4)
!8 = !DILocation(line: 15, column: 7, scope: !4)
!9 = !DILocation(line: 12, column: 6, scope: !4)
!10 = distinct !DISubprogram(name: "f", linkageName: "_CX1N4mainM4main1XIM3std5int32E1fIM3std4boolEEM4main1XIM3std4boolE1_M3std4bool", scope: !3, file: !3, line: 4, type: !5, scopeLine: 4, spFlags: DISPFlagDefinition, unit: !2)
!11 = distinct !DISubprogram(name: "g", linkageName: "_CX1N4mainM4main1XIM3std4boolE1gE4void0_", scope: !3, file: !3, line: 9, type: !5, scopeLine: 9, spFlags: DISPFlagDefinition, unit: !2)
