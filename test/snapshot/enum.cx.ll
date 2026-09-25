
%never = type {}

@_CX1G4main1xE = private global i32 0
@0 = private unnamed_addr constant [56 x i8] c"invalid value in switch over enum 'A' at enum.cx:11:13\0A\00", align 1

define i32 @main() #0 !dbg !4 {
  %foo = alloca i32, align 4
  store i32 2, ptr %foo, align 4
  %foo.load = load i32, ptr %foo, align 4
  %1 = icmp eq i32 %foo.load, 1
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  br label %if.end

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %foo.load1 = load i32, ptr %foo, align 4
  %2 = icmp ne i32 %foo.load1, 0
  br i1 %2, label %if.then2, label %if.else3

if.then2:                                         ; preds = %if.end
  br label %if.end4

if.else3:                                         ; preds = %if.end
  br label %if.end4

if.end4:                                          ; preds = %if.else3, %if.then2
  %foo.load5 = load i32, ptr %foo, align 4
  switch i32 %foo.load5, label %switch.default [
    i32 0, label %switch.case.0
    i32 1, label %switch.case.1
    i32 2, label %switch.case.2
  ]

switch.case.0:                                    ; preds = %if.end4
  ret i32 0

switch.case.1:                                    ; preds = %if.end4
  ret i32 0

switch.case.2:                                    ; preds = %if.end4
  ret i32 0

switch.default:                                   ; preds = %if.end4
  %3 = call %never @_CX1N3std10assertFailEM3std5never1_PKM3std4char(ptr @0), !dbg !7
  unreachable
}

declare %never @_CX1N3std10assertFailEM3std5never1_PKM3std4char(ptr) #0

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "enum.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 7, type: !5, scopeLine: 7, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 7, column: 6, scope: !4)
