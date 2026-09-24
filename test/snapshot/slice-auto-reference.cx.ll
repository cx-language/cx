
%"Slice<int>" = type { ptr, i32 }

define void @_EN4main3fooE5SliceI3intE(%"Slice<int>" %a) #0 !dbg !4 {
  %a1 = alloca %"Slice<int>", align 8
  store %"Slice<int>" %a, ptr %a1, align 8
  ret void
}

define i32 @main() #0 !dbg !7 {
  %a = alloca [3 x i32], align 4
  store [3 x i32] [i32 1, i32 2, i32 3], ptr %a, align 4
  %1 = getelementptr inbounds [3 x i32], ptr %a, i32 0, i32 0
  %2 = insertvalue %"Slice<int>" undef, ptr %1, 0
  %3 = insertvalue %"Slice<int>" %2, i32 3, 1
  call void @_EN4main3fooE5SliceI3intE(%"Slice<int>" %3), !dbg !8
  call void @_EN4main3bazEP5ArrayI3intN3_E(ptr %a), !dbg !9
  ret i32 0
}

define void @_EN4main3bazEP5ArrayI3intN3_E(ptr %b) #0 !dbg !10 {
  %b1 = alloca ptr, align 8
  store ptr %b, ptr %b1, align 8
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "slice-auto-reference.cx")
!4 = distinct !DISubprogram(name: "foo", linkageName: "_EN4main3fooE5SliceI3intE", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 5, type: !5, scopeLine: 5, spFlags: DISPFlagDefinition, unit: !2)
!8 = !DILocation(line: 7, column: 5, scope: !7)
!9 = !DILocation(line: 8, column: 5, scope: !7)
!10 = distinct !DISubprogram(name: "baz", linkageName: "_EN4main3bazEP5ArrayI3intN3_E", scope: !3, file: !3, line: 11, type: !5, scopeLine: 11, spFlags: DISPFlagDefinition, unit: !2)
