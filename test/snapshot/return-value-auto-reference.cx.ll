
%A = type { [2 x i1] }
%B = type { i8 }

@i = private global i32 42

define ptr @_EN4main3fooE() #0 !dbg !4 {
  %a = alloca ptr, align 8
  %1 = alloca %A, align 8
  %b = alloca ptr, align 8
  %2 = alloca %B, align 8
  call void @_EN4main1A4initE5ArrayI4boolN2_E(ptr %1, [2 x i1] [i1 false, i1 true]), !dbg !7
  %3 = call ptr @_EN4main1A2ffE(ptr %1), !dbg !8
  store ptr %3, ptr %a, align 8
  call void @_EN4main1B4initE4char(ptr %2, i8 97), !dbg !9
  %4 = call ptr @_EN4main1B2ggE(ptr %2), !dbg !10
  store ptr %4, ptr %b, align 8
  ret ptr @i
}

define void @_EN4main1A4initE5ArrayI4boolN2_E(ptr %this, [2 x i1] %b) #0 !dbg !11 {
  %b1 = alloca [2 x i1], align 1
  store [2 x i1] %b, ptr %b1, align 1
  %b2 = getelementptr inbounds %A, ptr %this, i32 0, i32 0
  %b.load = load [2 x i1], ptr %b1, align 1
  store [2 x i1] %b.load, ptr %b2, align 1
  ret void
}

define ptr @_EN4main1A2ffE(ptr %this) #0 !dbg !12 {
  %b = getelementptr inbounds %A, ptr %this, i32 0, i32 0
  %1 = getelementptr inbounds [2 x i1], ptr %b, i32 0, i32 1
  ret ptr %1
}

define void @_EN4main1B4initE4char(ptr %this, i8 %a) #0 !dbg !13 {
  %a1 = alloca i8, align 1
  store i8 %a, ptr %a1, align 1
  %a2 = getelementptr inbounds %B, ptr %this, i32 0, i32 0
  %a.load = load i8, ptr %a1, align 1
  store i8 %a.load, ptr %a2, align 1
  ret void
}

define ptr @_EN4main1B2ggE(ptr %this) #0 !dbg !14 {
  %a = getelementptr inbounds %B, ptr %this, i32 0, i32 0
  ret ptr %a
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "return-value-auto-reference.cx")
!4 = distinct !DISubprogram(name: "foo", linkageName: "_EN4main3fooE", scope: !3, file: !3, line: 5, type: !5, scopeLine: 5, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 6, column: 13, scope: !4)
!8 = !DILocation(line: 6, column: 29, scope: !4)
!9 = !DILocation(line: 7, column: 13, scope: !4)
!10 = !DILocation(line: 7, column: 20, scope: !4)
!11 = distinct !DISubprogram(name: "init", linkageName: "_EN4main1A4initE5ArrayI4boolN2_E", scope: !3, file: !3, line: 11, type: !5, scopeLine: 11, spFlags: DISPFlagDefinition, unit: !2)
!12 = distinct !DISubprogram(name: "ff", linkageName: "_EN4main1A2ffE", scope: !3, file: !3, line: 14, type: !5, scopeLine: 14, spFlags: DISPFlagDefinition, unit: !2)
!13 = distinct !DISubprogram(name: "init", linkageName: "_EN4main1B4initE4char", scope: !3, file: !3, line: 19, type: !5, scopeLine: 19, spFlags: DISPFlagDefinition, unit: !2)
!14 = distinct !DISubprogram(name: "gg", linkageName: "_EN4main1B2ggE", scope: !3, file: !3, line: 22, type: !5, scopeLine: 22, spFlags: DISPFlagDefinition, unit: !2)
