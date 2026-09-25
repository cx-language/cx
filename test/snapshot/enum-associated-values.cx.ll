
%0 = type { { i1, i32 } }
%E = type { i32, %0 }
%never = type {}

@0 = private unnamed_addr constant [74 x i8] c"invalid value in switch over enum 'E' at enum-associated-values.cx:14:13\0A\00", align 1

define i32 @main() #0 !dbg !4 {
  %e = alloca %E, align 8
  %enum = alloca %E, align 8
  %enum1 = alloca %E, align 8
  %enum2 = alloca %E, align 8
  %enum3 = alloca %E, align 8
  %tag = getelementptr inbounds %E, ptr %enum, i32 0, i32 0
  store i32 0, ptr %tag, align 4
  %enum.load = load %E, ptr %enum, align 4
  store %E %enum.load, ptr %e, align 4
  %tag4 = getelementptr inbounds %E, ptr %enum1, i32 0, i32 0
  store i32 1, ptr %tag4, align 4
  %associatedValue = getelementptr inbounds %E, ptr %enum1, i32 0, i32 1
  store { i1, i32 } { i1 false, i32 42 }, ptr %associatedValue, align 4
  %enum.load5 = load %E, ptr %enum1, align 4
  store %E %enum.load5, ptr %e, align 4
  %tag6 = getelementptr inbounds %E, ptr %enum2, i32 0, i32 0
  store i32 2, ptr %tag6, align 4
  %associatedValue7 = getelementptr inbounds %E, ptr %enum2, i32 0, i32 1
  store { i32 } { i32 43 }, ptr %associatedValue7, align 4
  %enum.load8 = load %E, ptr %enum2, align 4
  store %E %enum.load8, ptr %e, align 4
  %e.tag = getelementptr inbounds %E, ptr %e, i32 0, i32 0
  %e.tag.load = load i32, ptr %e.tag, align 4
  switch i32 %e.tag.load, label %switch.default [
    i32 0, label %switch.case.0
    i32 1, label %switch.case.1
    i32 2, label %switch.case.2
  ]

switch.case.0:                                    ; preds = %0
  br label %switch.end

switch.case.1:                                    ; preds = %0
  %1 = getelementptr inbounds %E, ptr %e, i32 0, i32 1
  %tag9 = getelementptr inbounds %E, ptr %enum3, i32 0, i32 0
  store i32 2, ptr %tag9, align 4
  %i = getelementptr inbounds { i1, i32 }, ptr %1, i32 0, i32 1
  %i.load = load i32, ptr %i, align 4
  %2 = insertvalue { i32 } undef, i32 %i.load, 0
  %associatedValue10 = getelementptr inbounds %E, ptr %enum3, i32 0, i32 1
  store { i32 } %2, ptr %associatedValue10, align 4
  %enum.load11 = load %E, ptr %enum3, align 4
  store %E %enum.load11, ptr %e, align 4
  %i12 = getelementptr inbounds { i1, i32 }, ptr %1, i32 0, i32 1
  %i.load13 = load i32, ptr %i12, align 4
  ret i32 %i.load13

switch.case.2:                                    ; preds = %0
  %3 = getelementptr inbounds %E, ptr %e, i32 0, i32 1
  %eb.load = load i32, ptr %3, align 4
  ret i32 %eb.load

switch.default:                                   ; preds = %0
  %4 = call %never @_CX1N3std10assertFailEM3std5never1_PKM3std4char(ptr @0), !dbg !7
  unreachable

switch.end:                                       ; preds = %switch.case.0
  %e.tag14 = getelementptr inbounds %E, ptr %e, i32 0, i32 0
  %e.tag.load15 = load i32, ptr %e.tag14, align 4
  %5 = icmp eq i32 %e.tag.load15, 0
  %e.tag16 = getelementptr inbounds %E, ptr %e, i32 0, i32 0
  %e.tag.load17 = load i32, ptr %e.tag16, align 4
  %6 = icmp eq i32 %e.tag.load17, 1
  ret i32 0
}

declare %never @_CX1N3std10assertFailEM3std5never1_PKM3std4char(ptr) #0

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "enum-associated-values.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 9, type: !5, scopeLine: 9, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 9, column: 5, scope: !4)
