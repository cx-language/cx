
%X = type { ptr }

define i32 @_EN4main3fooE3int(i32 %a) #0 !dbg !4 {
  %a1 = alloca i32, align 4
  store i32 %a, ptr %a1, align 4
  %a.load = load i32, ptr %a1, align 4
  ret i32 %a.load
}

define i32 @main() #0 !dbg !7 {
  %x = alloca %X, align 8
  call void @_EN4main1X4initEF3int_3int(ptr %x, ptr @_EN4main3fooE3int), !dbg !8
  %p = getelementptr inbounds %X, ptr %x, i32 0, i32 0
  %p.load = load ptr, ptr %p, align 8
  %1 = call i32 %p.load(i32 42), !dbg !9
  call void @_EN4main1X1fE(ptr %x), !dbg !10
  ret i32 0
}

define void @_EN4main1X4initEF3int_3int(ptr %this, ptr %p) #0 !dbg !11 {
  %p1 = alloca ptr, align 8
  store ptr %p, ptr %p1, align 8
  %p2 = getelementptr inbounds %X, ptr %this, i32 0, i32 0
  %p.load = load ptr, ptr %p1, align 8
  store ptr %p.load, ptr %p2, align 8
  ret void
}

define void @_EN4main1X1fE(ptr %this) #0 !dbg !12 {
  %p2 = alloca ptr, align 8
  %p = getelementptr inbounds %X, ptr %this, i32 0, i32 0
  %p.load = load ptr, ptr %p, align 8
  %1 = call i32 %p.load(i32 42), !dbg !13
  %p1 = getelementptr inbounds %X, ptr %this, i32 0, i32 0
  %p.load2 = load ptr, ptr %p1, align 8
  %2 = call i32 %p.load2(i32 42), !dbg !14
  %p3 = getelementptr inbounds %X, ptr %this, i32 0, i32 0
  %p.load4 = load ptr, ptr %p3, align 8
  store ptr %p.load4, ptr %p2, align 8
  %p2.load = load ptr, ptr %p2, align 8
  %3 = call i32 %p2.load(i32 42), !dbg !15
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "function-pointer-member-variable.cx")
!4 = distinct !DISubprogram(name: "foo", linkageName: "_EN4main3fooE3int", scope: !3, file: !3, line: 14, type: !5, scopeLine: 14, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 18, type: !5, scopeLine: 18, spFlags: DISPFlagDefinition, unit: !2)
!8 = !DILocation(line: 19, column: 13, scope: !7)
!9 = !DILocation(line: 20, column: 7, scope: !7)
!10 = !DILocation(line: 21, column: 7, scope: !7)
!11 = distinct !DISubprogram(name: "init", linkageName: "_EN4main1X4initEF3int_3int", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!12 = distinct !DISubprogram(name: "f", linkageName: "_EN4main1X1fE", scope: !3, file: !3, line: 6, type: !5, scopeLine: 6, spFlags: DISPFlagDefinition, unit: !2)
!13 = !DILocation(line: 7, column: 14, scope: !12)
!14 = !DILocation(line: 8, column: 9, scope: !12)
!15 = !DILocation(line: 10, column: 9, scope: !12)
