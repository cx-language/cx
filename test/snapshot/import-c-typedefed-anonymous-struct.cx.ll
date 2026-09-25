
%Foo = type { i32 }

define i32 @main() #0 !dbg !4 {
  %x = alloca %Foo, align 8
  %x.load = load %Foo, ptr %x, align 4
  call void @foo(%Foo %x.load), !dbg !7
  call void @_CX1N3std10checkLeaksE4void0_(), !dbg !8
  ret i32 0
}

declare void @foo(%Foo) #0

declare void @_CX1N3std10checkLeaksE4void0_() #0

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "import-c-typedefed-anonymous-struct.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 5, type: !5, scopeLine: 5, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 7, column: 5, scope: !4)
!8 = !DILocation(line: 5, column: 6, scope: !4)
