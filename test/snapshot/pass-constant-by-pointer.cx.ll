
%"S<int32>" = type {}

define i32 @main() #0 !dbg !4 {
  %s = alloca %"S<int32>", align 8
  %1 = alloca i32, align 4
  call void @_EN4main1SI5int32E4initE(ptr %s), !dbg !7
  store i32 0, ptr %1, align 4
  call void @_EN4main1SI5int32E1fEP5int32(ptr %s, ptr %1), !dbg !8
  ret i32 0
}

define void @_EN4main1SI5int32E4initE(ptr %this) #0 !dbg !9 {
  ret void
}

define void @_EN4main1SI5int32E1fEP5int32(ptr %this, ptr %t) #0 !dbg !10 {
  %t1 = alloca ptr, align 8
  store ptr %t, ptr %t1, align 8
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "pass-constant-by-pointer.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 9, type: !5, scopeLine: 9, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 10, column: 13, scope: !4)
!8 = !DILocation(line: 11, column: 7, scope: !4)
!9 = distinct !DISubprogram(name: "init", linkageName: "_EN4main1SI5int32E4initE", scope: !3, file: !3, line: 5, type: !5, scopeLine: 5, spFlags: DISPFlagDefinition, unit: !2)
!10 = distinct !DISubprogram(name: "f", linkageName: "_EN4main1SI5int32E1fEP5int32", scope: !3, file: !3, line: 6, type: !5, scopeLine: 6, spFlags: DISPFlagDefinition, unit: !2)
