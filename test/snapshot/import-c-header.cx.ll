
%Foo = type { i32, ptr }

@0 = private unnamed_addr constant [8 x i8] c"foo bar\00", align 1
@1 = private unnamed_addr constant [4 x i8] c"foo\00", align 1

define i32 @main() #0 !dbg !4 {
  %f = alloca %Foo, align 8
  %bar = alloca i32, align 4
  %1 = call i32 @puts(ptr @0), !dbg !7
  %bar1 = getelementptr inbounds %Foo, ptr %f, i32 0, i32 0
  store i32 47, ptr %bar1, align 4
  %baz = getelementptr inbounds %Foo, ptr %f, i32 0, i32 1
  store ptr @1, ptr %baz, align 8
  %2 = call i32 @getBar(ptr %f), !dbg !8
  store i32 %2, ptr %bar, align 4
  call void @_CX1N3std10checkLeaksE4void0_(), !dbg !9
  ret i32 0
}

declare i32 @puts(ptr) #0

declare i32 @getBar(ptr) #0

declare void @_CX1N3std10checkLeaksE4void0_() #0

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "import-c-header.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 6, type: !5, scopeLine: 6, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 7, column: 5, scope: !4)
!8 = !DILocation(line: 11, column: 15, scope: !4)
!9 = !DILocation(line: 6, column: 6, scope: !4)
