
%Foo = type { i32 }

define i32 @main() #0 !dbg !4 {
  %f = alloca %Foo, align 8
  %rf = alloca ptr, align 8
  store ptr %f, ptr %rf, align 8
  %rf.load = load ptr, ptr %rf, align 8
  call void @_CX1N4mainM4main3Foo3barE4void0_(ptr %rf.load), !dbg !7
  call void @_CX1N3std10checkLeaksE4void0_(), !dbg !8
  ret i32 0
}

define void @_CX1N4mainM4main3Foo3barE4void0_(ptr %this) #0 !dbg !9 {
  ret void
}

declare void @_CX1N3std10checkLeaksE4void0_() #0

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "member-function-call-via-local-pointer.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 8, type: !5, scopeLine: 8, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 11, column: 8, scope: !4)
!8 = !DILocation(line: 8, column: 6, scope: !4)
!9 = distinct !DISubprogram(name: "bar", linkageName: "_CX1N4mainM4main3Foo3barE4void0_", scope: !3, file: !3, line: 5, type: !5, scopeLine: 5, spFlags: DISPFlagDefinition, unit: !2)
