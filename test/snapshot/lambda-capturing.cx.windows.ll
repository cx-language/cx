
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

define void @_CX1N4main13capturesParamE4void1_M3std5int32(ptr %__context, i32 %p) #0 !dbg !4 {
  %p1 = alloca i32, align 4
  %d = alloca i32, align 4
  %b = alloca %__closure0, align 8
  store i32 %p, ptr %p1, align 4
  store i32 3, ptr %d, align 4
  %p.load = load i32, ptr %p1, align 4
  %insert.alloca = alloca %__closure0, align 8
  store %__closure0 { ptr @_CX1N4main11____lambda0EM3std5int321_M3std5int32C2_M3std5int32M3std5int32, i32 undef, i32 undef }, ptr %insert.alloca, align 8
  %insert.gep = getelementptr inbounds %__closure0, ptr %insert.alloca, i32 0, i32 1
  store i32 %p.load, ptr %insert.gep, align 4
  %d.load = load i32, ptr %d, align 4
  %insert.alloca2 = alloca %__closure0, align 8
  call void @llvm.memcpy.p0.p0.i64(ptr align 8 %insert.alloca2, ptr align 8 %insert.alloca, i64 16, i1 false)
  %insert.gep3 = getelementptr inbounds %__closure0, ptr %insert.alloca2, i32 0, i32 2
  store i32 %d.load, ptr %insert.gep3, align 4
  call void @llvm.memcpy.p0.p0.i64(ptr align 8 %b, ptr align 8 %insert.alloca2, i64 16, i1 false)
  %b.load = alloca %__closure0, align 8
  call void @llvm.memcpy.p0.p0.i64(ptr align 8 %b.load, ptr align 8 %b, i64 16, i1 false)
  %1 = getelementptr inbounds %__closure0, ptr %b.load, i32 0, i32 0
  %2 = load ptr, ptr %1, align 8
  %3 = getelementptr inbounds %__closure0, ptr %b.load, i32 0, i32 1
  %4 = load i32, ptr %3, align 4
  %5 = getelementptr inbounds %__closure0, ptr %b.load, i32 0, i32 2
  %6 = load i32, ptr %5, align 4
  %7 = call i32 %2(ptr %__context, i32 %4, i32 %6, i32 1), !dbg !7
  ret void
}

define i32 @_CX1N4main11____lambda0EM3std5int321_M3std5int32C2_M3std5int32M3std5int32(ptr %__context, i32 %__capture_p, i32 %__capture_d, i32 %c) #0 !dbg !8 {
  %__capture_p1 = alloca i32, align 4
  %__capture_d2 = alloca i32, align 4
  %c3 = alloca i32, align 4
  store i32 %__capture_p, ptr %__capture_p1, align 4
  store i32 %__capture_d, ptr %__capture_d2, align 4
  store i32 %c, ptr %c3, align 4
  %c.load = load i32, ptr %c3, align 4
  %__capture_p.load = load i32, ptr %__capture_p1, align 4
  %1 = call { i32, i1 } @llvm.sadd.with.overflow.i32(i32 %c.load, i32 %__capture_p.load)
  %2 = extractvalue { i32, i1 } %1, 0
  %3 = extractvalue { i32, i1 } %1, 1
  %4 = xor i1 %3, true
  %overflow.condition = icmp eq i1 %4, false
  br i1 %overflow.condition, label %overflow.fail, label %overflow.success

overflow.fail:                                    ; preds = %0
  %5 = call %never @_CX1N3std10assertFailEM3std5never1_PM3std4char(ptr %__context, ptr @0), !dbg !9
  unreachable

overflow.success:                                 ; preds = %0
  %__capture_d.load = load i32, ptr %__capture_d2, align 4
  %6 = call { i32, i1 } @llvm.sadd.with.overflow.i32(i32 %2, i32 %__capture_d.load)
  %7 = extractvalue { i32, i1 } %6, 0
  %8 = extractvalue { i32, i1 } %6, 1
  %9 = xor i1 %8, true
  %overflow.condition4 = icmp eq i1 %9, false
  br i1 %overflow.condition4, label %overflow.fail5, label %overflow.success6

