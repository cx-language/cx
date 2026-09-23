
%"ArrayRef<int>" = type { ptr, i32 }

define i32 @main() #0 !dbg !4 {
  %1 = alloca [3 x i32], align 4
  %2 = alloca [3 x i32], align 4
  store [3 x i32] [i32 1, i32 2, i32 3], ptr %1, align 4
  %3 = getelementptr inbounds [3 x i32], ptr %1, i32 0, i32 0
  %4 = insertvalue %"ArrayRef<int>" undef, ptr %3, 0
  %5 = insertvalue %"ArrayRef<int>" %4, i32 3, 1
  call void @_EN4main1fI3intEE8ArrayRefI3intE(%"ArrayRef<int>" %5), !dbg !7
  store [3 x i32] [i32 1, i32 2, i32 3], ptr %2, align 4
  %6 = getelementptr inbounds [3 x i32], ptr %2, i32 0, i32 0
  %7 = insertvalue %"ArrayRef<int>" undef, ptr %6, 0
  %8 = insertvalue %"ArrayRef<int>" %7, i32 3, 1
  call void @_EN4main1fI3intEE8ArrayRefI3intE(%"ArrayRef<int>" %8), !dbg !8
  ret i32 0
}

define void @_EN4main1fI3intEE8ArrayRefI3intE(%"ArrayRef<int>" %a) #0 !dbg !9 {
  %a1 = alloca %"ArrayRef<int>", align 8
  %s = alloca i32, align 4
  store %"ArrayRef<int>" %a, ptr %a1, align 8
  %1 = call i32 @_EN3std8ArrayRefI3intE4sizeE(ptr %a1), !dbg !10
  store i32 %1, ptr %s, align 4
  ret void
}

define i32 @_EN3std8ArrayRefI3intE4sizeE(ptr %this) #0 !dbg !11 {
  %size = getelementptr inbounds %"ArrayRef<int>", ptr %this, i32 0, i32 1
  %size.load = load i32, ptr %size, align 4
  ret i32 %size.load
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "temporary-unsized-array-argument.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 7, type: !5, scopeLine: 7, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 8, column: 5, scope: !4)
!8 = !DILocation(line: 9, column: 5, scope: !4)
!9 = distinct !DISubprogram(name: "f", linkageName: "_EN4main1fI3intEE8ArrayRefI3intE", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!10 = !DILocation(line: 4, column: 15, scope: !9)
!11 = distinct !DISubprogram(name: "size", linkageName: "_EN3std8ArrayRefI3intE4sizeE", scope: !12, file: !12, line: 31, type: !5, scopeLine: 31, spFlags: DISPFlagDefinition, unit: !2)
!12 = !DIFile(filename: "ArrayRef.cx")
