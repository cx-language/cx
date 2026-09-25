
@0 = private unnamed_addr constant [43 x i8] c"integer overflow at array-generic.cx:5:21\0A\00", align 1
@1 = private unnamed_addr constant [43 x i8] c"integer overflow at array-generic.cx:5:28\0A\00", align 1

define i32 @main() #0 !dbg !4 {
  %a = alloca [3 x i32], align 4
  store [3 x i32] [i32 10, i32 20, i32 30], ptr %a, align 4
  %1 = getelementptr inbounds [3 x i32], ptr %a, i32 0, i32 0
  %.load = load i32, ptr %1, align 4
  %2 = sext i32 %.load to i64
  %3 = add i64 3, %2
  %4 = trunc i64 %3 to i32
  %5 = sext i32 %4 to i64
  %6 = icmp ne i64 %3, %5
  %7 = xor i1 %6, true
  %overflow.condition = icmp eq i1 %7, false
  br i1 %overflow.condition, label %overflow.fail, label %overflow.success

overflow.fail:                                    ; preds = %0
  call void @_CX1N3std10assertFailE4void1_PKM3std4char(ptr @0), !dbg !7
  unreachable

overflow.success:                                 ; preds = %0
  %8 = getelementptr inbounds [3 x i32], ptr %a, i32 0, i32 2
  %.load1 = load i32, ptr %8, align 4
  %9 = sext i32 %4 to i64
  %10 = sext i32 %.load1 to i64
  %11 = add i64 %9, %10
  %12 = trunc i64 %11 to i32
  %13 = sext i32 %12 to i64
  %14 = icmp ne i64 %11, %13
  %15 = xor i1 %14, true
  %overflow.condition2 = icmp eq i1 %15, false
  br i1 %overflow.condition2, label %overflow.fail3, label %overflow.success4

overflow.fail3:                                   ; preds = %overflow.success
  call void @_CX1N3std10assertFailE4void1_PKM3std4char(ptr @1), !dbg !7
  unreachable

overflow.success4:                                ; preds = %overflow.success
  ret i32 %12
}

declare void @_CX1N3std10assertFailE4void1_PKM3std4char(ptr) #0

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
!7 = !DILocation(line: 3, column: 5, scope: !4)
