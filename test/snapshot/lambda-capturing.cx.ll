
%__closure0 = type { ptr, i32, i32 }
%never = type {}
%S = type { i32 }
%__closure2 = type { ptr, i32 }
%__closure1 = type { ptr, i32, ptr }

@0 = private unnamed_addr constant [46 x i8] c"integer overflow at lambda-capturing.cx:5:26\0A\00", align 1
@1 = private unnamed_addr constant [46 x i8] c"integer overflow at lambda-capturing.cx:5:30\0A\00", align 1
@2 = private unnamed_addr constant [47 x i8] c"integer overflow at lambda-capturing.cx:23:26\0A\00", align 1
@3 = private unnamed_addr constant [47 x i8] c"integer overflow at lambda-capturing.cx:13:30\0A\00", align 1
@4 = private unnamed_addr constant [47 x i8] c"integer overflow at lambda-capturing.cx:13:34\0A\00", align 1

define void @_CX1N4main13capturesParamE4void1_M3std5int32(i32 %p) #0 !dbg !4 {
  %p1 = alloca i32, align 4
  %d = alloca i32, align 4
  %b = alloca %__closure0, align 8
  store i32 %p, ptr %p1, align 4
  store i32 3, ptr %d, align 4
  %p.load = load i32, ptr %p1, align 4
  %1 = insertvalue %__closure0 { ptr @_CX1N4main11____lambda0EM3std5int321_M3std5int32C2_M3std5int32M3std5int32, i32 undef, i32 undef }, i32 %p.load, 1
  %d.load = load i32, ptr %d, align 4
  %2 = insertvalue %__closure0 %1, i32 %d.load, 2
  store %__closure0 %2, ptr %b, align 8
  %b.load = load %__closure0, ptr %b, align 8
  %3 = extractvalue %__closure0 %b.load, 0
  %4 = extractvalue %__closure0 %b.load, 1
  %5 = extractvalue %__closure0 %b.load, 2
  %6 = call i32 %3(i32 %4, i32 %5, i32 1), !dbg !7
  ret void
}

define i32 @_CX1N4main11____lambda0EM3std5int321_M3std5int32C2_M3std5int32M3std5int32(i32 %__capture_p, i32 %__capture_d, i32 %c) #0 !dbg !8 {
  %__capture_p1 = alloca i32, align 4
  %__capture_d2 = alloca i32, align 4
  %c3 = alloca i32, align 4
  store i32 %__capture_p, ptr %__capture_p1, align 4
  store i32 %__capture_d, ptr %__capture_d2, align 4
  store i32 %c, ptr %c3, align 4
  %c.load = load i32, ptr %c3, align 4
  %__capture_p.load = load i32, ptr %__capture_p1, align 4
  %1 = sext i32 %c.load to i64
  %2 = sext i32 %__capture_p.load to i64
  %3 = add i64 %1, %2
  %4 = trunc i64 %3 to i32
  %5 = sext i32 %4 to i64
  %6 = icmp ne i64 %3, %5
  %7 = xor i1 %6, true
  %overflow.condition = icmp eq i1 %7, false
  br i1 %overflow.condition, label %overflow.fail, label %overflow.success

overflow.fail:                                    ; preds = %0
  %8 = call %never @_CX1N3std10assertFailEM3std5never1_PKM3std4char(ptr @0), !dbg !9
  unreachable

overflow.success:                                 ; preds = %0
  %__capture_d.load = load i32, ptr %__capture_d2, align 4
  %9 = sext i32 %4 to i64
  %10 = sext i32 %__capture_d.load to i64
  %11 = add i64 %9, %10
  %12 = trunc i64 %11 to i32
  %13 = sext i32 %12 to i64
  %14 = icmp ne i64 %11, %13
  %15 = xor i1 %14, true
  %overflow.condition4 = icmp eq i1 %15, false
  br i1 %overflow.condition4, label %overflow.fail5, label %overflow.success6

overflow.fail5:                                   ; preds = %overflow.success
  %16 = call %never @_CX1N3std10assertFailEM3std5never1_PKM3std4char(ptr @1), !dbg !9
  unreachable

overflow.success6:                                ; preds = %overflow.success
  ret i32 %12
}

declare %never @_CX1N3std10assertFailEM3std5never1_PKM3std4char(ptr) #0

