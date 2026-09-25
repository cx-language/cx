
%never = type {}

@0 = private unnamed_addr constant [48 x i8] c"integer overflow at compound-assignment.cx:5:7\0A\00", align 1
@1 = private unnamed_addr constant [48 x i8] c"integer overflow at compound-assignment.cx:6:7\0A\00", align 1
@2 = private unnamed_addr constant [48 x i8] c"integer overflow at compound-assignment.cx:7:7\0A\00", align 1

define i32 @main() #0 !dbg !4 {
  %i = alloca i32, align 4
  store i32 0, ptr %i, align 4
  %i.load = load i32, ptr %i, align 4
  %1 = sext i32 %i.load to i64
  %2 = add i64 %1, 1
  %3 = trunc i64 %2 to i32
  %4 = sext i32 %3 to i64
  %5 = icmp ne i64 %2, %4
  %6 = xor i1 %5, true
  %overflow.condition = icmp eq i1 %6, false
  br i1 %overflow.condition, label %overflow.fail, label %overflow.success

overflow.fail:                                    ; preds = %0
  %7 = call %never @_CX1N3std10assertFailEM3std5never1_PKM3std4char(ptr @0), !dbg !7
  unreachable

overflow.success:                                 ; preds = %0
  store i32 %3, ptr %i, align 4
  %i.load1 = load i32, ptr %i, align 4
  %8 = sext i32 %i.load1 to i64
  %9 = sub i64 %8, 1
  %10 = trunc i64 %9 to i32
  %11 = sext i32 %10 to i64
  %12 = icmp ne i64 %9, %11
  %13 = xor i1 %12, true
  %overflow.condition2 = icmp eq i1 %13, false
  br i1 %overflow.condition2, label %overflow.fail3, label %overflow.success4

overflow.fail3:                                   ; preds = %overflow.success
  %14 = call %never @_CX1N3std10assertFailEM3std5never1_PKM3std4char(ptr @1), !dbg !7
  unreachable

overflow.success4:                                ; preds = %overflow.success
  store i32 %10, ptr %i, align 4
  %i.load5 = load i32, ptr %i, align 4
  %15 = sext i32 %i.load5 to i64
  %16 = mul i64 %15, 1
  %17 = trunc i64 %16 to i32
  %18 = sext i32 %17 to i64
  %19 = icmp ne i64 %16, %18
  %20 = xor i1 %19, true
  %overflow.condition6 = icmp eq i1 %20, false
  br i1 %overflow.condition6, label %overflow.fail7, label %overflow.success8

overflow.fail7:                                   ; preds = %overflow.success4
  %21 = call %never @_CX1N3std10assertFailEM3std5never1_PKM3std4char(ptr @2), !dbg !7
  unreachable

overflow.success8:                                ; preds = %overflow.success4
  store i32 %17, ptr %i, align 4
  %i.load9 = load i32, ptr %i, align 4
  %22 = sdiv i32 %i.load9, 1
  store i32 %22, ptr %i, align 4
  %i.load10 = load i32, ptr %i, align 4
  %23 = and i32 %i.load10, 1
  store i32 %23, ptr %i, align 4
  %i.load11 = load i32, ptr %i, align 4
  %24 = or i32 %i.load11, 1
  store i32 %24, ptr %i, align 4
  %i.load12 = load i32, ptr %i, align 4
  %25 = xor i32 %i.load12, 1
  store i32 %25, ptr %i, align 4
  %i.load13 = load i32, ptr %i, align 4
  %26 = shl i32 %i.load13, 1
  store i32 %26, ptr %i, align 4
  %i.load14 = load i32, ptr %i, align 4
  %27 = ashr i32 %i.load14, 1
  store i32 %27, ptr %i, align 4
  call void @_CX1N3std10checkLeaksE4void0_(), !dbg !7
  ret i32 0
}

declare %never @_CX1N3std10assertFailEM3std5never1_PKM3std4char(ptr) #0

declare void @_CX1N3std10checkLeaksE4void0_() #0

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "compound-assignment.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 3, column: 6, scope: !4)
