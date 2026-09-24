
@0 = private unnamed_addr constant [49 x i8] c"integer overflow at lambda-noncapturing.cx:8:29\0A\00", align 1
@1 = private unnamed_addr constant [49 x i8] c"integer overflow at lambda-noncapturing.cx:9:31\0A\00", align 1
@2 = private unnamed_addr constant [50 x i8] c"integer overflow at lambda-noncapturing.cx:12:22\0A\00", align 1
@3 = private unnamed_addr constant [50 x i8] c"integer overflow at lambda-noncapturing.cx:14:22\0A\00", align 1
@4 = private unnamed_addr constant [50 x i8] c"integer overflow at lambda-noncapturing.cx:18:19\0A\00", align 1

define i32 @main() #0 !dbg !4 {
  call void @_EN4main3fooI5int32EEF5int325int32_5int32(ptr @_EN4main9__lambda0E5int325int32), !dbg !7
  call void @_EN4main3fooI5int32EEF5int325int32_5int32(ptr @_EN4main9__lambda1E5int325int32), !dbg !8
  call void @_EN4main3fooI5int32EEF5int325int32_5int32(ptr @_EN4main9__lambda2E5int325int32), !dbg !9
  call void @_EN4main3fooI5int32EEF5int325int32_5int32(ptr @_EN4main9__lambda3E5int325int32), !dbg !10
  call void @_EN4main3fooI4voidEEF5int325int32_4void(ptr @_EN4main9__lambda4E5int325int32), !dbg !11
  ret i32 0
}

define void @_EN4main3fooI5int32EEF5int325int32_5int32(ptr %a) #0 !dbg !12 {
  %a1 = alloca ptr, align 8
  store ptr %a, ptr %a1, align 8
  %a.load = load ptr, ptr %a1, align 8
  %1 = call i32 %a.load(i32 1, i32 2), !dbg !13
  ret void
}

define i32 @_EN4main9__lambda0E5int325int32(i32 %a, i32 %b) #0 !dbg !14 {
  %a1 = alloca i32, align 4
  %b2 = alloca i32, align 4
  store i32 %a, ptr %a1, align 4
  store i32 %b, ptr %b2, align 4
  %a.load = load i32, ptr %a1, align 4
  %b.load = load i32, ptr %b2, align 4
  %1 = sext i32 %a.load to i64
  %2 = sext i32 %b.load to i64
  %3 = add i64 %1, %2
  %4 = trunc i64 %3 to i32
  %5 = sext i32 %4 to i64
  %6 = icmp ne i64 %3, %5
  %7 = xor i1 %6, true
  %overflow.condition = icmp eq i1 %7, false
  br i1 %overflow.condition, label %overflow.fail, label %overflow.success

overflow.fail:                                    ; preds = %0
  call void @_EN3std10assertFailEP4char(ptr @0), !dbg !15
  unreachable

overflow.success:                                 ; preds = %0
  ret i32 %4
}

define i32 @_EN4main9__lambda1E5int325int32(i32 %a, i32 %b) #0 !dbg !16 {
  %a1 = alloca i32, align 4
  %b2 = alloca i32, align 4
  store i32 %a, ptr %a1, align 4
  store i32 %b, ptr %b2, align 4
  %a.load = load i32, ptr %a1, align 4
  %b.load = load i32, ptr %b2, align 4
  %1 = sext i32 %a.load to i64
  %2 = sext i32 %b.load to i64
  %3 = add i64 %1, %2
  %4 = trunc i64 %3 to i32
  %5 = sext i32 %4 to i64
  %6 = icmp ne i64 %3, %5
  %7 = xor i1 %6, true
  %overflow.condition = icmp eq i1 %7, false
  br i1 %overflow.condition, label %overflow.fail, label %overflow.success

overflow.fail:                                    ; preds = %0
  call void @_EN3std10assertFailEP4char(ptr @1), !dbg !17
  unreachable

overflow.success:                                 ; preds = %0
  ret i32 %4
}

define i32 @_EN4main9__lambda2E5int325int32(i32 %a, i32 %b) #0 !dbg !18 {
  %a1 = alloca i32, align 4
  %b2 = alloca i32, align 4
  store i32 %a, ptr %a1, align 4
  store i32 %b, ptr %b2, align 4
  %a.load = load i32, ptr %a1, align 4
  %b.load = load i32, ptr %b2, align 4
  %1 = icmp sgt i32 %a.load, %b.load
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  %a.load3 = load i32, ptr %a1, align 4
  %b.load4 = load i32, ptr %b2, align 4
  %2 = sext i32 %a.load3 to i64
  %3 = sext i32 %b.load4 to i64
  %4 = add i64 %2, %3
  %5 = trunc i64 %4 to i32
  %6 = sext i32 %5 to i64
  %7 = icmp ne i64 %4, %6
  %8 = xor i1 %7, true
  %overflow.condition = icmp eq i1 %8, false
  br i1 %overflow.condition, label %overflow.fail, label %overflow.success

