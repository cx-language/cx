
%"A<int32>" = type { i32 }

define void @_EN4main3fooE1AI5int32E(%"A<int32>" %a) #0 !dbg !4 {
  %a1 = alloca %"A<int32>", align 8
  store %"A<int32>" %a, ptr %a1, align 4
  call void @_EN4main1AI5int32E6deinitE(ptr %a1), !dbg !7
  ret void
}

define void @_EN4main1AI5int32E6deinitE(ptr %this) #0 !dbg !8 {
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "struct-with-destructor-as-value-param.cx")
!4 = distinct !DISubprogram(name: "foo", linkageName: "_EN4main3fooE1AI5int32E", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 3, column: 6, scope: !4)
!8 = distinct !DISubprogram(name: "deinit", linkageName: "_EN4main1AI5int32E6deinitE", scope: !3, file: !3, line: 7, type: !5, scopeLine: 7, spFlags: DISPFlagDefinition, unit: !2)
