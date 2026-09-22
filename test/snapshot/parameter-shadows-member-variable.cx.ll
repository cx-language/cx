
%S = type { i32 }

@0 = private unnamed_addr constant [63 x i8] c"integer overflow at parameter-shadows-member-variable.cx:7:17\0A\00", align 1

define i32 @main() #0 !dbg !4 {
  %1 = alloca %S, align 8
  call void @_EN4main1S4initE(ptr %1), !dbg !7
  call void @_EN4main1S3fooE3int(ptr %1, i32 30), !dbg !8
  ret i32 0
}

define void @_EN4main1S4initE(ptr %this) #0 !dbg !9 {
  ret void
}

define void @_EN4main1S3fooE3int(ptr %this, i32 %bar) #0 !dbg !10 {
  %bar1 = alloca i32, align 4
  store i32 %bar, ptr %bar1, align 4
  %bar.load = load i32, ptr %bar1, align 4
  %1 = sext i32 %bar.load to i64
  %2 = add i64 %1, 42
  %3 = trunc i64 %2 to i32
  %4 = sext i32 %3 to i64
  %5 = icmp ne i64 %2, %4
  %6 = xor i1 %5, true
  %overflow.condition = icmp eq i1 %6, false
  br i1 %overflow.condition, label %overflow.fail, label %overflow.success

overflow.fail:                                    ; preds = %0
  call void @_EN3std10assertFailEP4char(ptr @0), !dbg !11
  unreachable

overflow.success:                                 ; preds = %0
  ret void
}

declare void @_EN3std10assertFailEP4char(ptr) #0

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "parameter-shadows-member-variable.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 11, type: !5, scopeLine: 11, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 12, column: 5, scope: !4)
!8 = !DILocation(line: 12, column: 9, scope: !4)
!9 = distinct !DISubprogram(name: "init", linkageName: "_EN4main1S4initE", scope: !3, file: !3, line: 5, type: !5, scopeLine: 5, spFlags: DISPFlagDefinition, unit: !2)
!10 = distinct !DISubprogram(name: "foo", linkageName: "_EN4main1S3fooE3int", scope: !3, file: !3, line: 6, type: !5, scopeLine: 6, spFlags: DISPFlagDefinition, unit: !2)
!11 = !DILocation(line: 6, column: 10, scope: !10)
