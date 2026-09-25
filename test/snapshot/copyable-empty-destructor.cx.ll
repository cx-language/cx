
%Foo = type {}

define i32 @main() #0 !dbg !4 {
  %foo = alloca %Foo, align 8
  call void @_CX1N4mainM4main3Foo6deinitE4void0_(ptr %foo), !dbg !7
  ret i32 0
}

define void @_CX1N4mainM4main3Foo6deinitE4void0_(ptr %this) #0 !dbg !8 {
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "copyable-empty-destructor.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 7, type: !5, scopeLine: 7, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 7, column: 6, scope: !4)
!8 = distinct !DISubprogram(name: "deinit", linkageName: "_CX1N4mainM4main3Foo6deinitE4void0_", scope: !3, file: !3, line: 4, type: !5, scopeLine: 4, spFlags: DISPFlagDefinition, unit: !2)
