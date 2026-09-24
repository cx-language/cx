
%"M<int32>" = type {}
%"Slice<int32>" = type { ptr, i32 }

define i32 @main() #0 !dbg !4 {
  %b = alloca [3 x i32], align 4
  %1 = alloca %"M<int32>", align 8
  store [3 x i32] [i32 1, i32 2, i32 3], ptr %b, align 4
  %2 = getelementptr inbounds [3 x i32], ptr %b, i32 0, i32 0
  %3 = insertvalue %"Slice<int32>" undef, ptr %2, 0
  %4 = insertvalue %"Slice<int32>" %3, i32 3, 1
  call void @_EN4main1MI5int32E4initE5SliceI5int32E(ptr %1, %"Slice<int32>" %4), !dbg !7
  ret i32 0
}

define void @_EN4main1MI5int32E4initE5SliceI5int32E(ptr %this, %"Slice<int32>" %a) #0 !dbg !8 {
  %a1 = alloca %"Slice<int32>", align 8
  store %"Slice<int32>" %a, ptr %a1, align 8
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "generic-type-generic-param-in-constructor-params.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 7, type: !5, scopeLine: 7, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 9, column: 9, scope: !4)
!8 = distinct !DISubprogram(name: "init", linkageName: "_EN4main1MI5int32E4initE5SliceI5int32E", scope: !3, file: !3, line: 4, type: !5, scopeLine: 4, spFlags: DISPFlagDefinition, unit: !2)