overflow.fail5:                                   ; preds = %overflow.success
  %10 = call %never @_CX1N3std10assertFailEM3std5never1_PM3std4char(ptr %__context, ptr @1), !dbg !9
  unreachable

overflow.success6:                                ; preds = %overflow.success
  ret i32 %7
}

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias writeonly captures(none), ptr noalias readonly captures(none), i64, i1 immarg) #1

; Function Attrs: nocallback nocreateundeforpoison nofree nosync nounwind speculatable willreturn memory(none)
declare { i32, i1 } @llvm.sadd.with.overflow.i32(i32, i32) #2

declare %never @_CX1N3std10assertFailEM3std5never1_PM3std4char(ptr, ptr) #0

define i32 @main() #0 !dbg !10 {
  %1 = call ptr @_set_thread_local_invalid_parameter_handler(ptr @__cx_noop_invalid_parameter_handler), !dbg !11
  %s = alloca %S, align 8
  %a = alloca i32, align 4
  %b = alloca %__closure2, align 8
  %2 = call ptr @_CX1N3std11ambientRootERM3std7Context0_(ptr null), !dbg !11
  call void @_CX1N4main13capturesParamE4void1_M3std5int32(ptr %2, i32 100), !dbg !12
  call void @_CX1N4mainM4main1S4initE4void1_M3std5int32(ptr %2, ptr %s, i32 5), !dbg !13
  %3 = call i32 @_CX1N4mainM4main1S3getEM3std5int321_M3std5int32(ptr %2, ptr %s, i32 1), !dbg !14
  store i32 1, ptr %a, align 4
  %a.load = load i32, ptr %a, align 4
  %insert.alloca = alloca %__closure2, align 8
  store %__closure2 { ptr @_CX1N4main11____lambda2EM3std5int321_M3std5int32C1_M3std5int32, i32 undef }, ptr %insert.alloca, align 8
  %insert.gep = getelementptr inbounds %__closure2, ptr %insert.alloca, i32 0, i32 1
  store i32 %a.load, ptr %insert.gep, align 4
  call void @llvm.memcpy.p0.p0.i64(ptr align 8 %b, ptr align 8 %insert.alloca, i64 16, i1 false)
  %b.load = alloca %__closure2, align 8
  call void @llvm.memcpy.p0.p0.i64(ptr align 8 %b.load, ptr align 8 %b, i64 16, i1 false)
  %4 = getelementptr inbounds %__closure2, ptr %b.load, i32 0, i32 0
  %5 = load ptr, ptr %4, align 8
  %6 = getelementptr inbounds %__closure2, ptr %b.load, i32 0, i32 1
  %7 = load i32, ptr %6, align 4
  %8 = call i32 %5(ptr %2, i32 %7, i32 2), !dbg !15
  call void @_CX1N3std10checkLeaksE4void0_(ptr %2), !dbg !11
  ret i32 0
}

define private void @__cx_noop_invalid_parameter_handler(ptr %0, ptr %1, ptr %2, i32 %3, i64 %4) {
  ret void
}

declare ptr @_set_thread_local_invalid_parameter_handler(ptr)

declare ptr @_CX1N3std11ambientRootERM3std7Context0_(ptr) #0

define void @_CX1N4mainM4main1S4initE4void1_M3std5int32(ptr %__context, ptr %this, i32 %d) #0 !dbg !16 {
  %this1 = alloca ptr, align 8
  %d2 = alloca i32, align 4
  store ptr %this, ptr %this1, align 8
  store i32 %d, ptr %d2, align 4
  %this.load = load ptr, ptr %this1, align 8
  %d3 = getelementptr inbounds %S, ptr %this.load, i32 0, i32 0
  %d.load = load i32, ptr %d2, align 4
  store i32 %d.load, ptr %d3, align 4
  ret void
}

