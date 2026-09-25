
%never = type {}

@0 = private unnamed_addr constant [57 x i8] c"integer overflow at function-pointer-parameter.cx:12:26\0A\00", align 1

define void @_CX1N4main1fE4void0_() #0 !dbg !4 {
  ret void
}

define i32 @_CX1N4main2f2EM3std5int322_M3std5int32M3std4bool(i32 %a, i1 %b) #0 !dbg !7 {
  %a1 = alloca i32, align 4
  %b2 = alloca i1, align 1
  store i32 %a, ptr %a1, align 4
  store i1 %b, ptr %b2, align 1
  %a.load = load i32, ptr %a1, align 4
  ret i32 %a.load
}

define void @_CX1N4main1gE4void1_F0_4void(ptr %p) #0 !dbg !8 {
  %p1 = alloca ptr, align 8
  store ptr %p, ptr %p1, align 8
  %p.load = load ptr, ptr %p1, align 8
  call void %p.load(), !dbg !9
  ret void
}

define void @_CX1N4main2g2E4void1_F2_M3std5int32M3std4boolM3std5int32(ptr %p) #0 !dbg !10 {
  %p1 = alloca ptr, align 8
  %a = alloca i32, align 4
  store ptr %p, ptr %p1, align 8
  %p.load = load ptr, ptr %p1, align 8
  %1 = call i32 %p.load(i32 42, i1 false), !dbg !11
  %2 = sext i32 %1 to i64
  %3 = add i64 %2, 1
  %4 = trunc i64 %3 to i32
  %5 = sext i32 %4 to i64
  %6 = icmp ne i64 %3, %5
  %7 = xor i1 %6, true
  %overflow.condition = icmp eq i1 %7, false
  br i1 %overflow.condition, label %overflow.fail, label %overflow.success

overflow.fail:                                    ; preds = %0
  %8 = call %never @_CX1N3std10assertFailEM3std5never1_PKM3std4char(ptr @0), !dbg !12
  unreachable

overflow.success:                                 ; preds = %0
  store i32 %4, ptr %a, align 4
  ret void
}

declare %never @_CX1N3std10assertFailEM3std5never1_PKM3std4char(ptr) #0

define i32 @main() #0 !dbg !13 {
  call void @_CX1N4main1gE4void1_F0_4void(ptr @_CX1N4main1fE4void0_), !dbg !14
  call void @_CX1N4main2g2E4void1_F2_M3std5int32M3std4boolM3std5int32(ptr @_CX1N4main2f2EM3std5int322_M3std5int32M3std4bool), !dbg !15
  ret i32 0
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "function-pointer-parameter.cx")
!4 = distinct !DISubprogram(name: "f", linkageName: "_CX1N4main1fE4void0_", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = distinct !DISubprogram(name: "f2", linkageName: "_CX1N4main2f2EM3std5int322_M3std5int32M3std4bool", scope: !3, file: !3, line: 5, type: !5, scopeLine: 5, spFlags: DISPFlagDefinition, unit: !2)
!8 = distinct !DISubprogram(name: "g", linkageName: "_CX1N4main1gE4void1_F0_4void", scope: !3, file: !3, line: 7, type: !5, scopeLine: 7, spFlags: DISPFlagDefinition, unit: !2)
!9 = !DILocation(line: 8, column: 5, scope: !8)
!10 = distinct !DISubprogram(name: "g2", linkageName: "_CX1N4main2g2E4void1_F2_M3std5int32M3std4boolM3std5int32", scope: !3, file: !3, line: 11, type: !5, scopeLine: 11, spFlags: DISPFlagDefinition, unit: !2)
!11 = !DILocation(line: 12, column: 13, scope: !10)
!12 = !DILocation(line: 11, column: 6, scope: !10)
!13 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 15, type: !5, scopeLine: 15, spFlags: DISPFlagDefinition, unit: !2)
!14 = !DILocation(line: 16, column: 5, scope: !13)
!15 = !DILocation(line: 17, column: 5, scope: !13)
