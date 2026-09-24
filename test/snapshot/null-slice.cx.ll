
%0 = type { { %"Slice<int8>" } }
%"Optional<Slice<int8>>" = type { i32, %0 }
%"Slice<int8>" = type { ptr, i32 }

define i32 @main() #0 !dbg !4 {
  %a = alloca %"Optional<Slice<int8>>", align 8
  %enum = alloca %"Optional<Slice<int8>>", align 8
  %tag = getelementptr inbounds %"Optional<Slice<int8>>", ptr %enum, i32 0, i32 0
  store i32 0, ptr %tag, align 4
  %enum.load = alloca %"Optional<Slice<int8>>", align 8
  call void @llvm.memcpy.p0.p0.i64(ptr align 8 %enum.load, ptr align 8 %enum, i64 24, i1 false)
  call void @llvm.memcpy.p0.p0.i64(ptr align 8 %a, ptr align 8 %enum.load, i64 24, i1 false)
  ret i32 0
}

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias writeonly captures(none), ptr noalias readonly captures(none), i64, i1 immarg) #1

attributes #0 = { "frame-pointer"="all" }
attributes #1 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "null-slice.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