if.else:                                          ; preds = %0
  %a.load5 = load i32, ptr %a1, align 4
  %b.load6 = load i32, ptr %b2, align 4
  %9 = sext i32 %a.load5 to i64
  %10 = sext i32 %b.load6 to i64
  %11 = sub i64 %9, %10
  %12 = trunc i64 %11 to i32
  %13 = sext i32 %12 to i64
  %14 = icmp ne i64 %11, %13
  %15 = xor i1 %14, true
  %overflow.condition7 = icmp eq i1 %15, false
  br i1 %overflow.condition7, label %overflow.fail8, label %overflow.success9

if.end:                                           ; No predecessors!
  unreachable

overflow.fail:                                    ; preds = %if.then
  call void @_EN3std10assertFailEP4char(ptr @2), !dbg !19
  unreachable

overflow.success:                                 ; preds = %if.then
  ret i32 %5

overflow.fail8:                                   ; preds = %if.else
  call void @_EN3std10assertFailEP4char(ptr @3), !dbg !19
  unreachable

overflow.success9:                                ; preds = %if.else
  ret i32 %12
}

define i32 @_EN4main9__lambda3E5int325int32(i32 %a, i32 %b) #0 !dbg !20 {
  %a1 = alloca i32, align 4
  %b2 = alloca i32, align 4
  %x = alloca i32, align 4
  store i32 %a, ptr %a1, align 4
  store i32 %b, ptr %b2, align 4
  %a.load = load i32, ptr %a1, align 4
  %b.load = load i32, ptr %b2, align 4
  %1 = sext i32 %a.load to i64
  %2 = sext i32 %b.load to i64
  %3 = add i64 %1, %2
  %4 = trunc i64 %3 to i32
  %5 = sext i32 %4 to i64
  %6 = icmp ne i64 %3, %5
  %7 = xor i1 %6, true
  %overflow.condition = icmp eq i1 %7, false
  br i1 %overflow.condition, label %overflow.fail, label %overflow.success

overflow.fail:                                    ; preds = %0
  call void @_EN3std10assertFailEP4char(ptr @4), !dbg !21
  unreachable

overflow.success:                                 ; preds = %0
  store i32 %4, ptr %x, align 4
  %x.load = load i32, ptr %x, align 4
  ret i32 %x.load
}

define void @_EN4main3fooI4voidEEF5int325int32_4void(ptr %a) #0 !dbg !22 {
  %a1 = alloca ptr, align 8
  store ptr %a, ptr %a1, align 8
  %a.load = load ptr, ptr %a1, align 8
  call void %a.load(i32 1, i32 2), !dbg !23
  ret void
}

define void @_EN4main9__lambda4E5int325int32(i32 %a, i32 %b) #0 !dbg !24 {
  %a1 = alloca i32, align 4
  %b2 = alloca i32, align 4
  store i32 %a, ptr %a1, align 4
  store i32 %b, ptr %b2, align 4
  %a.load = load i32, ptr %a1, align 4
  %b.load = load i32, ptr %b2, align 4
  %1 = icmp eq i32 %a.load, %b.load
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  ret void

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else
  ret void
}

declare void @_EN3std10assertFailEP4char(ptr) #0

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "lambda-noncapturing.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 7, type: !5, scopeLine: 7, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 8, column: 5, scope: !4)
!8 = !DILocation(line: 9, column: 5, scope: !4)
!9 = !DILocation(line: 10, column: 5, scope: !4)
!10 = !DILocation(line: 17, column: 5, scope: !4)
!11 = !DILocation(line: 21, column: 5, scope: !4)
!12 = distinct !DISubprogram(name: "foo", linkageName: "_EN4main3fooI5int32EEF5int325int32_5int32", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!13 = !DILocation(line: 4, column: 9, scope: !12)
!14 = distinct !DISubprogram(name: "__lambda0", linkageName: "_EN4main9__lambda0E5int325int32", scope: !3, file: !3, line: 8, type: !5, scopeLine: 8, spFlags: DISPFlagDefinition, unit: !2)
!15 = !DILocation(line: 8, column: 9, scope: !14)
!16 = distinct !DISubprogram(name: "__lambda1", linkageName: "_EN4main9__lambda1E5int325int32", scope: !3, file: !3, line: 9, type: !5, scopeLine: 9, spFlags: DISPFlagDefinition, unit: !2)
!17 = !DILocation(line: 9, column: 9, scope: !16)
!18 = distinct !DISubprogram(name: "__lambda2", linkageName: "_EN4main9__lambda2E5int325int32", scope: !3, file: !3, line: 10, type: !5, scopeLine: 10, spFlags: DISPFlagDefinition, unit: !2)
!19 = !DILocation(line: 10, column: 9, scope: !18)
!20 = distinct !DISubprogram(name: "__lambda3", linkageName: "_EN4main9__lambda3E5int325int32", scope: !3, file: !3, line: 17, type: !5, scopeLine: 17, spFlags: DISPFlagDefinition, unit: !2)
!21 = !DILocation(line: 17, column: 9, scope: !20)
!22 = distinct !DISubprogram(name: "foo", linkageName: "_EN4main3fooI4voidEEF5int325int32_4void", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!23 = !DILocation(line: 4, column: 9, scope: !22)
!24 = distinct !DISubprogram(name: "__lambda4", linkageName: "_EN4main9__lambda4E5int325int32", scope: !3, file: !3, line: 21, type: !5, scopeLine: 21, spFlags: DISPFlagDefinition, unit: !2)
