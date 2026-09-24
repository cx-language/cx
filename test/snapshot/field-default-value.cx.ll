
%Foo = type { i32 }
%Bar = type { i32, i32 }

define i32 @main() #0 !dbg !4 {
  %foo = alloca %Foo, align 8
  %bar = alloca %Bar, align 8
  call void @_EN4main3Foo4initE5int32(ptr %foo, i32 42), !dbg !7
  call void @_EN4main3Bar4initE5int325int32(ptr %bar, i32 -1, i32 42), !dbg !8
  ret i32 0
}

define void @_EN4main3Foo4initE5int32(ptr %this, i32 %i) #0 !dbg !9 {
  %i1 = alloca i32, align 4
  store i32 %i, ptr %i1, align 4
  %i2 = getelementptr inbounds %Foo, ptr %this, i32 0, i32 0
  %i.load = load i32, ptr %i1, align 4
  store i32 %i.load, ptr %i2, align 4
  ret void
}

define void @_EN4main3Bar4initE5int325int32(ptr %this, i32 %j, i32 %i) #0 !dbg !10 {
  %j1 = alloca i32, align 4
  %i2 = alloca i32, align 4
  store i32 %j, ptr %j1, align 4
  store i32 %i, ptr %i2, align 4
  %i3 = getelementptr inbounds %Bar, ptr %this, i32 0, i32 0
  %i.load = load i32, ptr %i2, align 4
  store i32 %i.load, ptr %i3, align 4
  %j4 = getelementptr inbounds %Bar, ptr %this, i32 0, i32 1
  %j.load = load i32, ptr %j1, align 4
  store i32 %j.load, ptr %j4, align 4
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "field-default-value.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 12, type: !5, scopeLine: 12, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 13, column: 15, scope: !4)
!8 = !DILocation(line: 14, column: 15, scope: !4)
!9 = distinct !DISubprogram(name: "init", linkageName: "_EN4main3Foo4initE5int32", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!10 = distinct !DISubprogram(name: "init", linkageName: "_EN4main3Bar4initE5int325int32", scope: !3, file: !3, line: 7, type: !5, scopeLine: 7, spFlags: DISPFlagDefinition, unit: !2)
