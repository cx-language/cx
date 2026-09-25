
%never = type {}

@0 = private unnamed_addr constant [61 x i8] c"Unwrap failed at var-decl-in-control-flow-condition.cx:5:11\0A\00", align 1
@1 = private unnamed_addr constant [61 x i8] c"Unwrap failed at var-decl-in-control-flow-condition.cx:9:11\0A\00", align 1
@2 = private unnamed_addr constant [62 x i8] c"Unwrap failed at var-decl-in-control-flow-condition.cx:15:11\0A\00", align 1
@3 = private unnamed_addr constant [62 x i8] c"Unwrap failed at var-decl-in-control-flow-condition.cx:19:11\0A\00", align 1
@4 = private unnamed_addr constant [62 x i8] c"Unwrap failed at var-decl-in-control-flow-condition.cx:23:11\0A\00", align 1
@5 = private unnamed_addr constant [62 x i8] c"Unwrap failed at var-decl-in-control-flow-condition.cx:28:11\0A\00", align 1
@6 = private unnamed_addr constant [62 x i8] c"Unwrap failed at var-decl-in-control-flow-condition.cx:32:11\0A\00", align 1

define void @_CX1N4main1fE4void0_() #0 !dbg !4 {
  %c = alloca ptr, align 8
  %c1 = alloca ptr, align 8
  br label %loop.condition

loop.condition:                                   ; preds = %__implicit_unwrap.success, %0
  %1 = call ptr @_CX1N4main1hEOPM3std5int320_(), !dbg !7
  store ptr %1, ptr %c, align 8
  %c.load = load ptr, ptr %c, align 8
  %2 = icmp ne ptr %c.load, null
  br i1 %2, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %c.load2 = load ptr, ptr %c, align 8
  %__implicit_unwrap.condition = icmp eq ptr %c.load2, null
  br i1 %__implicit_unwrap.condition, label %__implicit_unwrap.fail, label %__implicit_unwrap.success

loop.end:                                         ; preds = %loop.condition
  %3 = call ptr @_CX1N4main1hEOPM3std5int320_(), !dbg !8
  store ptr %3, ptr %c1, align 8
  %c.load3 = load ptr, ptr %c1, align 8
  %4 = icmp ne ptr %c.load3, null
  br i1 %4, label %if.then, label %if.else

__implicit_unwrap.fail:                           ; preds = %loop.body
  %5 = call %never @_CX1N3std10assertFailEM3std5never1_PKM3std4char(ptr @0), !dbg !9
  unreachable

__implicit_unwrap.success:                        ; preds = %loop.body
  %c.load4 = load ptr, ptr %c, align 8
  call void @_CX1N4main1jE4void1_PM3std5int32(ptr %c.load4), !dbg !10
  br label %loop.condition

if.then:                                          ; preds = %loop.end
  %c.load5 = load ptr, ptr %c1, align 8
  %__implicit_unwrap.condition6 = icmp eq ptr %c.load5, null
  br i1 %__implicit_unwrap.condition6, label %__implicit_unwrap.fail7, label %__implicit_unwrap.success8

if.else:                                          ; preds = %loop.end
  br label %if.end

if.end:                                           ; preds = %__implicit_unwrap.success8, %if.else
  ret void

__implicit_unwrap.fail7:                          ; preds = %if.then
  %6 = call %never @_CX1N3std10assertFailEM3std5never1_PKM3std4char(ptr @1), !dbg !9
  unreachable

__implicit_unwrap.success8:                       ; preds = %if.then
  %c.load9 = load ptr, ptr %c1, align 8
  call void @_CX1N4main1jE4void1_PM3std5int32(ptr %c.load9), !dbg !11
  br label %if.end
}

define ptr @_CX1N4main1hEOPM3std5int320_() #0 !dbg !12 {
  ret ptr null
}

declare %never @_CX1N3std10assertFailEM3std5never1_PKM3std4char(ptr) #0

define void @_CX1N4main1jE4void1_PM3std5int32(ptr %i) #0 !dbg !13 {
  %i1 = alloca ptr, align 8
  store ptr %i, ptr %i1, align 8
  ret void
}

