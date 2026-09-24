
%S = type { i32 }

@i = private global i32 0

define i32 @main() #0 !dbg !4 {
  %i = alloca i32, align 4
  %s = alloca %S, align 8
  store i32 0, ptr %i, align 4
  call void @_EN4main1S4initE5int32(ptr %s, i32 0), !dbg !7
  call void @_EN4main1S1fE(ptr %s), !dbg !8
  ret i32 0
}

define void @_EN4main1S4initE5int32(ptr %this, i32 %i) #0 !dbg !9 {
  %i1 = alloca i32, align 4
  store i32 %i, ptr %i1, align 4
  %i2 = getelementptr inbounds %S, ptr %this, i32 0, i32 0
  %i.load = load i32, ptr %i1, align 4
  store i32 %i.load, ptr %i2, align 4
  ret void
}

define void @_EN4main1S1fE(ptr %this) #0 !dbg !10 {
  %i = alloca i32, align 4
  store i32 0, ptr %i, align 4
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "variable-shadowing.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 18, type: !5, scopeLine: 18, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 20, column: 11, scope: !4)
!8 = !DILocation(line: 21, column: 7, scope: !4)
!9 = distinct !DISubprogram(name: "init", linkageName: "_EN4main1S4initE5int32", scope: !3, file: !3, line: 5, type: !5, scopeLine: 5, spFlags: DISPFlagDefinition, unit: !2)
!10 = distinct !DISubprogram(name: "f", linkageName: "_EN4main1S1fE", scope: !3, file: !3, line: 8, type: !5, scopeLine: 8, spFlags: DISPFlagDefinition, unit: !2)
