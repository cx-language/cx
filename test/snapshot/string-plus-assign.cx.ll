
%StringBuf = type { %"List<char>" }
%"List<char>" = type { ptr, i32, i32 }
%string = type { %"Slice<char>" }
%"Slice<char>" = type { ptr, i32 }

@0 = private unnamed_addr constant [1 x i8] zeroinitializer, align 1

define i32 @main() #0 !dbg !4 {
  %a = alloca %StringBuf, align 8
  %__str = alloca %string, align 8
  call void @_CX1N3stdM3std9StringBuf4initE4void0_(ptr %a), !dbg !7
  %a.load = load %StringBuf, ptr %a, align 8
  call void @_CX1N3stdM3std6string4initE4void2_PM3std4charM3std5int32(ptr %__str, ptr @0, i32 0), !dbg !8
  %__str.load = load %string, ptr %__str, align 8
  %1 = call %StringBuf @_CX1N3stdo2plEM3std9StringBuf2_M3std9StringBufM3std6string(%StringBuf %a.load, %string %__str.load), !dbg !9
  store %StringBuf %1, ptr %a, align 8
  call void @_CX1N3stdM3std9StringBuf6deinitE4void0_(ptr %a), !dbg !8
  ret i32 0
}

declare void @_CX1N3stdM3std9StringBuf4initE4void0_(ptr) #0

declare void @_CX1N3stdM3std6string4initE4void2_PM3std4charM3std5int32(ptr, ptr, i32) #0

declare %StringBuf @_CX1N3stdo2plEM3std9StringBuf2_M3std9StringBufM3std6string(%StringBuf, %string) #0

declare void @_CX1N3stdM3std9StringBuf6deinitE4void0_(ptr) #0

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