define i32 @main() #0 !dbg !10 {
  %s = alloca %S, align 8
  %a = alloca i32, align 4
  %b = alloca %__closure2, align 8
  call void @_CX1N4main13capturesParamE4void1_M3std5int32(i32 100), !dbg !11
  call void @_CX1N4mainM4main1S4initE4void1_M3std5int32(ptr %s, i32 5), !dbg !12
  %1 = call i32 @_CX1N4mainM4main1S3getEM3std5int321_M3std5int32(ptr %s, i32 1), !dbg !13
  store i32 1, ptr %a, align 4
  %a.load = load i32, ptr %a, align 4
  %2 = insertvalue %__closure2 { ptr @_CX1N4main11____lambda2EM3std5int321_M3std5int32C1_M3std5int32, i32 undef }, i32 %a.load, 1
  store %__closure2 %2, ptr %b, align 8
  %b.load = load %__closure2, ptr %b, align 8
  %3 = extractvalue %__closure2 %b.load, 0
  %4 = extractvalue %__closure2 %b.load, 1
  %5 = call i32 %3(i32 %4, i32 2), !dbg !14
  call void @_CX1N3std10checkLeaksE4void0_(), !dbg !15
  ret i32 0
}

define void @_CX1N4mainM4main1S4initE4void1_M3std5int32(ptr %this, i32 %d) #0 !dbg !16 {
  %d1 = alloca i32, align 4
  store i32 %d, ptr %d1, align 4
  %d2 = getelementptr inbounds %S, ptr %this, i32 0, i32 0
  %d.load = load i32, ptr %d1, align 4
  store i32 %d.load, ptr %d2, align 4
  ret void
}

define i32 @_CX1N4mainM4main1S3getEM3std5int321_M3std5int32(ptr %this, i32 %c) #0 !dbg !17 {
  %c1 = alloca i32, align 4
  %b = alloca %__closure1, align 8
  store i32 %c, ptr %c1, align 4
  %c.load = load i32, ptr %c1, align 4
  %insert.alloca = alloca %__closure1, align 8
  store %__closure1 { ptr @_CX1N4main11____lambda1EM3std5int321_M3std5int32C2_M3std5int32PM4main1S, i32 undef, ptr undef }, ptr %insert.alloca, align 8
  %insert.gep = getelementptr inbounds %__closure1, ptr %insert.alloca, i32 0, i32 1
  store i32 %c.load, ptr %insert.gep, align 4
  %insert.alloca2 = alloca %__closure1, align 8
  call void @llvm.memcpy.p0.p0.i64(ptr align 8 %insert.alloca2, ptr align 8 %insert.alloca, i64 24, i1 false)
  %insert.gep3 = getelementptr inbounds %__closure1, ptr %insert.alloca2, i32 0, i32 2
  store ptr %this, ptr %insert.gep3, align 8
  call void @llvm.memcpy.p0.p0.i64(ptr align 8 %b, ptr align 8 %insert.alloca2, i64 24, i1 false)
  %b.load = alloca %__closure1, align 8
  call void @llvm.memcpy.p0.p0.i64(ptr align 8 %b.load, ptr align 8 %b, i64 24, i1 false)
  %1 = getelementptr inbounds %__closure1, ptr %b.load, i32 0, i32 0
  %2 = load ptr, ptr %1, align 8
  %3 = getelementptr inbounds %__closure1, ptr %b.load, i32 0, i32 1
  %4 = load i32, ptr %3, align 4
  %5 = getelementptr inbounds %__closure1, ptr %b.load, i32 0, i32 2
  %6 = load ptr, ptr %5, align 8
  %7 = call i32 %2(i32 %4, ptr %6, i32 1), !dbg !18
  ret i32 %7
}

define i32 @_CX1N4main11____lambda2EM3std5int321_M3std5int32C1_M3std5int32(i32 %__capture_a, i32 %c) #0 !dbg !19 {
  %__capture_a1 = alloca i32, align 4
  %c2 = alloca i32, align 4
  store i32 %__capture_a, ptr %__capture_a1, align 4
  store i32 %c, ptr %c2, align 4
  %c.load = load i32, ptr %c2, align 4
  %__capture_a.load = load i32, ptr %__capture_a1, align 4
  %1 = sext i32 %c.load to i64
  %2 = sext i32 %__capture_a.load to i64
  %3 = add i64 %1, %2
  %4 = trunc i64 %3 to i32
  %5 = sext i32 %4 to i64
  %6 = icmp ne i64 %3, %5
  %7 = xor i1 %6, true
  %overflow.condition = icmp eq i1 %7, false
  br i1 %overflow.condition, label %overflow.fail, label %overflow.success

overflow.fail:                                    ; preds = %0
  %8 = call %never @_CX1N3std10assertFailEM3std5never1_PKM3std4char(ptr @2), !dbg !20
  unreachable

overflow.success:                                 ; preds = %0
  ret i32 %4
}

declare void @_CX1N3std10checkLeaksE4void0_() #0

