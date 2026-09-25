
%string = type { %"Slice<char>" }
%"Slice<char>" = type { ptr, i32 }

@0 = private unnamed_addr constant [1 x i8] zeroinitializer, align 1

define i32 @main() #0 !dbg !4 {
  %__str = alloca %string, align 8
  call void @_CX1N3stdM3std6string4initE4void2_PM3std4charM3std5int32(ptr %__str, ptr @0, i32 0), !dbg !7
  %1 = call i32 @_CX1N3stdM3std6string4sizeEM3std5int320_(ptr %__str), !dbg !8
  call void @_CX1N3std10checkLeaksE4void0_(), !dbg !7
  ret i32 %1
}

declare void @_CX1N3stdM3std6string4initE4void2_PM3std4charM3std5int32(ptr, ptr, i32) #0

declare i32 @_CX1N3stdM3std6string4sizeEM3std5int320_(ptr) #0

declare void @_CX1N3std10checkLeaksE4void0_() #0

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "empty-string-literal.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 3, column: 5, scope: !4)
!8 = !DILocation(line: 4, column: 15, scope: !4)
