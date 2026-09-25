
%Foo = type { i32 }
%Bar = type { i32 }

define i32 @main() #0 !dbg !4 {
  %1 = alloca %Foo, align 8
  %2 = alloca %Bar, align 8
  call void @_CX1N4mainM4main3Foo4initE4void1_M3std5int32(ptr %1, i32 1), !dbg !7
  call void @_CX1N4mainM4main3Foo7doStuffE4void0_(ptr %1), !dbg !8
  call void @_CX1N4mainM4main3Bar4initE4void1_M3std5int32(ptr %2, i32 2), !dbg !9
  call void @_CX1N4mainM4main3Bar7doStuffE4void0_(ptr %2), !dbg !10
  call void @_CX1N3std10checkLeaksE4void0_(), !dbg !11
  ret i32 0
}

define void @_CX1N4mainM4main3Foo4initE4void1_M3std5int32(ptr %this, i32 %i) #0 !dbg !12 {
  %i1 = alloca i32, align 4
  store i32 %i, ptr %i1, align 4
  %i2 = getelementptr inbounds %Foo, ptr %this, i32 0, i32 0
  %i.load = load i32, ptr %i1, align 4
  store i32 %i.load, ptr %i2, align 4
  ret void
}

define void @_CX1N4mainM4main3Foo7doStuffE4void0_(ptr %this) #0 !dbg !13 {
  ret void
}

define void @_CX1N4mainM4main3Bar4initE4void1_M3std5int32(ptr %this, i32 %i) #0 !dbg !14 {
  %i1 = alloca i32, align 4
  store i32 %i, ptr %i1, align 4
  %i2 = getelementptr inbounds %Bar, ptr %this, i32 0, i32 0
  %i.load = load i32, ptr %i1, align 4
  store i32 %i.load, ptr %i2, align 4
  ret void
}

define void @_CX1N4mainM4main3Bar7doStuffE4void0_(ptr %this) #0 !dbg !15 {
  ret void
}

declare void @_CX1N3std10checkLeaksE4void0_() #0

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "member-function-on-different-types.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 13, type: !5, scopeLine: 13, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 14, column: 5, scope: !4)
!8 = !DILocation(line: 14, column: 12, scope: !4)
!9 = !DILocation(line: 15, column: 5, scope: !4)
!10 = !DILocation(line: 15, column: 12, scope: !4)
!11 = !DILocation(line: 13, column: 6, scope: !4)
!12 = distinct !DISubprogram(name: "init", linkageName: "_CX1N4mainM4main3Foo4initE4void1_M3std5int32", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!13 = distinct !DISubprogram(name: "doStuff", linkageName: "_CX1N4mainM4main3Foo7doStuffE4void0_", scope: !3, file: !3, line: 5, type: !5, scopeLine: 5, spFlags: DISPFlagDefinition, unit: !2)
!14 = distinct !DISubprogram(name: "init", linkageName: "_CX1N4mainM4main3Bar4initE4void1_M3std5int32", scope: !3, file: !3, line: 8, type: !5, scopeLine: 8, spFlags: DISPFlagDefinition, unit: !2)
!15 = distinct !DISubprogram(name: "doStuff", linkageName: "_CX1N4mainM4main3Bar7doStuffE4void0_", scope: !3, file: !3, line: 10, type: !5, scopeLine: 10, spFlags: DISPFlagDefinition, unit: !2)
