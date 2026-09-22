
%"C<bool>" = type {}

define i32 @main() #0 !dbg !4 {
  %c = alloca %"C<bool>", align 8
  call void @_EN4main1CI4boolE4initE(ptr %c), !dbg !7
  call void @_EN4main1CI4boolE1fE(ptr %c), !dbg !8
  ret i32 0
}

define void @_EN4main1CI4boolE4initE(ptr %this) #0 !dbg !9 {
  ret void
}

define void @_EN4main1CI4boolE1fE(ptr %this) #0 !dbg !10 {
  call void @_EN4main1CI4boolE1gE(ptr %this), !dbg !11
  ret void
}

define void @_EN4main1CI4boolE1gE(ptr %this) #0 !dbg !12 {
  %a = alloca i64, align 8
  store i64 1, ptr %a, align 8
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "generic-type-member-func-using-generic-param-2.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 13, type: !5, scopeLine: 13, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 14, column: 13, scope: !4)
!8 = !DILocation(line: 15, column: 7, scope: !4)
!9 = distinct !DISubprogram(name: "init", linkageName: "_EN4main1CI4boolE4initE", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!10 = distinct !DISubprogram(name: "f", linkageName: "_EN4main1CI4boolE1fE", scope: !3, file: !3, line: 4, type: !5, scopeLine: 4, spFlags: DISPFlagDefinition, unit: !2)
!11 = !DILocation(line: 5, column: 9, scope: !10)
!12 = distinct !DISubprogram(name: "g", linkageName: "_EN4main1CI4boolE1gE", scope: !3, file: !3, line: 8, type: !5, scopeLine: 8, spFlags: DISPFlagDefinition, unit: !2)
