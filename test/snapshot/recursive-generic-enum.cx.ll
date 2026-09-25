
%0 = type { { [12 x i32] } }
%E = type { i32, %0 }
%"S<E>" = type { ptr }

define i32 @main() #0 !dbg !4 {
  %inner = alloca %E, align 8
  %enum = alloca %E, align 8
  %1 = alloca %"S<E>", align 8
  %e = alloca %E, align 8
  %enum1 = alloca %E, align 8
  %2 = alloca %"S<E>", align 8
  %tag = getelementptr inbounds %E, ptr %enum, i32 0, i32 0
  store i32 0, ptr %tag, align 4
  call void @_CX1N4mainM4main1SIM4main1EE4initE4void1_OPM4main1E(ptr %1, ptr null), !dbg !7
  %.load = load %"S<E>", ptr %1, align 8
  %3 = insertvalue { %"S<E>" } undef, %"S<E>" %.load, 0
  %associatedValue = getelementptr inbounds %E, ptr %enum, i32 0, i32 1
  store { %"S<E>" } %3, ptr %associatedValue, align 8
  %enum.load = alloca %E, align 8
  call void @llvm.memcpy.p0.p0.i64(ptr align 4 %enum.load, ptr align 4 %enum, i64 52, i1 false)
  call void @llvm.memcpy.p0.p0.i64(ptr align 4 %inner, ptr align 4 %enum.load, i64 52, i1 false)
  %tag2 = getelementptr inbounds %E, ptr %enum1, i32 0, i32 0
  store i32 0, ptr %tag2, align 4
  call void @_CX1N4mainM4main1SIM4main1EE4initE4void1_OPM4main1E(ptr %2, ptr %inner), !dbg !8
  %.load3 = load %"S<E>", ptr %2, align 8
  %4 = insertvalue { %"S<E>" } undef, %"S<E>" %.load3, 0
  %associatedValue4 = getelementptr inbounds %E, ptr %enum1, i32 0, i32 1
  store { %"S<E>" } %4, ptr %associatedValue4, align 8
  %enum.load5 = alloca %E, align 8
  call void @llvm.memcpy.p0.p0.i64(ptr align 4 %enum.load5, ptr align 4 %enum1, i64 52, i1 false)
  call void @llvm.memcpy.p0.p0.i64(ptr align 4 %e, ptr align 4 %enum.load5, i64 52, i1 false)
  ret i32 0
}

define void @_CX1N4mainM4main1SIM4main1EE4initE4void1_OPM4main1E(ptr %this, ptr %e) #0 !dbg !9 {
  %e1 = alloca ptr, align 8
  store ptr %e, ptr %e1, align 8
  %e2 = getelementptr inbounds %"S<E>", ptr %this, i32 0, i32 0
  %e.load = load ptr, ptr %e1, align 8
  store ptr %e.load, ptr %e2, align 8
  ret void
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
!3 = !DIFile(filename: "recursive-generic-enum.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 12, type: !5, scopeLine: 12, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 13, column: 21, scope: !4)
!8 = !DILocation(line: 14, column: 15, scope: !4)
!9 = distinct !DISubprogram(name: "init", linkageName: "_CX1N4mainM4main1SIM4main1EE4initE4void1_OPM4main1E", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
