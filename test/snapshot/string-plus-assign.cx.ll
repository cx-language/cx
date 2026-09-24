
%StringBuf = type { %"List<char>" }
%"List<char>" = type { ptr, i32, i32 }
%string = type { %"Slice<char>" }
%"Slice<char>" = type { ptr, i32 }

@0 = private unnamed_addr constant [1 x i8] zeroinitializer, align 1

define i32 @main() #0 !dbg !4 {
  %a = alloca %StringBuf, align 8
  %__str = alloca %string, align 8
  call void @_EN3std9StringBuf4initE(ptr %a), !dbg !7
  %a.load = load %StringBuf, ptr %a, align 8
  call void @_EN3std6string4initEP4char3int(ptr %__str, ptr @0, i32 0), !dbg !8
  %__str.load = load %string, ptr %__str, align 8
  %1 = call %StringBuf @_EN3stdplE9StringBuf6string(%StringBuf %a.load, %string %__str.load), !dbg !9
  store %StringBuf %1, ptr %a, align 8
  call void @_EN3std9StringBuf6deinitE(ptr %a), !dbg !8
  ret i32 0
}

define void @_EN3std9StringBuf4initE(ptr %this) #0 !dbg !10 {
  %1 = alloca %"List<char>", align 8
  %characters = getelementptr inbounds %StringBuf, ptr %this, i32 0, i32 0
  call void @_EN3std4ListI4charE4initE(ptr %1), !dbg !12
  %.load = load %"List<char>", ptr %1, align 8
  store %"List<char>" %.load, ptr %characters, align 8
  %characters1 = getelementptr inbounds %StringBuf, ptr %this, i32 0, i32 0
  %2 = call ptr @_EN3std4ListI4charE4pushE4char(ptr %characters1, i8 0), !dbg !13
  ret void
}

declare void @_EN3std6string4initEP4char3int(ptr, ptr, i32) #0

declare %StringBuf @_EN3stdplE9StringBuf6string(%StringBuf, %string) #0

declare void @_EN3std9StringBuf6deinitE(ptr) #0

define void @_EN3std4ListI4charE4initE(ptr %this) #0 !dbg !14 {
  %size = getelementptr inbounds %"List<char>", ptr %this, i32 0, i32 1
  store i32 0, ptr %size, align 4
  %capacity = getelementptr inbounds %"List<char>", ptr %this, i32 0, i32 2
  store i32 0, ptr %capacity, align 4
  ret void
}

declare ptr @_EN3std4ListI4charE4pushE4char(ptr, i8) #0

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "string-plus-assign.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 4, column: 13, scope: !4)
!8 = !DILocation(line: 3, column: 6, scope: !4)
!9 = !DILocation(line: 5, column: 7, scope: !4)
!10 = distinct !DISubprogram(name: "init", linkageName: "_EN3std9StringBuf4initE", scope: !11, file: !11, line: 7, type: !5, scopeLine: 7, spFlags: DISPFlagDefinition, unit: !2)
!11 = !DIFile(filename: "StringBuf.cx")
!12 = !DILocation(line: 8, column: 22, scope: !10)
!13 = !DILocation(line: 9, column: 20, scope: !10)
!14 = distinct !DISubprogram(name: "init", linkageName: "_EN3std4ListI4charE4initE", scope: !15, file: !15, line: 8, type: !5, scopeLine: 8, spFlags: DISPFlagDefinition, unit: !2)
!15 = !DIFile(filename: "List.cx")
