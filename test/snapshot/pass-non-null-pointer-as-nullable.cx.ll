
define void @_CX1N4main3fooE4void1_OPM3std5int32(ptr %p) #0 !dbg !4 {
  %p1 = alloca ptr, align 8
  store ptr %p, ptr %p1, align 8
  ret void
}

define i32 @main() #0 !dbg !7 {
  %i = alloca i32, align 4
  %x = alloca ptr, align 8
  store i32 42, ptr %i, align 4
  store ptr %i, ptr %x, align 8
  %x.load = load ptr, ptr %x, align 8
  call void @_CX1N4main3fooE4void1_OPM3std5int32(ptr %x.load), !dbg !8
  call void @_CX1N4main3fooE4void1_OPM3std5int32(ptr %i), !dbg !9
  ret i32 0
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "pass-non-null-pointer-as-nullable.cx")
!4 = distinct !DISubprogram(name: "foo", linkageName: "_CX1N4main3fooE4void1_OPM3std5int32", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 5, type: !5, scopeLine: 5, spFlags: DISPFlagDefinition, unit: !2)
!8 = !DILocation(line: 8, column: 5, scope: !7)
!9 = !DILocation(line: 9, column: 5, scope: !7)
