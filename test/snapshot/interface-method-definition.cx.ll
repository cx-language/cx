
%B = type {}
%never = type {}

@0 = private unnamed_addr constant [57 x i8] c"integer overflow at interface-method-definition.cx:5:26\0A\00", align 1

define i32 @main() #0 !dbg !4 {
  %b = alloca %B, align 8
  %x = alloca i32, align 4
  call void @_CX1N4mainM4main1B4initE4void0_(ptr %b), !dbg !7
  %1 = call i32 @_CX1N4mainM4main1B1gEM3std5int320_(ptr %b), !dbg !8
  store i32 %1, ptr %x, align 4
  ret i32 0
}

define void @_CX1N4mainM4main1B4initE4void0_(ptr %this) #0 !dbg !9 {
  ret void
}

define i32 @_CX1N4mainM4main1B1gEM3std5int320_(ptr %this) #0 !dbg !10 {
  %1 = call i32 @_CX1N4mainM4main1B1fEM3std5int320_(ptr %this), !dbg !11
  %2 = sext i32 %1 to i64
  %3 = mul i64 %2, 2
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
  ret i32 %4
}

define i32 @_CX1N4mainM4main1B1fEM3std5int320_(ptr %this) #0 !dbg !13 {
  ret i32 21
}

declare %never @_CX1N3std10assertFailEM3std5never1_PKM3std4char(ptr) #0

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "interface-method-definition.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 12, type: !5, scopeLine: 12, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 13, column: 13, scope: !4)
!8 = !DILocation(line: 14, column: 15, scope: !4)
!9 = distinct !DISubprogram(name: "init", linkageName: "_CX1N4mainM4main1B4initE4void0_", scope: !3, file: !3, line: 8, type: !5, scopeLine: 8, spFlags: DISPFlagDefinition, unit: !2)
!10 = distinct !DISubprogram(name: "g", linkageName: "_CX1N4mainM4main1B1gEM3std5int320_", scope: !3, file: !3, line: 5, type: !5, scopeLine: 5, spFlags: DISPFlagDefinition, unit: !2)
!11 = !DILocation(line: 5, column: 22, scope: !10)
!12 = !DILocation(line: 5, column: 9, scope: !10)
!13 = distinct !DISubprogram(name: "f", linkageName: "_CX1N4mainM4main1B1fEM3std5int320_", scope: !3, file: !3, line: 9, type: !5, scopeLine: 9, spFlags: DISPFlagDefinition, unit: !2)
