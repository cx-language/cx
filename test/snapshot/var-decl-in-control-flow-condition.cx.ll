
@0 = private unnamed_addr constant [61 x i8] c"Unwrap failed at var-decl-in-control-flow-condition.cx:5:11\0A\00", align 1
@1 = private unnamed_addr constant [61 x i8] c"Unwrap failed at var-decl-in-control-flow-condition.cx:9:11\0A\00", align 1
@2 = private unnamed_addr constant [62 x i8] c"Unwrap failed at var-decl-in-control-flow-condition.cx:15:11\0A\00", align 1
@3 = private unnamed_addr constant [62 x i8] c"Unwrap failed at var-decl-in-control-flow-condition.cx:19:11\0A\00", align 1
@4 = private unnamed_addr constant [62 x i8] c"Unwrap failed at var-decl-in-control-flow-condition.cx:23:11\0A\00", align 1
@5 = private unnamed_addr constant [62 x i8] c"Unwrap failed at var-decl-in-control-flow-condition.cx:28:11\0A\00", align 1
@6 = private unnamed_addr constant [62 x i8] c"Unwrap failed at var-decl-in-control-flow-condition.cx:32:11\0A\00", align 1

define void @_EN4main1fE() #0 !dbg !4 {
  %c = alloca ptr, align 8
  %c1 = alloca ptr, align 8
  br label %loop.condition

loop.condition:                                   ; preds = %__implicit_unwrap.success, %0
  %1 = call ptr @_EN4main1hE(), !dbg !7
  store ptr %1, ptr %c, align 8
  %c.load = load ptr, ptr %c, align 8
  %2 = icmp ne ptr %c.load, null
  br i1 %2, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %c.load2 = load ptr, ptr %c, align 8
  %__implicit_unwrap.condition = icmp eq ptr %c.load2, null
  br i1 %__implicit_unwrap.condition, label %__implicit_unwrap.fail, label %__implicit_unwrap.success

loop.end:                                         ; preds = %loop.condition
  %3 = call ptr @_EN4main1hE(), !dbg !8
  store ptr %3, ptr %c1, align 8
  %c.load3 = load ptr, ptr %c1, align 8
  %4 = icmp ne ptr %c.load3, null
  br i1 %4, label %if.then, label %if.else

__implicit_unwrap.fail:                           ; preds = %loop.body
  call void @_EN3std10assertFailEP4char(ptr @0), !dbg !9
  unreachable

__implicit_unwrap.success:                        ; preds = %loop.body
  call void @_EN4main1jEP3int(ptr %c.load2), !dbg !10
  br label %loop.condition

if.then:                                          ; preds = %loop.end
  %c.load4 = load ptr, ptr %c1, align 8
  %__implicit_unwrap.condition5 = icmp eq ptr %c.load4, null
  br i1 %__implicit_unwrap.condition5, label %__implicit_unwrap.fail6, label %__implicit_unwrap.success7

if.else:                                          ; preds = %loop.end
  br label %if.end

if.end:                                           ; preds = %__implicit_unwrap.success7, %if.else
  ret void

__implicit_unwrap.fail6:                          ; preds = %if.then
  call void @_EN3std10assertFailEP4char(ptr @1), !dbg !9
  unreachable

__implicit_unwrap.success7:                       ; preds = %if.then
  call void @_EN4main1jEP3int(ptr %c.load4), !dbg !11
  br label %if.end
}

define ptr @_EN4main1hE() #0 !dbg !12 {
  ret ptr null
}

declare void @_EN3std10assertFailEP4char(ptr) #0

define void @_EN4main1jEP3int(ptr %i) #0 !dbg !13 {
  %i1 = alloca ptr, align 8
  store ptr %i, ptr %i1, align 8
  ret void
}

define void @_EN4main1gE() #0 !dbg !14 {
  %c1 = alloca ptr, align 8
  %c2 = alloca ptr, align 8
  %c3 = alloca ptr, align 8
  %c4 = alloca ptr, align 8
  %c5 = alloca ptr, align 8
  br label %loop.condition

loop.condition:                                   ; preds = %__implicit_unwrap.success, %0
  %1 = call ptr @_EN4main1hE(), !dbg !15
  store ptr %1, ptr %c1, align 8
  %c1.load = load ptr, ptr %c1, align 8
  %2 = icmp ne ptr %c1.load, null
  br i1 %2, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %c1.load1 = load ptr, ptr %c1, align 8
  %__implicit_unwrap.condition = icmp eq ptr %c1.load1, null
  br i1 %__implicit_unwrap.condition, label %__implicit_unwrap.fail, label %__implicit_unwrap.success

loop.end:                                         ; preds = %loop.condition
  %3 = call ptr @_EN4main1hE(), !dbg !16
  store ptr %3, ptr %c2, align 8
  %c2.load = load ptr, ptr %c2, align 8
  %4 = icmp ne ptr %c2.load, null
  br i1 %4, label %if.then, label %if.else

__implicit_unwrap.fail:                           ; preds = %loop.body
  call void @_EN3std10assertFailEP4char(ptr @2), !dbg !17
  unreachable

__implicit_unwrap.success:                        ; preds = %loop.body
  call void @_EN4main1jEP3int(ptr %c1.load1), !dbg !18
  br label %loop.condition

