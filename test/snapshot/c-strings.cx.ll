
%S = type { [1024 x i8] }
%string = type { %"Slice<char>" }
%"Slice<char>" = type { ptr, i32 }
%StringBuf = type { %"List<char>" }
%"List<char>" = type { ptr, i32, i32 }

@0 = private unnamed_addr constant [2 x i8] c"x\00", align 1
@1 = private unnamed_addr constant [2 x i8] c"x\00", align 1

define i32 @main() #0 !dbg !4 {
  %s = alloca %S, align 8
  %__str = alloca %string, align 8
  %1 = alloca %StringBuf, align 8
  %__str1 = alloca %string, align 8
  %a = getelementptr inbounds %S, ptr %s, i32 0, i32 0
  call void @_EN3std6string4initEP4char5int32(ptr %__str, ptr @0, i32 1), !dbg !7
  %__str.load = load %string, ptr %__str, align 8
  %2 = call i1 @_EN3stdeqEP4char6string(ptr %a, %string %__str.load), !dbg !8
  %a2 = getelementptr inbounds %S, ptr %s, i32 0, i32 0
  call void @_EN3std6string4initEP4char5int32(ptr %__str1, ptr @1, i32 1), !dbg !7
  %__str.load3 = load %string, ptr %__str1, align 8
  call void @_EN3std9StringBuf4initE6string(ptr %1, %string %__str.load3), !dbg !9
  %3 = call i1 @_EN3stdeqEAU_4charR9StringBuf(ptr %a2, ptr %1), !dbg !10
  %4 = xor i1 %3, true
  ret i32 0
}

declare void @_EN3std6string4initEP4char5int32(ptr, ptr, i32) #0

declare i1 @_EN3stdeqEP4char6string(ptr, %string) #0

declare void @_EN3std9StringBuf4initE6string(ptr, %string) #0

declare i1 @_EN3stdeqEAU_4charR9StringBuf(ptr, ptr) #0

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "c-strings.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 7, type: !5, scopeLine: 7, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 7, column: 6, scope: !4)
!8 = !DILocation(line: 9, column: 9, scope: !4)
!9 = !DILocation(line: 10, column: 5, scope: !4)
!10 = !DILocation(line: 10, column: 20, scope: !4)
