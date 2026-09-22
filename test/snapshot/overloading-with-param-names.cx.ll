
%Foo = type { i32 }

define void @_EN4main3fooE1i3int(i32 %i) #0 !dbg !4 {
  %i1 = alloca i32, align 4
  %1 = alloca %Foo, align 8
  %2 = alloca %Foo, align 8
  store i32 %i, ptr %i1, align 4
  %i.load = load i32, ptr %i1, align 4
  call void @_EN4main3Foo4initE1i3int(ptr %1, i32 %i.load), !dbg !7
  %i.load2 = load i32, ptr %i1, align 4
  call void @_EN4main3Foo3fooE1i3int(ptr %1, i32 %i.load2), !dbg !8
  %i.load3 = load i32, ptr %i1, align 4
  call void @_EN4main3Foo4initE3qux3int(ptr %2, i32 %i.load3), !dbg !9
  %i.load4 = load i32, ptr %i1, align 4
  call void @_EN4main3Foo3fooE3qux3int(ptr %2, i32 %i.load4), !dbg !10
  ret void
}

define void @_EN4main3Foo4initE1i3int(ptr %this, i32 %i) #0 !dbg !11 {
  %i1 = alloca i32, align 4
  store i32 %i, ptr %i1, align 4
  ret void
}

define void @_EN4main3Foo3fooE1i3int(ptr %this, i32 %i) #0 !dbg !12 {
  %i1 = alloca i32, align 4
  store i32 %i, ptr %i1, align 4
  ret void
}

define void @_EN4main3Foo4initE3qux3int(ptr %this, i32 %qux) #0 !dbg !13 {
  %qux1 = alloca i32, align 4
  store i32 %qux, ptr %qux1, align 4
  ret void
}

define void @_EN4main3Foo3fooE3qux3int(ptr %this, i32 %qux) #0 !dbg !14 {
  %qux1 = alloca i32, align 4
  store i32 %qux, ptr %qux1, align 4
  ret void
}

define void @_EN4main3fooE3qux3int(i32 %qux) #0 !dbg !15 {
  %qux1 = alloca i32, align 4
  store i32 %qux, ptr %qux1, align 4
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "overloading-with-param-names.cx")
!4 = distinct !DISubprogram(name: "foo", linkageName: "_EN4main3fooE1i3int", scope: !3, file: !3, line: 11, type: !5, scopeLine: 11, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 12, column: 5, scope: !4)
!8 = !DILocation(line: 12, column: 16, scope: !4)
!9 = !DILocation(line: 13, column: 5, scope: !4)
!10 = !DILocation(line: 13, column: 18, scope: !4)
!11 = distinct !DISubprogram(name: "init", linkageName: "_EN4main3Foo4initE1i3int", scope: !3, file: !3, line: 5, type: !5, scopeLine: 5, spFlags: DISPFlagDefinition, unit: !2)
!12 = distinct !DISubprogram(name: "foo", linkageName: "_EN4main3Foo3fooE1i3int", scope: !3, file: !3, line: 7, type: !5, scopeLine: 7, spFlags: DISPFlagDefinition, unit: !2)
!13 = distinct !DISubprogram(name: "init", linkageName: "_EN4main3Foo4initE3qux3int", scope: !3, file: !3, line: 6, type: !5, scopeLine: 6, spFlags: DISPFlagDefinition, unit: !2)
!14 = distinct !DISubprogram(name: "foo", linkageName: "_EN4main3Foo3fooE3qux3int", scope: !3, file: !3, line: 8, type: !5, scopeLine: 8, spFlags: DISPFlagDefinition, unit: !2)
!15 = distinct !DISubprogram(name: "foo", linkageName: "_EN4main3fooE3qux3int", scope: !3, file: !3, line: 16, type: !5, scopeLine: 16, spFlags: DISPFlagDefinition, unit: !2)
