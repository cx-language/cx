
define i32 @main() #0 !dbg !4 {
  %p = alloca ptr, align 8
  %cp = alloca ptr, align 8
  %v = alloca ptr, align 8
  %b = alloca ptr, align 8
  %i = alloca i32, align 4
  %p.load = load ptr, ptr %p, align 8
  store ptr %p.load, ptr %v, align 8
  %p.load1 = load ptr, ptr %p, align 8
  call void @_EN4main3barEP4void(ptr %p.load1), !dbg !7
  %v.load = load ptr, ptr %v, align 8
  call void @_EN4main3barEP4void(ptr %v.load), !dbg !8
  %v.load2 = load ptr, ptr %v, align 8
  store ptr %v.load2, ptr %b, align 8
  %b.load = load ptr, ptr %b, align 8
  store ptr %b.load, ptr %v, align 8
  %cp.load = load ptr, ptr %cp, align 8
  call void @_EN4main3bazEOP4void(ptr %cp.load), !dbg !9
  store i32 0, ptr %i, align 4
  call void @_EN4main3bazEOP4void(ptr %i), !dbg !10
  ret i32 0
}

define void @_EN4main3barEP4void(ptr %p) #0 !dbg !11 {
  %p1 = alloca ptr, align 8
  store ptr %p, ptr %p1, align 8
  ret void
}

define void @_EN4main3bazEOP4void(ptr %p) #0 !dbg !12 {
  %p1 = alloca ptr, align 8
  store ptr %p, ptr %p1, align 8
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "implicit-cast-to-void-pointer.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 7, column: 5, scope: !4)
!8 = !DILocation(line: 8, column: 5, scope: !4)
!9 = !DILocation(line: 11, column: 5, scope: !4)
!10 = !DILocation(line: 13, column: 5, scope: !4)
!11 = distinct !DISubprogram(name: "bar", linkageName: "_EN4main3barEP4void", scope: !3, file: !3, line: 16, type: !5, scopeLine: 16, spFlags: DISPFlagDefinition, unit: !2)
!12 = distinct !DISubprogram(name: "baz", linkageName: "_EN4main3bazEOP4void", scope: !3, file: !3, line: 18, type: !5, scopeLine: 18, spFlags: DISPFlagDefinition, unit: !2)