define void @_CX1N4main1gE4void0_() #0 !dbg !14 {
  %c1 = alloca ptr, align 8
  %c2 = alloca ptr, align 8
  %c3 = alloca ptr, align 8
  %c4 = alloca ptr, align 8
  %c5 = alloca ptr, align 8
  br label %loop.condition

loop.condition:                                   ; preds = %__implicit_unwrap.success, %0
  %1 = call ptr @_CX1N4main1hEOPM3std5int320_(), !dbg !15
  store ptr %1, ptr %c1, align 8
  %c1.load = load ptr, ptr %c1, align 8
  %2 = icmp ne ptr %c1.load, null
  br i1 %2, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %c1.load1 = load ptr, ptr %c1, align 8
  %__implicit_unwrap.condition = icmp eq ptr %c1.load1, null
  br i1 %__implicit_unwrap.condition, label %__implicit_unwrap.fail, label %__implicit_unwrap.success

loop.end:                                         ; preds = %loop.condition
  %3 = call ptr @_CX1N4main1hEOPM3std5int320_(), !dbg !16
  store ptr %3, ptr %c2, align 8
  %c2.load = load ptr, ptr %c2, align 8
  %4 = icmp ne ptr %c2.load, null
  br i1 %4, label %if.then, label %if.else

__implicit_unwrap.fail:                           ; preds = %loop.body
  %5 = call %never @_CX1N3std10assertFailEM3std5never1_PKM3std4char(ptr @2), !dbg !17
  unreachable

__implicit_unwrap.success:                        ; preds = %loop.body
  %c1.load2 = load ptr, ptr %c1, align 8
  call void @_CX1N4main1jE4void1_PM3std5int32(ptr %c1.load2), !dbg !18
  br label %loop.condition

if.then:                                          ; preds = %loop.end
  %c2.load3 = load ptr, ptr %c2, align 8
  %__implicit_unwrap.condition4 = icmp eq ptr %c2.load3, null
  br i1 %__implicit_unwrap.condition4, label %__implicit_unwrap.fail5, label %__implicit_unwrap.success6

if.else:                                          ; preds = %loop.end
  br label %if.end

if.end:                                           ; preds = %__implicit_unwrap.success6, %if.else
  br label %loop.condition8

__implicit_unwrap.fail5:                          ; preds = %if.then
  %6 = call %never @_CX1N3std10assertFailEM3std5never1_PKM3std4char(ptr @3), !dbg !17
  unreachable

__implicit_unwrap.success6:                       ; preds = %if.then
  %c2.load7 = load ptr, ptr %c2, align 8
  call void @_CX1N4main1jE4void1_PM3std5int32(ptr %c2.load7), !dbg !19
  br label %if.end

loop.condition8:                                  ; preds = %__implicit_unwrap.success14, %if.end
  %7 = call ptr @_CX1N4main1hEOPM3std5int320_(), !dbg !20
  store ptr %7, ptr %c3, align 8
  %c3.load = load ptr, ptr %c3, align 8
  %8 = icmp ne ptr %c3.load, null
  br i1 %8, label %loop.body9, label %loop.end12

loop.body9:                                       ; preds = %loop.condition8
  %c3.load10 = load ptr, ptr %c3, align 8
  %__implicit_unwrap.condition11 = icmp eq ptr %c3.load10, null
  br i1 %__implicit_unwrap.condition11, label %__implicit_unwrap.fail13, label %__implicit_unwrap.success14

loop.end12:                                       ; preds = %loop.condition8
  %9 = call ptr @_CX1N4main1hEOPM3std5int320_(), !dbg !21
  store ptr %9, ptr %c4, align 8
  %c4.load = load ptr, ptr %c4, align 8
  %10 = icmp ne ptr %c4.load, null
  br i1 %10, label %if.then16, label %if.else19

__implicit_unwrap.fail13:                         ; preds = %loop.body9
  %11 = call %never @_CX1N3std10assertFailEM3std5never1_PKM3std4char(ptr @4), !dbg !17
  unreachable

__implicit_unwrap.success14:                      ; preds = %loop.body9
  %c3.load15 = load ptr, ptr %c3, align 8
  call void @_CX1N4main1jE4void1_PM3std5int32(ptr %c3.load15), !dbg !22
  br label %loop.condition8

