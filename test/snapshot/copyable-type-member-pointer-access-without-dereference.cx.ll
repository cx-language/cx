
%S = type { ptr }

define i32 @main() #0 !dbg !4 {
  %1 = alloca %S, align 8
  %2 = alloca i32, align 4
  store i32 1, ptr %2, align 4
  call void @_EN4main1S4initEP3int(ptr %1, ptr %2), !dbg !7
  call void @_EN4main1S3fooE(ptr %1), !dbg !8
  ret i32 0
}

define void @_EN4main1S4initEP3int(ptr %this, ptr %a) #0 !dbg !9 {
  %a1 = alloca ptr, align 8
  store ptr %a, ptr %a1, align 8
  %a2 = getelementptr inbounds %S, ptr %this, i32 0, i32 0
  %a.load = load ptr, ptr %a1, align 8
  store ptr %a.load, ptr %a2, align 8
  ret void
}

define void @_EN4main1S3fooE(ptr %this) #0 !dbg !10 {
  %b = alloca ptr, align 8
  %a = getelementptr inbounds %S, ptr %this, i32 0, i32 0
  %a.load = load ptr, ptr %a, align 8
  store ptr %a.load, ptr %b, align 8
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "copyable-type-member-pointer-access-without-dereference.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 11, type: !5, scopeLine: 11, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 12, column: 5, scope: !4)
!8 = !DILocation(line: 12, column: 10, scope: !4)
!9 = distinct !DISubprogram(name: "init", linkageName: "_EN4main1S4initEP3int", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!10 = distinct !DISubprogram(name: "foo", linkageName: "_EN4main1S3fooE", scope: !3, file: !3, line: 6, type: !5, scopeLine: 6, spFlags: DISPFlagDefinition, unit: !2)
