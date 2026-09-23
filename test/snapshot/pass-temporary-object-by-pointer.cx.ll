
%A = type { i32 }
%B = type { i32 }

define void @_EN4main1fEP1A(ptr %a) #0 !dbg !4 {
  %a1 = alloca ptr, align 8
  store ptr %a, ptr %a1, align 8
  ret void
}

define void @_EN4main1fEP1B(ptr %b) #0 !dbg !7 {
  %b1 = alloca ptr, align 8
  store ptr %b, ptr %b1, align 8
  ret void
}

define void @_EN4main1fEP3int(ptr %i) #0 !dbg !8 {
  %i1 = alloca ptr, align 8
  store ptr %i, ptr %i1, align 8
  ret void
}

define i32 @main() #0 !dbg !9 {
  %a = alloca %A, align 8
  %b = alloca %B, align 8
  %1 = alloca %A, align 8
  %2 = alloca %B, align 8
  %3 = alloca i32, align 4
  call void @_EN4main1A4initE(ptr %1), !dbg !10
  call void @_EN4main1fEP1A(ptr %1), !dbg !11
  call void @_EN4main1B4initE(ptr %2), !dbg !12
  call void @_EN4main1fEP1B(ptr %2), !dbg !13
  store i32 0, ptr %3, align 4
  call void @_EN4main1fEP3int(ptr %3), !dbg !14
  ret i32 0
}

define void @_EN4main1A4initE(ptr %this) #0 !dbg !15 {
  ret void
}

define void @_EN4main1B4initE(ptr %this) #0 !dbg !16 {
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "pass-temporary-object-by-pointer.cx")
!4 = distinct !DISubprogram(name: "f", linkageName: "_EN4main1fEP1A", scope: !3, file: !3, line: 13, type: !5, scopeLine: 13, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = distinct !DISubprogram(name: "f", linkageName: "_EN4main1fEP1B", scope: !3, file: !3, line: 14, type: !5, scopeLine: 14, spFlags: DISPFlagDefinition, unit: !2)
!8 = distinct !DISubprogram(name: "f", linkageName: "_EN4main1fEP3int", scope: !3, file: !3, line: 15, type: !5, scopeLine: 15, spFlags: DISPFlagDefinition, unit: !2)
!9 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 17, type: !5, scopeLine: 17, spFlags: DISPFlagDefinition, unit: !2)
!10 = !DILocation(line: 20, column: 8, scope: !9)
!11 = !DILocation(line: 20, column: 5, scope: !9)
!12 = !DILocation(line: 21, column: 8, scope: !9)
!13 = !DILocation(line: 21, column: 5, scope: !9)
!14 = !DILocation(line: 22, column: 5, scope: !9)
!15 = distinct !DISubprogram(name: "init", linkageName: "_EN4main1A4initE", scope: !3, file: !3, line: 5, type: !5, scopeLine: 5, spFlags: DISPFlagDefinition, unit: !2)
!16 = distinct !DISubprogram(name: "init", linkageName: "_EN4main1B4initE", scope: !3, file: !3, line: 10, type: !5, scopeLine: 10, spFlags: DISPFlagDefinition, unit: !2)