define i32 @_CX1N4mainM4main1S3getEM3std5int321_M3std5int32(ptr %__context, ptr %this, i32 %c) #0 !dbg !17 {
  %this1 = alloca ptr, align 8
  %c2 = alloca i32, align 4
  %b = alloca %__closure1, align 8
  store ptr %this, ptr %this1, align 8
  store i32 %c, ptr %c2, align 4
  %c.load = load i32, ptr %c2, align 4
  %insert.alloca = alloca %__closure1, align 8
  store %__closure1 { ptr @_CX1N4main11____lambda1EM3std5int321_M3std5int32C2_M3std5int32PM4main1S, i32 undef, ptr undef }, ptr %insert.alloca, align 8
  %insert.gep = getelementptr inbounds %__closure1, ptr %insert.alloca, i32 0, i32 1
  store i32 %c.load, ptr %insert.gep, align 4
  %this.load = load ptr, ptr %this1, align 8
  %insert.alloca3 = alloca %__closure1, align 8
  call void @llvm.memcpy.p0.p0.i64(ptr align 8 %insert.alloca3, ptr align 8 %insert.alloca, i64 24, i1 false)
  %insert.gep4 = getelementptr inbounds %__closure1, ptr %insert.alloca3, i32 0, i32 2
  store ptr %this.load, ptr %insert.gep4, align 8
  call void @llvm.memcpy.p0.p0.i64(ptr align 8 %b, ptr align 8 %insert.alloca3, i64 24, i1 false)
  %b.load = alloca %__closure1, align 8
  call void @llvm.memcpy.p0.p0.i64(ptr align 8 %b.load, ptr align 8 %b, i64 24, i1 false)
  %1 = getelementptr inbounds %__closure1, ptr %b.load, i32 0, i32 0
  %2 = load ptr, ptr %1, align 8
  %3 = getelementptr inbounds %__closure1, ptr %b.load, i32 0, i32 1
  %4 = load i32, ptr %3, align 4
  %5 = getelementptr inbounds %__closure1, ptr %b.load, i32 0, i32 2
  %6 = load ptr, ptr %5, align 8
  %7 = call i32 %2(ptr %__context, i32 %4, ptr %6, i32 1), !dbg !18
  ret i32 %7
}

define i32 @_CX1N4main11____lambda2EM3std5int321_M3std5int32C1_M3std5int32(ptr %__context, i32 %__capture_a, i32 %c) #0 !dbg !19 {
  %__capture_a1 = alloca i32, align 4
  %c2 = alloca i32, align 4
  store i32 %__capture_a, ptr %__capture_a1, align 4
  store i32 %c, ptr %c2, align 4
  %c.load = load i32, ptr %c2, align 4
  %__capture_a.load = load i32, ptr %__capture_a1, align 4
  %1 = call { i32, i1 } @llvm.sadd.with.overflow.i32(i32 %c.load, i32 %__capture_a.load)
  %2 = extractvalue { i32, i1 } %1, 0
  %3 = extractvalue { i32, i1 } %1, 1
  %4 = xor i1 %3, true
  %overflow.condition = icmp eq i1 %4, false
  br i1 %overflow.condition, label %overflow.fail, label %overflow.success

overflow.fail:                                    ; preds = %0
  %5 = call %never @_CX1N3std10assertFailEM3std5never1_PM3std4char(ptr %__context, ptr @2), !dbg !20
  unreachable

overflow.success:                                 ; preds = %0
  ret i32 %2
}

declare void @_CX1N3std10checkLeaksE4void0_(ptr) #0

