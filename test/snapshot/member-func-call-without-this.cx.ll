
%S = type {}
%"Foo<int>" = type {}

define i32 @main() #0 !dbg !4 {
  %s = alloca %S, align 8
  %t = alloca %"Foo<int>", align 8
  call void @_EN4main1S4initE(ptr %s), !dbg !7
  call void @_EN4main1S3fooE(ptr %s), !dbg !8
  call void @_EN4main3FooI3intE4initE(ptr %t), !dbg !9
  call void @_EN4main3FooI3intE3bazE(ptr %t), !dbg !10
  call void @_EN4main3FooI3intE6deinitE(ptr %t), !dbg !11
  ret i32 0
}

define void @_EN4main1S4initE(ptr %this) #0 !dbg !12 {
  ret void
}

define void @_EN4main1S3fooE(ptr %this) #0 !dbg !13 {
  call void @_EN4main1S3barE(ptr %this), !dbg !14
  ret void
}

define void @_EN4main3FooI3intE4initE(ptr %this) #0 !dbg !15 {
  call void @_EN4main3FooI3intE3bazE(ptr %this), !dbg !16
  ret void
}

define void @_EN4main3FooI3intE3bazE(ptr %this) #0 !dbg !17 {
  call void @_EN4main3FooI3intE3quxE(ptr %this), !dbg !18
  ret void
}

define void @_EN4main3FooI3intE6deinitE(ptr %this) #0 !dbg !19 {
  call void @_EN4main3FooI3intE3bazE(ptr %this), !dbg !20
  ret void
}

define void @_EN4main1S3barE(ptr %this) #0 !dbg !21 {
  ret void
}

define void @_EN4main3FooI3intE3quxE(ptr %this) #0 !dbg !22 {
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "member-func-call-without-this.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 27, type: !5, scopeLine: 27, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 28, column: 13, scope: !4)
!8 = !DILocation(line: 29, column: 7, scope: !4)
!9 = !DILocation(line: 31, column: 13, scope: !4)
!10 = !DILocation(line: 32, column: 7, scope: !4)
!11 = !DILocation(line: 27, column: 6, scope: !4)
!12 = distinct !DISubprogram(name: "init", linkageName: "_EN4main1S4initE", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!13 = distinct !DISubprogram(name: "foo", linkageName: "_EN4main1S3fooE", scope: !3, file: !3, line: 4, type: !5, scopeLine: 4, spFlags: DISPFlagDefinition, unit: !2)
!14 = !DILocation(line: 5, column: 9, scope: !13)
!15 = distinct !DISubprogram(name: "init", linkageName: "_EN4main3FooI3intE4initE", scope: !3, file: !3, line: 12, type: !5, scopeLine: 12, spFlags: DISPFlagDefinition, unit: !2)
!16 = !DILocation(line: 13, column: 9, scope: !15)
!17 = distinct !DISubprogram(name: "baz", linkageName: "_EN4main3FooI3intE3bazE", scope: !3, file: !3, line: 20, type: !5, scopeLine: 20, spFlags: DISPFlagDefinition, unit: !2)
!18 = !DILocation(line: 21, column: 9, scope: !17)
!19 = distinct !DISubprogram(name: "deinit", linkageName: "_EN4main3FooI3intE6deinitE", scope: !3, file: !3, line: 16, type: !5, scopeLine: 16, spFlags: DISPFlagDefinition, unit: !2)
!20 = !DILocation(line: 17, column: 9, scope: !19)
!21 = distinct !DISubprogram(name: "bar", linkageName: "_EN4main1S3barE", scope: !3, file: !3, line: 8, type: !5, scopeLine: 8, spFlags: DISPFlagDefinition, unit: !2)
!22 = distinct !DISubprogram(name: "qux", linkageName: "_EN4main3FooI3intE3quxE", scope: !3, file: !3, line: 24, type: !5, scopeLine: 24, spFlags: DISPFlagDefinition, unit: !2)