define i32 @_CX1N4main11____lambda1EM3std5int321_M3std5int32C2_M3std5int32PM4main1S(i32 %__capture_c, ptr %__capture_this, i32 %x) #0 !dbg !21 {
  %__capture_c1 = alloca i32, align 4
  %x2 = alloca i32, align 4
  store i32 %__capture_c, ptr %__capture_c1, align 4
  store i32 %x, ptr %x2, align 4
  %x.load = load i32, ptr %x2, align 4
  %__capture_c.load = load i32, ptr %__capture_c1, align 4
  %1 = sext i32 %x.load to i64
  %2 = sext i32 %__capture_c.load to i64
  %3 = add i64 %1, %2
  %4 = trunc i64 %3 to i32
  %5 = sext i32 %4 to i64
  %6 = icmp ne i64 %3, %5
  %7 = xor i1 %6, true
  %overflow.condition = icmp eq i1 %7, false
  br i1 %overflow.condition, label %overflow.fail, label %overflow.success

overflow.fail:                                    ; preds = %0
  %8 = call %never @_CX1N3std10assertFailEM3std5never1_PKM3std4char(ptr @3), !dbg !22
  unreachable

overflow.success:                                 ; preds = %0
  %d = getelementptr inbounds %S, ptr %__capture_this, i32 0, i32 0
  %d.load = load i32, ptr %d, align 4
  %9 = sext i32 %4 to i64
  %10 = sext i32 %d.load to i64
  %11 = add i64 %9, %10
  %12 = trunc i64 %11 to i32
  %13 = sext i32 %12 to i64
  %14 = icmp ne i64 %11, %13
  %15 = xor i1 %14, true
  %overflow.condition3 = icmp eq i1 %15, false
  br i1 %overflow.condition3, label %overflow.fail4, label %overflow.success5

overflow.fail4:                                   ; preds = %overflow.success
  %16 = call %never @_CX1N3std10assertFailEM3std5never1_PKM3std4char(ptr @4), !dbg !22
  unreachable

overflow.success5:                                ; preds = %overflow.success
  ret i32 %12
}

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias writeonly captures(none), ptr noalias readonly captures(none), i64, i1 immarg) #1

attributes #0 = { "frame-pointer"="all" }
attributes #1 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"DEBUG-FORMAT", i32 0}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "lambda-capturing.cx")
!4 = distinct !DISubprogram(name: "capturesParam", linkageName: "_CX1N4main13capturesParamE4void1_M3std5int32", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 6, column: 9, scope: !4)
!8 = distinct !DISubprogram(name: "__lambda0", linkageName: "_CX1N4main11____lambda0EM3std5int321_M3std5int32C2_M3std5int32M3std5int32", scope: !3, file: !3, line: 5, type: !5, scopeLine: 5, spFlags: DISPFlagDefinition, unit: !2)
!9 = !DILocation(line: 5, column: 13, scope: !8)
!10 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 18, type: !5, scopeLine: 18, spFlags: DISPFlagDefinition, unit: !2)
!11 = !DILocation(line: 19, column: 5, scope: !10)
!12 = !DILocation(line: 20, column: 13, scope: !10)
!13 = !DILocation(line: 21, column: 11, scope: !10)
!14 = !DILocation(line: 24, column: 9, scope: !10)
!15 = !DILocation(line: 18, column: 6, scope: !10)
!16 = distinct !DISubprogram(name: "init", linkageName: "_CX1N4mainM4main1S4initE4void1_M3std5int32", scope: !3, file: !3, line: 9, type: !5, scopeLine: 9, spFlags: DISPFlagDefinition, unit: !2)
!17 = distinct !DISubprogram(name: "get", linkageName: "_CX1N4mainM4main1S3getEM3std5int321_M3std5int32", scope: !3, file: !3, line: 12, type: !5, scopeLine: 12, spFlags: DISPFlagDefinition, unit: !2)
!18 = !DILocation(line: 14, column: 16, scope: !17)
!19 = distinct !DISubprogram(name: "__lambda2", linkageName: "_CX1N4main11____lambda2EM3std5int321_M3std5int32C1_M3std5int32", scope: !3, file: !3, line: 23, type: !5, scopeLine: 23, spFlags: DISPFlagDefinition, unit: !2)
!20 = !DILocation(line: 23, column: 13, scope: !19)
!21 = distinct !DISubprogram(name: "__lambda1", linkageName: "_CX1N4main11____lambda1EM3std5int321_M3std5int32C2_M3std5int32PM4main1S", scope: !3, file: !3, line: 13, type: !5, scopeLine: 13, spFlags: DISPFlagDefinition, unit: !2)
!22 = !DILocation(line: 13, column: 17, scope: !21)