define i32 @_CX1N4main11____lambda1EM3std5int321_M3std5int32C2_M3std5int32PM4main1S(ptr %__context, i32 %__capture_c, ptr %__capture_this, i32 %x) #0 !dbg !21 {
  %__capture_c1 = alloca i32, align 4
  %__capture_this2 = alloca ptr, align 8
  %x3 = alloca i32, align 4
  store i32 %__capture_c, ptr %__capture_c1, align 4
  store ptr %__capture_this, ptr %__capture_this2, align 8
  store i32 %x, ptr %x3, align 4
  %x.load = load i32, ptr %x3, align 4
  %__capture_c.load = load i32, ptr %__capture_c1, align 4
  %1 = call { i32, i1 } @llvm.sadd.with.overflow.i32(i32 %x.load, i32 %__capture_c.load)
  %2 = extractvalue { i32, i1 } %1, 0
  %3 = extractvalue { i32, i1 } %1, 1
  %4 = xor i1 %3, true
  %overflow.condition = icmp eq i1 %4, false
  br i1 %overflow.condition, label %overflow.fail, label %overflow.success

overflow.fail:                                    ; preds = %0
  %5 = call %never @_CX1N3std10assertFailEM3std5never1_PM3std4char(ptr %__context, ptr @3), !dbg !22
  unreachable

overflow.success:                                 ; preds = %0
  %__capture_this.load = load ptr, ptr %__capture_this2, align 8
  %d = getelementptr inbounds %S, ptr %__capture_this.load, i32 0, i32 0
  %d.load = load i32, ptr %d, align 4
  %6 = call { i32, i1 } @llvm.sadd.with.overflow.i32(i32 %2, i32 %d.load)
  %7 = extractvalue { i32, i1 } %6, 0
  %8 = extractvalue { i32, i1 } %6, 1
  %9 = xor i1 %8, true
  %overflow.condition4 = icmp eq i1 %9, false
  br i1 %overflow.condition4, label %overflow.fail5, label %overflow.success6

overflow.fail5:                                   ; preds = %overflow.success
  %10 = call %never @_CX1N3std10assertFailEM3std5never1_PM3std4char(ptr %__context, ptr @4), !dbg !22
  unreachable

overflow.success6:                                ; preds = %overflow.success
  ret i32 %7
}

attributes #0 = { "frame-pointer"="all" }
attributes #1 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { nocallback nocreateundeforpoison nofree nosync nounwind speculatable willreturn memory(none) }

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
!11 = !DILocation(line: 18, column: 6, scope: !10)
!12 = !DILocation(line: 19, column: 5, scope: !10)
!13 = !DILocation(line: 20, column: 13, scope: !10)
!14 = !DILocation(line: 21, column: 11, scope: !10)
!15 = !DILocation(line: 24, column: 9, scope: !10)
!16 = distinct !DISubprogram(name: "init", linkageName: "_CX1N4mainM4main1S4initE4void1_M3std5int32", scope: !3, file: !3, line: 9, type: !5, scopeLine: 9, spFlags: DISPFlagDefinition, unit: !2)
!17 = distinct !DISubprogram(name: "get", linkageName: "_CX1N4mainM4main1S3getEM3std5int321_M3std5int32", scope: !3, file: !3, line: 12, type: !5, scopeLine: 12, spFlags: DISPFlagDefinition, unit: !2)
!18 = !DILocation(line: 14, column: 16, scope: !17)
!19 = distinct !DISubprogram(name: "__lambda2", linkageName: "_CX1N4main11____lambda2EM3std5int321_M3std5int32C1_M3std5int32", scope: !3, file: !3, line: 23, type: !5, scopeLine: 23, spFlags: DISPFlagDefinition, unit: !2)
!20 = !DILocation(line: 23, column: 13, scope: !19)
!21 = distinct !DISubprogram(name: "__lambda1", linkageName: "_CX1N4main11____lambda1EM3std5int321_M3std5int32C2_M3std5int32PM4main1S", scope: !3, file: !3, line: 13, type: !5, scopeLine: 13, spFlags: DISPFlagDefinition, unit: !2)
!22 = !DILocation(line: 13, column: 17, scope: !21)
