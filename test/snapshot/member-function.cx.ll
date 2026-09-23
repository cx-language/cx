
%Foo = type { i32 }

define i32 @main() #0 !dbg !4 {
  %foo = alloca %Foo, align 8
  %i = alloca i32, align 4
  call void @_EN4main3Foo4initE(ptr %foo), !dbg !7
  call void @_EN4main3Foo3barE(ptr %foo), !dbg !8
  %1 = call i32 @_EN4main3Foo3quxE(ptr %foo), !dbg !9
  store i32 %1, ptr %i, align 4
  ret i32 0
}

define void @_EN4main3Foo4initE(ptr %this) #0 !dbg !10 {
  %baz = getelementptr inbounds %Foo, ptr %this, i32 0, i32 0
  store i32 42, ptr %baz, align 4
  ret void
}

define void @_EN4main3Foo3barE(ptr %this) #0 !dbg !11 {
  %baz = getelementptr inbounds %Foo, ptr %this, i32 0, i32 0
  %baz.load = load i32, ptr %baz, align 4
  %1 = add i32 %baz.load, 1
  store i32 %1, ptr %baz, align 4
  ret void
}

define i32 @_EN4main3Foo3quxE(ptr %this) #0 !dbg !12 {
  %baz = getelementptr inbounds %Foo, ptr %this, i32 0, i32 0
  %baz.load = load i32, ptr %baz, align 4
  ret i32 %baz.load
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "member-function.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 13, type: !5, scopeLine: 13, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 14, column: 15, scope: !4)
!8 = !DILocation(line: 15, column: 9, scope: !4)
!9 = !DILocation(line: 16, column: 17, scope: !4)
!10 = distinct !DISubprogram(name: "init", linkageName: "_EN4main3Foo4initE", scope: !3, file: !3, line: 6, type: !5, scopeLine: 6, spFlags: DISPFlagDefinition, unit: !2)
!11 = distinct !DISubprogram(name: "bar", linkageName: "_EN4main3Foo3barE", scope: !3, file: !3, line: 8, type: !5, scopeLine: 8, spFlags: DISPFlagDefinition, unit: !2)
!12 = distinct !DISubprogram(name: "qux", linkageName: "_EN4main3Foo3quxE", scope: !3, file: !3, line: 10, type: !5, scopeLine: 10, spFlags: DISPFlagDefinition, unit: !2)
