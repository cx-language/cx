
%"Range<int32>" = type { i32, i32 }

define void @_CX1N4main1fE4void1_M3std5RangeIM3std5int32E(%"Range<int32>" %r) #0 !dbg !4 {
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
  call void @_CX1N3stdM3std5RangeIM3std5int32E4initE4void2_M3std5int32M3std5int32(ptr %1, i32 %foo.load, i32 %bar.load), !dbg !8
  %.load = load %"Range<int32>", ptr %1, align 4
  call void @_CX1N4main1fE4void1_M3std5RangeIM3std5int32E(%"Range<int32>" %.load), !dbg !9
  ret i32 0
}

define void @_CX1N3stdM3std5RangeIM3std5int32E4initE4void2_M3std5int32M3std5int32(ptr %this, i32 %start, i32 %end) #0 !dbg !10 {
  %start1 = alloca i32, align 4
  %end2 = alloca i32, align 4
  store i32 %start, ptr %start1, align 4
  store i32 %end, ptr %end2, align 4
  %start3 = getelementptr inbounds %"Range<int32>", ptr %this, i32 0, i32 0
  %start.load = load i32, ptr %start1, align 4
  store i32 %start.load, ptr %start3, align 4
  %end4 = getelementptr inbounds %"Range<int32>", ptr %this, i32 0, i32 1
  %end.load = load i32, ptr %end2, align 4
  store i32 %end.load, ptr %end4, align 4
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "infer-generic-args-forget-mutable.cx")
!4 = distinct !DISubprogram(name: "f", linkageName: "_CX1N4main1fE4void1_M3std5RangeIM3std5int32E", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 5, type: !5, scopeLine: 5, spFlags: DISPFlagDefinition, unit: !2)
!8 = !DILocation(line: 8, column: 10, scope: !7)
!9 = !DILocation(line: 8, column: 5, scope: !7)
!10 = distinct !DISubprogram(name: "init", linkageName: "_CX1N3stdM3std5RangeIM3std5int32E4initE4void2_M3std5int32M3std5int32", scope: !11, file: !11, line: 7, type: !5, scopeLine: 7, spFlags: DISPFlagDefinition, unit: !2)
!11 = !DIFile(filename: "Range.cx")
