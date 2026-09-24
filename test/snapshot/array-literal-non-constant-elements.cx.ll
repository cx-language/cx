
@0 = private unnamed_addr constant [65 x i8] c"integer overflow at array-literal-non-constant-elements.cx:4:17\0A\00", align 1
@1 = private unnamed_addr constant [65 x i8] c"integer overflow at array-literal-non-constant-elements.cx:4:28\0A\00", align 1
@2 = private unnamed_addr constant [65 x i8] c"integer overflow at array-literal-non-constant-elements.cx:8:18\0A\00", align 1
@3 = private unnamed_addr constant [65 x i8] c"integer overflow at array-literal-non-constant-elements.cx:8:29\0A\00", align 1

define [2 x i32] @_EN4main1fE5int325int32(i32 %foo, i32 %bar) #0 !dbg !4 {
  %foo1 = alloca i32, align 4
  %bar2 = alloca i32, align 4
  store i32 %foo, ptr %foo1, align 4
  store i32 %bar, ptr %bar2, align 4
  %foo.load = load i32, ptr %foo1, align 4
  %bar.load = load i32, ptr %bar2, align 4
  %1 = sext i32 %foo.load to i64
  %2 = sext i32 %bar.load to i64
  %3 = add i64 %1, %2
  %4 = trunc i64 %3 to i32
  %5 = sext i32 %4 to i64
  %6 = icmp ne i64 %3, %5
  %7 = xor i1 %6, true
  %overflow.condition = icmp eq i1 %7, false
  br i1 %overflow.condition, label %overflow.fail, label %overflow.success

overflow.fail:                                    ; preds = %0
  call void @_EN3std10assertFailEP4char(ptr @0), !dbg !7
  unreachable

overflow.success:                                 ; preds = %0
  %8 = insertvalue [2 x i32] undef, i32 %4, 0
  %foo.load3 = load i32, ptr %foo1, align 4
  %bar.load4 = load i32, ptr %bar2, align 4
  %9 = sext i32 %foo.load3 to i64
  %10 = sext i32 %bar.load4 to i64
  %11 = sub i64 %9, %10
  %12 = trunc i64 %11 to i32
  %13 = sext i32 %12 to i64
  %14 = icmp ne i64 %11, %13
  %15 = xor i1 %14, true
  %overflow.condition5 = icmp eq i1 %15, false
  br i1 %overflow.condition5, label %overflow.fail6, label %overflow.success7

overflow.fail6:                                   ; preds = %overflow.success
  call void @_EN3std10assertFailEP4char(ptr @1), !dbg !7
  unreachable

overflow.success7:                                ; preds = %overflow.success
  %16 = insertvalue [2 x i32] %8, i32 %12, 1
  ret [2 x i32] %16
}

declare void @_EN3std10assertFailEP4char(ptr) #0

define [2 x i32] @_EN4main1gE5int325int32(i32 %foo, i32 %bar) #0 !dbg !8 {
  %foo1 = alloca i32, align 4
  %bar2 = alloca i32, align 4
  %c = alloca [2 x i32], align 4
  store i32 %foo, ptr %foo1, align 4
  store i32 %bar, ptr %bar2, align 4
  %foo.load = load i32, ptr %foo1, align 4
  %bar.load = load i32, ptr %bar2, align 4
  %1 = sext i32 %foo.load to i64
  %2 = sext i32 %bar.load to i64
  %3 = add i64 %1, %2
  %4 = trunc i64 %3 to i32
  %5 = sext i32 %4 to i64
  %6 = icmp ne i64 %3, %5
  %7 = xor i1 %6, true
  %overflow.condition = icmp eq i1 %7, false
  br i1 %overflow.condition, label %overflow.fail, label %overflow.success

overflow.fail:                                    ; preds = %0
  call void @_EN3std10assertFailEP4char(ptr @2), !dbg !9
  unreachable

overflow.success:                                 ; preds = %0
  %8 = insertvalue [2 x i32] undef, i32 %4, 0
  %foo.load3 = load i32, ptr %foo1, align 4
  %bar.load4 = load i32, ptr %bar2, align 4
  %9 = sext i32 %foo.load3 to i64
  %10 = sext i32 %bar.load4 to i64
  %11 = sub i64 %9, %10
  %12 = trunc i64 %11 to i32
  %13 = sext i32 %12 to i64
  %14 = icmp ne i64 %11, %13
  %15 = xor i1 %14, true
  %overflow.condition5 = icmp eq i1 %15, false
  br i1 %overflow.condition5, label %overflow.fail6, label %overflow.success7

overflow.fail6:                                   ; preds = %overflow.success
  call void @_EN3std10assertFailEP4char(ptr @3), !dbg !9
  unreachable

overflow.success7:                                ; preds = %overflow.success
  %16 = insertvalue [2 x i32] %8, i32 %12, 1
  store [2 x i32] %16, ptr %c, align 4
  %c.load = load [2 x i32], ptr %c, align 4
  ret [2 x i32] %c.load
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "array-literal-non-constant-elements.cx")
!4 = distinct !DISubprogram(name: "f", linkageName: "_EN4main1fE5int325int32", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 3, column: 8, scope: !4)
!8 = distinct !DISubprogram(name: "g", linkageName: "_EN4main1gE5int325int32", scope: !3, file: !3, line: 7, type: !5, scopeLine: 7, spFlags: DISPFlagDefinition, unit: !2)
!9 = !DILocation(line: 7, column: 8, scope: !8)
