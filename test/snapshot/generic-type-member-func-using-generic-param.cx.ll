
%"A<int32>" = type {}

define i32 @main() #0 !dbg !4 {
  %a = alloca %"A<int32>", align 8
  call void @_EN4main1AI5int32E4initE(ptr %a), !dbg !7
  call void @_EN4main1AI5int32E1aE5int32(ptr %a, i32 5), !dbg !8
  ret i32 0
}

define void @_EN4main1AI5int32E4initE(ptr %this) #0 !dbg !9 {
  ret void
}

define void @_EN4main1AI5int32E1aE5int32(ptr %this, i32 %n) #0 !dbg !10 {
  %n1 = alloca i32, align 4
  store i32 %n, ptr %n1, align 4
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "generic-type-member-func-using-generic-param.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 7, type: !5, scopeLine: 7, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 8, column: 13, scope: !4)
!8 = !DILocation(line: 9, column: 7, scope: !4)
!9 = distinct !DISubprogram(name: "init", linkageName: "_EN4main1AI5int32E4initE", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!10 = distinct !DISubprogram(name: "a", linkageName: "_EN4main1AI5int32E1aE5int32", scope: !3, file: !3, line: 4, type: !5, scopeLine: 4, spFlags: DISPFlagDefinition, unit: !2)
