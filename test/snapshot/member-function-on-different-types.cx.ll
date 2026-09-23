
%Foo = type { i32 }
%Bar = type { i32 }

define i32 @main() #0 !dbg !4 {
  %1 = alloca %Foo, align 8
  %2 = alloca %Bar, align 8
  call void @_EN4main3Foo4initE3int(ptr %1, i32 1), !dbg !7
  call void @_EN4main3Foo7doStuffE(ptr %1), !dbg !8
  call void @_EN4main3Bar4initE3int(ptr %2, i32 2), !dbg !9
  call void @_EN4main3Bar7doStuffE(ptr %2), !dbg !10
  ret i32 0
}

define void @_EN4main3Foo4initE3int(ptr %this, i32 %i) #0 !dbg !11 {
  %i1 = alloca i32, align 4
  store i32 %i, ptr %i1, align 4
  %i2 = getelementptr inbounds %Foo, ptr %this, i32 0, i32 0
  %i.load = load i32, ptr %i1, align 4
  store i32 %i.load, ptr %i2, align 4
  ret void
}

define void @_EN4main3Foo7doStuffE(ptr %this) #0 !dbg !12 {
  ret void
}

define void @_EN4main3Bar4initE3int(ptr %this, i32 %i) #0 !dbg !13 {
  %i1 = alloca i32, align 4
  store i32 %i, ptr %i1, align 4
  %i2 = getelementptr inbounds %Bar, ptr %this, i32 0, i32 0
  %i.load = load i32, ptr %i1, align 4
  store i32 %i.load, ptr %i2, align 4
  ret void
}

define void @_EN4main3Bar7doStuffE(ptr %this) #0 !dbg !14 {
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "member-function-on-different-types.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 13, type: !5, scopeLine: 13, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 14, column: 5, scope: !4)
!8 = !DILocation(line: 14, column: 12, scope: !4)
!9 = !DILocation(line: 15, column: 5, scope: !4)
!10 = !DILocation(line: 15, column: 12, scope: !4)
!11 = distinct !DISubprogram(name: "init", linkageName: "_EN4main3Foo4initE3int", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!12 = distinct !DISubprogram(name: "doStuff", linkageName: "_EN4main3Foo7doStuffE", scope: !3, file: !3, line: 5, type: !5, scopeLine: 5, spFlags: DISPFlagDefinition, unit: !2)
!13 = distinct !DISubprogram(name: "init", linkageName: "_EN4main3Bar4initE3int", scope: !3, file: !3, line: 8, type: !5, scopeLine: 8, spFlags: DISPFlagDefinition, unit: !2)
!14 = distinct !DISubprogram(name: "doStuff", linkageName: "_EN4main3Bar7doStuffE", scope: !3, file: !3, line: 10, type: !5, scopeLine: 10, spFlags: DISPFlagDefinition, unit: !2)
