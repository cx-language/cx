
@0 = private unnamed_addr constant [43 x i8] c"integer overflow at array-generic.cx:5:21\0A\00", align 1
@1 = private unnamed_addr constant [43 x i8] c"integer overflow at array-generic.cx:5:28\0A\00", align 1

define i32 @main() #0 !dbg !4 {
  %a = alloca [3 x i32], align 4
  store [3 x i32] [i32 10, i32 20, i32 30], ptr %a, align 4
  %1 = call i32 @_EN3std5ArrayI3intN3_E4sizeE(ptr %a), !dbg !7
  %2 = getelementptr inbounds [3 x i32], ptr %a, i32 0, i32 0
  %.load = load i32, ptr %2, align 4
  %3 = sext i32 %1 to i64
  %4 = sext i32 %.load to i64
  %5 = add i64 %3, %4
  %6 = trunc i64 %5 to i32
  %7 = sext i32 %6 to i64
  %8 = icmp ne i64 %5, %7
  %9 = xor i1 %8, true
  %overflow.condition = icmp eq i1 %9, false
  br i1 %overflow.condition, label %overflow.fail, label %overflow.success

overflow.fail:                                    ; preds = %0
  call void @_EN3std10assertFailEP4char(ptr @0), !dbg !8
  unreachable

overflow.success:                                 ; preds = %0
  %10 = getelementptr inbounds [3 x i32], ptr %a, i32 0, i32 2
  %.load1 = load i32, ptr %10, align 4
  %11 = sext i32 %6 to i64
  %12 = sext i32 %.load1 to i64
  %13 = add i64 %11, %12
  %14 = trunc i64 %13 to i32
  %15 = sext i32 %14 to i64
  %16 = icmp ne i64 %13, %15
  %17 = xor i1 %16, true
  %overflow.condition2 = icmp eq i1 %17, false
  br i1 %overflow.condition2, label %overflow.fail3, label %overflow.success4

overflow.fail3:                                   ; preds = %overflow.success
  call void @_EN3std10assertFailEP4char(ptr @1), !dbg !8
  unreachable

overflow.success4:                                ; preds = %overflow.success
  ret i32 %14
}

define i32 @_EN3std5ArrayI3intN3_E4sizeE(ptr %this) #0 !dbg !9 {
  ret i32 3
}

declare void @_EN3std10assertFailEP4char(ptr) #0

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "array-generic.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 5, column: 14, scope: !4)
!8 = !DILocation(line: 3, column: 5, scope: !4)
!9 = distinct !DISubprogram(name: "size", linkageName: "_EN3std5ArrayI3intN3_E4sizeE", scope: !10, file: !10, line: 9, type: !5, scopeLine: 9, spFlags: DISPFlagDefinition, unit: !2)
!10 = !DIFile(filename: "Array.cx")
