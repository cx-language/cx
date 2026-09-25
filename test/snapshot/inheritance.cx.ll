
%A = type { i32, i32, i1 }
%B = type { i32, i32 }
%O = type { i32, i32 }

define i32 @main() #0 !dbg !4 {
  %a = alloca %A, align 8
  %b = alloca %B, align 8
  call void @_CX1N4mainM4main1A4initE4void2_M3std4boolM3std5int32(ptr %a, i1 false, i32 0), !dbg !7
  call void @_CX1N4mainM4main1B4initE4void1_M3std5int32(ptr %b, i32 4), !dbg !8
  ret i32 0
}

define void @_CX1N4mainM4main1A4initE4void2_M3std4boolM3std5int32(ptr %this, i1 %b, i32 %p) #0 !dbg !9 {
  %b1 = alloca i1, align 1
  %p2 = alloca i32, align 4
  store i1 %b, ptr %b1, align 1
  store i32 %p, ptr %p2, align 4
  %p.load = load i32, ptr %p2, align 4
  call void @_CX1N4mainM4main1O4initE4void1_M3std5int32(ptr %this, i32 %p.load), !dbg !10
  %b3 = getelementptr inbounds %A, ptr %this, i32 0, i32 2
  %b.load = load i1, ptr %b1, align 1
  store i1 %b.load, ptr %b3, align 1
  ret void
}

define void @_CX1N4mainM4main1B4initE4void1_M3std5int32(ptr %this, i32 %p) #0 !dbg !11 {
  %p1 = alloca i32, align 4
  store i32 %p, ptr %p1, align 4
  %p2 = getelementptr inbounds %B, ptr %this, i32 0, i32 0
  %p.load = load i32, ptr %p1, align 4
  store i32 %p.load, ptr %p2, align 4
  %r = getelementptr inbounds %B, ptr %this, i32 0, i32 1
  %p.load3 = load i32, ptr %p1, align 4
  store i32 %p.load3, ptr %r, align 4
  ret void
}

define void @_CX1N4mainM4main1O4initE4void1_M3std5int32(ptr %this, i32 %p) #0 !dbg !12 {
  %p1 = alloca i32, align 4
  store i32 %p, ptr %p1, align 4
  %p2 = getelementptr inbounds %O, ptr %this, i32 0, i32 0
  %p.load = load i32, ptr %p1, align 4
  store i32 %p.load, ptr %p2, align 4
  %r = getelementptr inbounds %O, ptr %this, i32 0, i32 1
  %p.load3 = load i32, ptr %p1, align 4
  store i32 %p.load3, ptr %r, align 4
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "inheritance.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 49, type: !5, scopeLine: 49, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 50, column: 13, scope: !4)
!8 = !DILocation(line: 51, column: 13, scope: !4)
!9 = distinct !DISubprogram(name: "init", linkageName: "_CX1N4mainM4main1A4initE4void2_M3std4boolM3std5int32", scope: !3, file: !3, line: 19, type: !5, scopeLine: 19, spFlags: DISPFlagDefinition, unit: !2)
!10 = !DILocation(line: 20, column: 11, scope: !9)
!11 = distinct !DISubprogram(name: "init", linkageName: "_CX1N4mainM4main1B4initE4void1_M3std5int32", scope: !3, file: !3, line: 8, type: !5, scopeLine: 8, spFlags: DISPFlagDefinition, unit: !2)
!12 = distinct !DISubprogram(name: "init", linkageName: "_CX1N4mainM4main1O4initE4void1_M3std5int32", scope: !3, file: !3, line: 8, type: !5, scopeLine: 8, spFlags: DISPFlagDefinition, unit: !2)