if.then16:                                        ; preds = %loop.end12
  %c4.load17 = load ptr, ptr %c4, align 8
  %__implicit_unwrap.condition18 = icmp eq ptr %c4.load17, null
  br i1 %__implicit_unwrap.condition18, label %__implicit_unwrap.fail21, label %__implicit_unwrap.success22

if.else19:                                        ; preds = %loop.end12
  br label %if.end20

if.end20:                                         ; preds = %__implicit_unwrap.success22, %if.else19
  %12 = call ptr @_CX1N4main1hEOPM3std5int320_(), !dbg !23
  store ptr %12, ptr %c5, align 8
  %c5.load = load ptr, ptr %c5, align 8
  %13 = icmp ne ptr %c5.load, null
  br i1 %13, label %if.then24, label %if.else27

__implicit_unwrap.fail21:                         ; preds = %if.then16
  %14 = call %never @_CX1N3std10assertFailEM3std5never1_PKM3std4char(ptr @5), !dbg !17
  unreachable

__implicit_unwrap.success22:                      ; preds = %if.then16
  %c4.load23 = load ptr, ptr %c4, align 8
  call void @_CX1N4main1jE4void1_PM3std5int32(ptr %c4.load23), !dbg !24
  br label %if.end20

if.then24:                                        ; preds = %if.end20
  %c5.load25 = load ptr, ptr %c5, align 8
  %__implicit_unwrap.condition26 = icmp eq ptr %c5.load25, null
  br i1 %__implicit_unwrap.condition26, label %__implicit_unwrap.fail29, label %__implicit_unwrap.success30

if.else27:                                        ; preds = %if.end20
  br label %if.end28

if.end28:                                         ; preds = %__implicit_unwrap.success30, %if.else27
  ret void

__implicit_unwrap.fail29:                         ; preds = %if.then24
  %15 = call %never @_CX1N3std10assertFailEM3std5never1_PKM3std4char(ptr @6), !dbg !17
  unreachable

__implicit_unwrap.success30:                      ; preds = %if.then24
  %c5.load31 = load ptr, ptr %c5, align 8
  call void @_CX1N4main1jE4void1_PM3std5int32(ptr %c5.load31), !dbg !25
  br label %if.end28
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "var-decl-in-control-flow-condition.cx")
!4 = distinct !DISubprogram(name: "f", linkageName: "_CX1N4main1fE4void0_", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 4, column: 20, scope: !4)
!8 = !DILocation(line: 8, column: 17, scope: !4)
!9 = !DILocation(line: 3, column: 6, scope: !4)
!10 = !DILocation(line: 5, column: 9, scope: !4)
!11 = !DILocation(line: 9, column: 9, scope: !4)
!12 = distinct !DISubprogram(name: "h", linkageName: "_CX1N4main1hEOPM3std5int320_", scope: !3, file: !3, line: 35, type: !5, scopeLine: 35, spFlags: DISPFlagDefinition, unit: !2)
!13 = distinct !DISubprogram(name: "j", linkageName: "_CX1N4main1jE4void1_PM3std5int32", scope: !3, file: !3, line: 39, type: !5, scopeLine: 39, spFlags: DISPFlagDefinition, unit: !2)
!14 = distinct !DISubprogram(name: "g", linkageName: "_CX1N4main1gE4void0_", scope: !3, file: !3, line: 13, type: !5, scopeLine: 13, spFlags: DISPFlagDefinition, unit: !2)
!15 = !DILocation(line: 14, column: 20, scope: !14)
!16 = !DILocation(line: 18, column: 17, scope: !14)
!17 = !DILocation(line: 13, column: 6, scope: !14)
!18 = !DILocation(line: 15, column: 9, scope: !14)
!19 = !DILocation(line: 19, column: 9, scope: !14)
!20 = !DILocation(line: 22, column: 22, scope: !14)
!21 = !DILocation(line: 26, column: 17, scope: !14)
!22 = !DILocation(line: 23, column: 9, scope: !14)
!23 = !DILocation(line: 31, column: 17, scope: !14)
!24 = !DILocation(line: 28, column: 9, scope: !14)
!25 = !DILocation(line: 32, column: 9, scope: !14)
