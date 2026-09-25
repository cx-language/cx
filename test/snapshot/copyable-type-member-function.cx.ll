
%Foo = type { i32 }

define i32 @main() #0 !dbg !4 {
  %foo = alloca %Foo, align 8
  %i = alloca i32, align 4
  call void @_CX1N4mainM4main3Foo4initE4void0_(ptr %foo), !dbg !7
  call void @_CX1N4mainM4main3Foo3barE4void0_(ptr %foo), !dbg !8
  %1 = call i32 @_CX1N4mainM4main3Foo3quxEM3std5int320_(ptr %foo), !dbg !9
  store i32 %1, ptr %i, align 4
  call void @_CX1N3std10checkLeaksE4void0_(), !dbg !10
  ret i32 0
}

define void @_CX1N4mainM4main3Foo4initE4void0_(ptr %this) #0 !dbg !11 {
  %baz = getelementptr inbounds %Foo, ptr %this, i32 0, i32 0
  store i32 42, ptr %baz, align 4
  ret void
}

define void @_CX1N4mainM4main3Foo3barE4void0_(ptr %this) #0 !dbg !12 {
  ret void
}

define i32 @_CX1N4mainM4main3Foo3quxEM3std5int320_(ptr %this) #0 !dbg !13 {
  %baz = getelementptr inbounds %Foo, ptr %this, i32 0, i32 0
  %baz.load = load i32, ptr %baz, align 4
  ret i32 %baz.load
}

declare void @_CX1N3std10checkLeaksE4void0_() #0

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "copyable-type-member-function.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 10, type: !5, scopeLine: 10, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 11, column: 15, scope: !4)
!8 = !DILocation(line: 12, column: 9, scope: !4)
!9 = !DILocation(line: 13, column: 17, scope: !4)
!10 = !DILocation(line: 10, column: 6, scope: !4)
!11 = distinct !DISubprogram(name: "init", linkageName: "_CX1N4mainM4main3Foo4initE4void0_", scope: !3, file: !3, line: 5, type: !5, scopeLine: 5, spFlags: DISPFlagDefinition, unit: !2)
!12 = distinct !DISubprogram(name: "bar", linkageName: "_CX1N4mainM4main3Foo3barE4void0_", scope: !3, file: !3, line: 6, type: !5, scopeLine: 6, spFlags: DISPFlagDefinition, unit: !2)
!13 = distinct !DISubprogram(name: "qux", linkageName: "_CX1N4mainM4main3Foo3quxEM3std5int320_", scope: !3, file: !3, line: 7, type: !5, scopeLine: 7, spFlags: DISPFlagDefinition, unit: !2)