if.then:                                          ; preds = %loop.end
  %c2.load2 = load ptr, ptr %c2, align 8
  %__implicit_unwrap.condition3 = icmp eq ptr %c2.load2, null
  br i1 %__implicit_unwrap.condition3, label %__implicit_unwrap.fail4, label %__implicit_unwrap.success5

if.else:                                          ; preds = %loop.end
  br label %if.end

if.end:                                           ; preds = %__implicit_unwrap.success5, %if.else
  br label %loop.condition6

__implicit_unwrap.fail4:                          ; preds = %if.then
  call void @_EN3std10assertFailEP4char(ptr @3), !dbg !17
  unreachable

__implicit_unwrap.success5:                       ; preds = %if.then
  call void @_EN4main1jEP3int(ptr %c2.load2), !dbg !19
  br label %if.end

loop.condition6:                                  ; preds = %__implicit_unwrap.success12, %if.end
  %5 = call ptr @_EN4main1hE(), !dbg !20
  store ptr %5, ptr %c3, align 8
  %c3.load = load ptr, ptr %c3, align 8
  %6 = icmp ne ptr %c3.load, null
  br i1 %6, label %loop.body7, label %loop.end10

loop.body7:                                       ; preds = %loop.condition6
  %c3.load8 = load ptr, ptr %c3, align 8
  %__implicit_unwrap.condition9 = icmp eq ptr %c3.load8, null
  br i1 %__implicit_unwrap.condition9, label %__implicit_unwrap.fail11, label %__implicit_unwrap.success12

loop.end10:                                       ; preds = %loop.condition6
  %7 = call ptr @_EN4main1hE(), !dbg !21
  store ptr %7, ptr %c4, align 8
  %c4.load = load ptr, ptr %c4, align 8
  %8 = icmp ne ptr %c4.load, null
  br i1 %8, label %if.then13, label %if.else16

__implicit_unwrap.fail11:                         ; preds = %loop.body7
  call void @_EN3std10assertFailEP4char(ptr @4), !dbg !17
  unreachable

__implicit_unwrap.success12:                      ; preds = %loop.body7
  call void @_EN4main1jEP3int(ptr %c3.load8), !dbg !22
  br label %loop.condition6

if.then13:                                        ; preds = %loop.end10
  %c4.load14 = load ptr, ptr %c4, align 8
  %__implicit_unwrap.condition15 = icmp eq ptr %c4.load14, null
  br i1 %__implicit_unwrap.condition15, label %__implicit_unwrap.fail18, label %__implicit_unwrap.success19

if.else16:                                        ; preds = %loop.end10
  br label %if.end17

if.end17:                                         ; preds = %__implicit_unwrap.success19, %if.else16
  %9 = call ptr @_EN4main1hE(), !dbg !23
  store ptr %9, ptr %c5, align 8
  %c5.load = load ptr, ptr %c5, align 8
  %10 = icmp ne ptr %c5.load, null
  br i1 %10, label %if.then20, label %if.else23

__implicit_unwrap.fail18:                         ; preds = %if.then13
  call void @_EN3std10assertFailEP4char(ptr @5), !dbg !17
  unreachable

__implicit_unwrap.success19:                      ; preds = %if.then13
  call void @_EN4main1jEP3int(ptr %c4.load14), !dbg !24
  br label %if.end17

if.then20:                                        ; preds = %if.end17
  %c5.load21 = load ptr, ptr %c5, align 8
  %__implicit_unwrap.condition22 = icmp eq ptr %c5.load21, null
  br i1 %__implicit_unwrap.condition22, label %__implicit_unwrap.fail25, label %__implicit_unwrap.success26

if.else23:                                        ; preds = %if.end17
  br label %if.end24

if.end24:                                         ; preds = %__implicit_unwrap.success26, %if.else23
  ret void

__implicit_unwrap.fail25:                         ; preds = %if.then20
  call void @_EN3std10assertFailEP4char(ptr @6), !dbg !17
  unreachable

__implicit_unwrap.success26:                      ; preds = %if.then20
  call void @_EN4main1jEP3int(ptr %c5.load21), !dbg !25
  br label %if.end24
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "var-decl-in-control-flow-condition.cx")
!4 = distinct !DISubprogram(name: "f", linkageName: "_EN4main1fE", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 4, column: 20, scope: !4)
!8 = !DILocation(line: 8, column: 17, scope: !4)
!9 = !DILocation(line: 3, column: 6, scope: !4)
!10 = !DILocation(line: 5, column: 9, scope: !4)
!11 = !DILocation(line: 9, column: 9, scope: !4)
!12 = distinct !DISubprogram(name: "h", linkageName: "_EN4main1hE", scope: !3, file: !3, line: 35, type: !5, scopeLine: 35, spFlags: DISPFlagDefinition, unit: !2)
!13 = distinct !DISubprogram(name: "j", linkageName: "_EN4main1jEP3int", scope: !3, file: !3, line: 39, type: !5, scopeLine: 39, spFlags: DISPFlagDefinition, unit: !2)
!14 = distinct !DISubprogram(name: "g", linkageName: "_EN4main1gE", scope: !3, file: !3, line: 13, type: !5, scopeLine: 13, spFlags: DISPFlagDefinition, unit: !2)
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
