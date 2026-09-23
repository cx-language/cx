
%A = type { i32, i32 }
%B = type { i32, i32 }

define i32 @main() #0 !dbg !4 {
  %a = alloca %A, align 8
  %b = alloca %B, align 8
  call void @_EN4main1A4initE(ptr %a), !dbg !7
  %1 = call i32 @_EN4main1A3fooE(ptr %a), !dbg !8
  call void @_EN4main1A3barE(ptr %a), !dbg !9
  call void @_EN4main1B4initE(ptr %b), !dbg !10
  %2 = call i32 @_EN4main1B3fooE(ptr %b), !dbg !11
  call void @_EN4main1B3barE(ptr %b), !dbg !12
  ret i32 0
}

define void @_EN4main1A4initE(ptr %this) #0 !dbg !13 {
  %j = getelementptr inbounds %A, ptr %this, i32 0, i32 1
  store i32 42, ptr %j, align 4
  ret void
}

define i32 @_EN4main1A3fooE(ptr %this) #0 !dbg !14 {
  %j = getelementptr inbounds %A, ptr %this, i32 0, i32 1
  %j.load = load i32, ptr %j, align 4
  ret i32 %j.load
}

define void @_EN4main1A3barE(ptr %this) #0 !dbg !15 {
  %j = getelementptr inbounds %A, ptr %this, i32 0, i32 1
  store i32 1, ptr %j, align 4
  ret void
}

define void @_EN4main1B4initE(ptr %this) #0 !dbg !16 {
  %j = getelementptr inbounds %B, ptr %this, i32 0, i32 1
  store i32 42, ptr %j, align 4
  ret void
}

define i32 @_EN4main1B3fooE(ptr %this) #0 !dbg !17 {
  %j = getelementptr inbounds %B, ptr %this, i32 0, i32 1
  %j.load = load i32, ptr %j, align 4
  ret i32 %j.load
}

define void @_EN4main1B3barE(ptr %this) #0 !dbg !18 {
  %j = getelementptr inbounds %B, ptr %this, i32 0, i32 1
  store i32 1, ptr %j, align 4
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "own-member-access-without-this.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 37, type: !5, scopeLine: 37, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 38, column: 13, scope: !4)
!8 = !DILocation(line: 39, column: 7, scope: !4)
!9 = !DILocation(line: 40, column: 7, scope: !4)
!10 = !DILocation(line: 41, column: 13, scope: !4)
!11 = !DILocation(line: 42, column: 7, scope: !4)
!12 = !DILocation(line: 43, column: 7, scope: !4)
!13 = distinct !DISubprogram(name: "init", linkageName: "_EN4main1A4initE", scope: !3, file: !3, line: 7, type: !5, scopeLine: 7, spFlags: DISPFlagDefinition, unit: !2)
!14 = distinct !DISubprogram(name: "foo", linkageName: "_EN4main1A3fooE", scope: !3, file: !3, line: 11, type: !5, scopeLine: 11, spFlags: DISPFlagDefinition, unit: !2)
!15 = distinct !DISubprogram(name: "bar", linkageName: "_EN4main1A3barE", scope: !3, file: !3, line: 15, type: !5, scopeLine: 15, spFlags: DISPFlagDefinition, unit: !2)
!16 = distinct !DISubprogram(name: "init", linkageName: "_EN4main1B4initE", scope: !3, file: !3, line: 24, type: !5, scopeLine: 24, spFlags: DISPFlagDefinition, unit: !2)
!17 = distinct !DISubprogram(name: "foo", linkageName: "_EN4main1B3fooE", scope: !3, file: !3, line: 28, type: !5, scopeLine: 28, spFlags: DISPFlagDefinition, unit: !2)
!18 = distinct !DISubprogram(name: "bar", linkageName: "_EN4main1B3barE", scope: !3, file: !3, line: 32, type: !5, scopeLine: 32, spFlags: DISPFlagDefinition, unit: !2)
