
%S2 = type { double, double, double, double }
%S = type { [4 x double] }

define void @_EN4main19returnsLargeStruct2E(ptr sret(%S2) align 8 %sret.arg) #0 !dbg !4 {
  %1 = alloca %S2, align 8
  call void @_EN4main2S24initE7float647float647float647float64(ptr %1, double 0.000000e+00, double 1.000000e+00, double 2.000000e+00, double 3.000000e+00), !dbg !7
  %.load = alloca %S2, align 8
  call void @llvm.memcpy.p0.p0.i64(ptr align 8 %.load, ptr align 8 %1, i64 32, i1 false)
  call void @llvm.memcpy.p0.p0.i64(ptr align 8 %sret.arg, ptr align 8 %.load, i64 32, i1 false)
  ret void
}

define void @_EN4main2S24initE7float647float647float647float64(ptr %this, double %x, double %y, double %z, double %w) #0 !dbg !8 {
  %x1 = alloca double, align 8
  %y2 = alloca double, align 8
  %z3 = alloca double, align 8
  %w4 = alloca double, align 8
  store double %x, ptr %x1, align 8
  store double %y, ptr %y2, align 8
  store double %z, ptr %z3, align 8
  store double %w, ptr %w4, align 8
  %x5 = getelementptr inbounds %S2, ptr %this, i32 0, i32 0
  %x.load = load double, ptr %x1, align 8
  store double %x.load, ptr %x5, align 8
  %y6 = getelementptr inbounds %S2, ptr %this, i32 0, i32 1
  %y.load = load double, ptr %y2, align 8
  store double %y.load, ptr %y6, align 8
  %z7 = getelementptr inbounds %S2, ptr %this, i32 0, i32 2
  %z.load = load double, ptr %z3, align 8
  store double %z.load, ptr %z7, align 8
  %w8 = getelementptr inbounds %S2, ptr %this, i32 0, i32 3
  %w.load = load double, ptr %w4, align 8
  store double %w.load, ptr %w8, align 8
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias writeonly captures(none), ptr noalias readonly captures(none), i64, i1 immarg) #1

define i32 @main() #0 !dbg !9 {
  %s = alloca %S, align 8
  %s2 = alloca %S2, align 8
  %sret.alloca = alloca %S, align 8, !dbg !10
  call void @returnsLargeStruct(ptr sret(%S) align 8 %sret.alloca), !dbg !10
  call void @llvm.memcpy.p0.p0.i64(ptr align 8 %s, ptr align 8 %sret.alloca, i64 32, i1 false)
  %sret.alloca1 = alloca %S2, align 8, !dbg !11
  call void @_EN4main19returnsLargeStruct2E(ptr sret(%S2) align 8 %sret.alloca1), !dbg !11
  call void @llvm.memcpy.p0.p0.i64(ptr align 8 %s2, ptr align 8 %sret.alloca1, i64 32, i1 false)
  ret i32 0
}

declare void @returnsLargeStruct(ptr sret(%S) align 8) #0

attributes #0 = { "frame-pointer"="all" }
attributes #1 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "struct-return.cx")
!4 = distinct !DISubprogram(name: "returnsLargeStruct2", linkageName: "_EN4main19returnsLargeStruct2E", scope: !3, file: !3, line: 12, type: !5, scopeLine: 12, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 13, column: 12, scope: !4)
!8 = distinct !DISubprogram(name: "init", linkageName: "_EN4main2S24initE7float647float647float647float64", scope: !3, file: !3, line: 5, type: !5, scopeLine: 5, spFlags: DISPFlagDefinition, unit: !2)
!9 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 16, type: !5, scopeLine: 16, spFlags: DISPFlagDefinition, unit: !2)
!10 = !DILocation(line: 17, column: 13, scope: !9)
!11 = !DILocation(line: 18, column: 14, scope: !9)
