
%0 = type { { i32 } }
%"Opt<int32>" = type { i32, %0 }
%never = type {}

@0 = private unnamed_addr constant [66 x i8] c"invalid value in switch over enum 'Opt' at generic-enum.cx:12:13\0A\00", align 1
@1 = private unnamed_addr constant [43 x i8] c"integer overflow at generic-enum.cx:14:39\0A\00", align 1
@2 = private unnamed_addr constant [66 x i8] c"invalid value in switch over enum 'Opt' at generic-enum.cx:19:13\0A\00", align 1

define i32 @main() #0 !dbg !4 {
  %a = alloca %"Opt<int32>", align 8
  %enum = alloca %"Opt<int32>", align 8
  %b = alloca %"Opt<int32>", align 8
  %enum1 = alloca %"Opt<int32>", align 8
  %enum2 = alloca %"Opt<int32>", align 8
  %enum3 = alloca %"Opt<int32>", align 8
  %tag = getelementptr inbounds %"Opt<int32>", ptr %enum, i32 0, i32 0
  store i32 0, ptr %tag, align 4
  %associatedValue = getelementptr inbounds %"Opt<int32>", ptr %enum, i32 0, i32 1
  store { i32 } { i32 1 }, ptr %associatedValue, align 4
  %enum.load = load %"Opt<int32>", ptr %enum, align 4
  store %"Opt<int32>" %enum.load, ptr %a, align 4
  %tag4 = getelementptr inbounds %"Opt<int32>", ptr %enum1, i32 0, i32 0
  store i32 1, ptr %tag4, align 4
  %enum.load5 = load %"Opt<int32>", ptr %enum1, align 4
  store %"Opt<int32>" %enum.load5, ptr %b, align 4
  %a.tag = getelementptr inbounds %"Opt<int32>", ptr %a, i32 0, i32 0
  %a.tag.load = load i32, ptr %a.tag, align 4
  switch i32 %a.tag.load, label %switch.default [
    i32 0, label %switch.case.0
    i32 1, label %switch.case.1
  ]

switch.case.0:                                    ; preds = %0
  %1 = getelementptr inbounds %"Opt<int32>", ptr %a, i32 0, i32 1
  %tag6 = getelementptr inbounds %"Opt<int32>", ptr %enum2, i32 0, i32 0
  store i32 0, ptr %tag6, align 4
  %some.load = load i32, ptr %1, align 4
  %2 = sext i32 %some.load to i64
  %3 = add i64 %2, 1
  %4 = trunc i64 %3 to i32
  %5 = sext i32 %4 to i64
  %6 = icmp ne i64 %3, %5
  %7 = xor i1 %6, true
  %overflow.condition = icmp eq i1 %7, false
  br i1 %overflow.condition, label %overflow.fail, label %overflow.success

switch.case.1:                                    ; preds = %0
  %tag7 = getelementptr inbounds %"Opt<int32>", ptr %enum3, i32 0, i32 0
  store i32 1, ptr %tag7, align 4
  %enum.load8 = load %"Opt<int32>", ptr %enum3, align 4
  store %"Opt<int32>" %enum.load8, ptr %b, align 4
  br label %switch.end

switch.default:                                   ; preds = %0
  %8 = call %never @_CX1N3std10assertFailEM3std5never1_PKM3std4char(ptr @0), !dbg !7
  unreachable

switch.end:                                       ; preds = %overflow.success, %switch.case.1
  %b.tag = getelementptr inbounds %"Opt<int32>", ptr %b, i32 0, i32 0
  %b.tag.load = load i32, ptr %b.tag, align 4
  switch i32 %b.tag.load, label %switch.default14 [
    i32 0, label %switch.case.011
    i32 1, label %switch.case.113
  ]

overflow.fail:                                    ; preds = %switch.case.0
  %9 = call %never @_CX1N3std10assertFailEM3std5never1_PKM3std4char(ptr @1), !dbg !7
  unreachable

overflow.success:                                 ; preds = %switch.case.0
  %10 = insertvalue { i32 } undef, i32 %4, 0
  %associatedValue9 = getelementptr inbounds %"Opt<int32>", ptr %enum2, i32 0, i32 1
  store { i32 } %10, ptr %associatedValue9, align 4
  %enum.load10 = load %"Opt<int32>", ptr %enum2, align 4
  store %"Opt<int32>" %enum.load10, ptr %b, align 4
  br label %switch.end

switch.case.011:                                  ; preds = %switch.end
  %11 = getelementptr inbounds %"Opt<int32>", ptr %b, i32 0, i32 1
  %some.load12 = load i32, ptr %11, align 4
  call void @_CX1N3std10checkLeaksE4void0_(), !dbg !7
  ret i32 %some.load12

switch.case.113:                                  ; preds = %switch.end
  call void @_CX1N3std10checkLeaksE4void0_(), !dbg !7
  ret i32 0

switch.default14:                                 ; preds = %switch.end
  %12 = call %never @_CX1N3std10assertFailEM3std5never1_PKM3std4char(ptr @2), !dbg !7
  unreachable
}

declare %never @_CX1N3std10assertFailEM3std5never1_PKM3std4char(ptr) #0

declare void @_CX1N3std10checkLeaksE4void0_() #0

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "generic-enum.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 8, type: !5, scopeLine: 8, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 8, column: 5, scope: !4)
