
@_CX1G4main2spE = private global ptr null

define i32 @main() #0 !dbg !4 {
  %_CX1G4main2spE.load = load ptr, ptr @_CX1G4main2spE, align 8
  call void @_CX1N4main3fooE4void1_OPM4main1S(ptr %_CX1G4main2spE.load), !dbg !7
  ret i32 0
}

define void @_CX1N4main3fooE4void1_OPM4main1S(ptr %p) #0 !dbg !8 {
  %p1 = alloca ptr, align 8
  store ptr %p, ptr %p1, align 8
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "global-ptr-to-copyable-as-argument.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 4, column: 5, scope: !4)
!8 = distinct !DISubprogram(name: "foo", linkageName: "_CX1N4main3fooE4void1_OPM4main1S", scope: !3, file: !3, line: 9, type: !5, scopeLine: 9, spFlags: DISPFlagDefinition, unit: !2)
