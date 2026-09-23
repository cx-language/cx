
%Foo = type { i32, i1 }

define i32 @main() #0 !dbg !4 {
  %f = alloca %Foo, align 8
  %bar = alloca i32, align 4
  %qux = alloca i1, align 1
  call void @_EN4main3Foo4initE3int4bool(ptr %f, i32 666, i1 true), !dbg !7
  %a = getelementptr inbounds %Foo, ptr %f, i32 0, i32 0
  %a.load = load i32, ptr %a, align 4
  %1 = add i32 %a.load, 1
  store i32 %1, ptr %a, align 4
  %a1 = getelementptr inbounds %Foo, ptr %f, i32 0, i32 0
  %a.load2 = load i32, ptr %a1, align 4
  store i32 %a.load2, ptr %bar, align 4
  %b = getelementptr inbounds %Foo, ptr %f, i32 0, i32 1
  %b.load = load i1, ptr %b, align 1
  store i1 %b.load, ptr %qux, align 1
  ret i32 0
}

define void @_EN4main3Foo4initE3int4bool(ptr %this, i32 %a, i1 %b) #0 !dbg !8 {
  %a1 = alloca i32, align 4
  %b2 = alloca i1, align 1
  store i32 %a, ptr %a1, align 4
  store i1 %b, ptr %b2, align 1
  %a3 = getelementptr inbounds %Foo, ptr %this, i32 0, i32 0
  %a.load = load i32, ptr %a1, align 4
  store i32 %a.load, ptr %a3, align 4
  %b4 = getelementptr inbounds %Foo, ptr %this, i32 0, i32 1
  %b.load = load i1, ptr %b2, align 1
  store i1 %b.load, ptr %b4, align 1
  %a5 = getelementptr inbounds %Foo, ptr %this, i32 0, i32 0
  %a.load6 = load i32, ptr %a5, align 4
  %1 = add i32 %a.load6, 1
  store i32 %1, ptr %a5, align 4
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "copyable-type-definition.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 14, type: !5, scopeLine: 14, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 15, column: 13, scope: !4)
!8 = distinct !DISubprogram(name: "init", linkageName: "_EN4main3Foo4initE3int4bool", scope: !3, file: !3, line: 7, type: !5, scopeLine: 7, spFlags: DISPFlagDefinition, unit: !2)
