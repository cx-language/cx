
@0 = private unnamed_addr constant [59 x i8] c"integer overflow at dereference-vs-multiplication.cx:11:5\0A\00", align 1

define i32 @_CX1N4main1gEM3std5int320_() #0 !dbg !4 {
  ret i32 1
}

define void @_CX1N4main1fE4void2_PM3std5int32M3std5int32(ptr %a, i32 %b) #0 !dbg !7 {
  %a1 = alloca ptr, align 8
  %b2 = alloca i32, align 4
  %m = alloca i32, align 4
  %n = alloca i32, align 4
  store ptr %a, ptr %a1, align 8
  store i32 %b, ptr %b2, align 4
  %1 = call i32 @_CX1N4main1gEM3std5int320_(), !dbg !8
  store i32 %1, ptr %m, align 4
  %a.load = load ptr, ptr %a1, align 8
  %b.load = load i32, ptr %b2, align 4
  store i32 %b.load, ptr %a.load, align 4
  %2 = call i32 @_CX1N4main1gEM3std5int320_(), !dbg !9
  %b.load3 = load i32, ptr %b2, align 4
  %3 = sext i32 %2 to i64
  %4 = sext i32 %b.load3 to i64
  %5 = mul i64 %3, %4
  %6 = trunc i64 %5 to i32
  %7 = sext i32 %6 to i64
  %8 = icmp ne i64 %5, %7
  %9 = xor i1 %8, true
  %overflow.condition = icmp eq i1 %9, false
  br i1 %overflow.condition, label %overflow.fail, label %overflow.success

overflow.fail:                                    ; preds = %0
  call void @_CX1N3std10assertFailE4void1_PKM3std4char(ptr @0), !dbg !10
  unreachable

overflow.success:                                 ; preds = %0
  store i32 %6, ptr %n, align 4
  ret void
}

declare void @_CX1N3std10assertFailE4void1_PKM3std4char(ptr) #0

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "dereference-vs-multiplication.cx")
!4 = distinct !DISubprogram(name: "g", linkageName: "_CX1N4main1gEM3std5int320_", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = distinct !DISubprogram(name: "f", linkageName: "_CX1N4main1fE4void2_PM3std5int32M3std5int32", scope: !3, file: !3, line: 7, type: !5, scopeLine: 7, spFlags: DISPFlagDefinition, unit: !2)
!8 = !DILocation(line: 8, column: 13, scope: !7)
!9 = !DILocation(line: 10, column: 13, scope: !7)
!10 = !DILocation(line: 7, column: 6, scope: !7)
