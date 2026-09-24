
%"Range<int32>" = type { i32, i32 }

define void @_EN4main1fE5RangeI5int32E(%"Range<int32>" %r) #0 !dbg !4 {
  %r1 = alloca %"Range<int32>", align 8
  store %"Range<int32>" %r, ptr %r1, align 4
  ret void
}

define i32 @main() #0 !dbg !7 {
  %foo = alloca i32, align 4
  %bar = alloca i32, align 4
  %1 = alloca %"Range<int32>", align 8
  store i32 0, ptr %foo, align 4
  store i32 0, ptr %bar, align 4
  %foo.load = load i32, ptr %foo, align 4
  %bar.load = load i32, ptr %bar, align 4
  call void @_EN3std5RangeI5int32E4initE5int325int32(ptr %1, i32 %foo.load, i32 %bar.load), !dbg !8
  %.load = load %"Range<int32>", ptr %1, align 4
  call void @_EN4main1fE5RangeI5int32E(%"Range<int32>" %.load), !dbg !9
  ret i32 0
}

declare void @_EN3std5RangeI5int32E4initE5int325int32(ptr, i32, i32) #0

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "infer-generic-args-forget-mutable.cx")
!4 = distinct !DISubprogram(name: "f", linkageName: "_EN4main1fE5RangeI5int32E", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 5, type: !5, scopeLine: 5, spFlags: DISPFlagDefinition, unit: !2)
!8 = !DILocation(line: 8, column: 10, scope: !7)
!9 = !DILocation(line: 8, column: 5, scope: !7)
