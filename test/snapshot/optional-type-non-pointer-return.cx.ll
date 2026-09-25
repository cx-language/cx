
%0 = type { { i32 } }
%"Optional<int32>" = type { i32, %0 }

define %"Optional<int32>" @_CX1N4main1fEOM3std5int320_() #0 !dbg !4 {
  %i = alloca i32, align 4
  %enum = alloca %"Optional<int32>", align 8
  store i32 1, ptr %i, align 4
  %tag = getelementptr inbounds %"Optional<int32>", ptr %enum, i32 0, i32 0
  store i32 1, ptr %tag, align 4
  %i.load = load i32, ptr %i, align 4
  %1 = insertvalue { i32 } undef, i32 %i.load, 0
  %associatedValue = getelementptr inbounds %"Optional<int32>", ptr %enum, i32 0, i32 1
  store { i32 } %1, ptr %associatedValue, align 4
  %enum.load = load %"Optional<int32>", ptr %enum, align 4
  ret %"Optional<int32>" %enum.load
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "optional-type-non-pointer-return.cx")
!4 = distinct !DISubprogram(name: "f", linkageName: "_CX1N4main1fEOM3std5int320_", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
