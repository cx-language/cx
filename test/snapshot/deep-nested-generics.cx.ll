
%"S<R>" = type { %"A<A<R>>" }
%"A<A<R>>" = type { %"A<R>" }
%"A<R>" = type { %R }
%R = type { i32 }

define i32 @main() #0 !dbg !4 {
  %s = alloca %"S<R>", align 8
  call void @_EN4main1SI1RE4initE(ptr %s), !dbg !7
  call void @_EN4main1SI1RE1sE(ptr %s), !dbg !8
  ret i32 0
}

define void @_EN4main1SI1RE4initE(ptr %this) #0 !dbg !9 {
  ret void
}

define void @_EN4main1SI1RE1sE(ptr %this) #0 !dbg !10 {
  %t = alloca %"A<R>", align 8
  %tt = alloca %R, align 8
  %a = getelementptr inbounds %"S<R>", ptr %this, i32 0, i32 0
  %1 = call %"A<R>" @_EN4main1AI1AI1REEixE5int32(ptr %a, i32 0), !dbg !11
  store %"A<R>" %1, ptr %t, align 4
  %2 = call %R @_EN4main1AI1REixE5int32(ptr %t, i32 0), !dbg !12
  store %R %2, ptr %tt, align 4
  %a1 = getelementptr inbounds %"S<R>", ptr %this, i32 0, i32 0
  %3 = call i32 @_EN4main1R1hE(ptr %tt), !dbg !13
  %4 = call %"A<R>" @_EN4main1AI1AI1REEixE5int32(ptr %a1, i32 %3), !dbg !14
  ret void
}

define %"A<R>" @_EN4main1AI1AI1REEixE5int32(ptr %this, i32 %i) #0 !dbg !15 {
  %i1 = alloca i32, align 4
  store i32 %i, ptr %i1, align 4
  %t = getelementptr inbounds %"A<A<R>>", ptr %this, i32 0, i32 0
  %t.load = load %"A<R>", ptr %t, align 4
  ret %"A<R>" %t.load
}

define %R @_EN4main1AI1REixE5int32(ptr %this, i32 %i) #0 !dbg !16 {
  %i1 = alloca i32, align 4
  store i32 %i, ptr %i1, align 4
  %t = getelementptr inbounds %"A<R>", ptr %this, i32 0, i32 0
  %t.load = load %R, ptr %t, align 4
  ret %R %t.load
}

define i32 @_EN4main1R1hE(ptr %this) #0 !dbg !17 {
  %i = getelementptr inbounds %R, ptr %this, i32 0, i32 0
  %i.load = load i32, ptr %i, align 4
  ret i32 %i.load
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "deep-nested-generics.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 41, type: !5, scopeLine: 41, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 42, column: 13, scope: !4)
!8 = !DILocation(line: 43, column: 7, scope: !4)
!9 = distinct !DISubprogram(name: "init", linkageName: "_EN4main1SI1RE4initE", scope: !3, file: !3, line: 30, type: !5, scopeLine: 30, spFlags: DISPFlagDefinition, unit: !2)
!10 = distinct !DISubprogram(name: "s", linkageName: "_EN4main1SI1RE1sE", scope: !3, file: !3, line: 34, type: !5, scopeLine: 34, spFlags: DISPFlagDefinition, unit: !2)
!11 = !DILocation(line: 35, column: 18, scope: !10)
!12 = !DILocation(line: 36, column: 19, scope: !10)
!13 = !DILocation(line: 37, column: 18, scope: !10)
!14 = !DILocation(line: 37, column: 14, scope: !10)
!15 = distinct !DISubprogram(name: "[]", linkageName: "_EN4main1AI1AI1REEixE5int32", scope: !3, file: !3, line: 22, type: !5, scopeLine: 22, spFlags: DISPFlagDefinition, unit: !2)
!16 = distinct !DISubprogram(name: "[]", linkageName: "_EN4main1AI1REixE5int32", scope: !3, file: !3, line: 22, type: !5, scopeLine: 22, spFlags: DISPFlagDefinition, unit: !2)
!17 = distinct !DISubprogram(name: "h", linkageName: "_EN4main1R1hE", scope: !3, file: !3, line: 10, type: !5, scopeLine: 10, spFlags: DISPFlagDefinition, unit: !2)
