
%Foo = type { i32 }
%Bar = type { i32 }

define void @_EN4main3quxEP3FooP3Bar(ptr %f, ptr %b) #0 !dbg !4 {
  %f1 = alloca ptr, align 8
  %b2 = alloca ptr, align 8
  store ptr %f, ptr %f1, align 8
  store ptr %b, ptr %b2, align 8
  %f.load = load ptr, ptr %f1, align 8
  call void @_EN4main3Foo3fooE(ptr %f.load), !dbg !7
  %b.load = load ptr, ptr %b2, align 8
  call void @_EN4main3Bar3barE(ptr %b.load), !dbg !8
  ret void
}

define void @_EN4main3Foo3fooE(ptr %this) #0 !dbg !9 {
  ret void
}

define void @_EN4main3Bar3barE(ptr %this) #0 !dbg !10 {
  ret void
}

define i32 @main() #0 !dbg !11 {
  %f = alloca %Foo, align 8
  %b = alloca %Bar, align 8
  call void @_EN4main3Foo4initE(ptr %f), !dbg !12
  call void @_EN4main3Bar4initE(ptr %b), !dbg !13
  call void @_EN4main3quxEP3FooP3Bar(ptr %f, ptr %b), !dbg !14
  ret i32 0
}

define void @_EN4main3Foo4initE(ptr %this) #0 !dbg !15 {
  %i = getelementptr inbounds %Foo, ptr %this, i32 0, i32 0
  store i32 0, ptr %i, align 4
  ret void
}

define void @_EN4main3Bar4initE(ptr %this) #0 !dbg !16 {
  %i = getelementptr inbounds %Bar, ptr %this, i32 0, i32 0
  store i32 0, ptr %i, align 4
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "member-function-call-via-pointer-parameter.cx")
!4 = distinct !DISubprogram(name: "qux", linkageName: "_EN4main3quxEP3FooP3Bar", scope: !3, file: !3, line: 15, type: !5, scopeLine: 15, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 16, column: 7, scope: !4)
!8 = !DILocation(line: 17, column: 7, scope: !4)
!9 = distinct !DISubprogram(name: "foo", linkageName: "_EN4main3Foo3fooE", scope: !3, file: !3, line: 6, type: !5, scopeLine: 6, spFlags: DISPFlagDefinition, unit: !2)
!10 = distinct !DISubprogram(name: "bar", linkageName: "_EN4main3Bar3barE", scope: !3, file: !3, line: 12, type: !5, scopeLine: 12, spFlags: DISPFlagDefinition, unit: !2)
!11 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 20, type: !5, scopeLine: 20, spFlags: DISPFlagDefinition, unit: !2)
!12 = !DILocation(line: 21, column: 13, scope: !11)
!13 = !DILocation(line: 22, column: 13, scope: !11)
!14 = !DILocation(line: 23, column: 5, scope: !11)
!15 = distinct !DISubprogram(name: "init", linkageName: "_EN4main3Foo4initE", scope: !3, file: !3, line: 5, type: !5, scopeLine: 5, spFlags: DISPFlagDefinition, unit: !2)
!16 = distinct !DISubprogram(name: "init", linkageName: "_EN4main3Bar4initE", scope: !3, file: !3, line: 11, type: !5, scopeLine: 11, spFlags: DISPFlagDefinition, unit: !2)
