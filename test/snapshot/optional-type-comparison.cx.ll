
%0 = type { { i32 } }
%"Optional<int32>" = type { i32, %0 }
%never = type {}

@0 = private unnamed_addr constant [36 x i8] c"Unwrap failed at Optional.cx:17:16\0A\00", align 1

define i32 @main() #0 !dbg !4 {
  %a = alloca %"Optional<int32>", align 8
  %enum = alloca %"Optional<int32>", align 8
  %1 = alloca i32, align 4
  %tag = getelementptr inbounds %"Optional<int32>", ptr %enum, i32 0, i32 0
  store i32 0, ptr %tag, align 4
  %enum.load = load %"Optional<int32>", ptr %enum, align 4
  store %"Optional<int32>" %enum.load, ptr %a, align 4
  store i32 1, ptr %1, align 4
  %2 = call i1 @_CX1N3stdo2eqIM3std5int32EEM3std4bool2_ROM3std5int32RM3std5int32(ptr %a, ptr %1), !dbg !7
  ret i32 0
}

define i1 @_CX1N3stdo2eqIM3std5int32EEM3std4bool2_ROM3std5int32RM3std5int32(ptr %a, ptr %b) #0 !dbg !8 {
  %a1 = alloca ptr, align 8
  %b2 = alloca ptr, align 8
  store ptr %a, ptr %a1, align 8
  store ptr %b, ptr %b2, align 8
  %a.load = load ptr, ptr %a1, align 8
  %a.load.load = load %"Optional<int32>", ptr %a.load, align 4
  %1 = extractvalue %"Optional<int32>" %a.load.load, 0
  %2 = icmp eq i32 %1, 1
  %3 = xor i1 %2, true
  br i1 %3, label %if.then, label %if.else

if.then:                                          ; preds = %0
  ret i1 false

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else
  %a.load3 = load ptr, ptr %a1, align 8
  %a.load.load4 = load %"Optional<int32>", ptr %a.load3, align 4
  %4 = extractvalue %"Optional<int32>" %a.load.load4, 0
  %5 = icmp eq i32 %4, 1
  %assert.condition = icmp eq i1 %5, false
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %if.end
  %6 = call %never @_CX1N3std10assertFailEM3std5never1_PKM3std4char(ptr @0), !dbg !10
  unreachable

assert.success:                                   ; preds = %if.end
  %7 = getelementptr inbounds %"Optional<int32>", ptr %a.load3, i32 0, i32 1
  %.load = load i32, ptr %7, align 4
  %b.load = load ptr, ptr %b2, align 8
  %b.load.load = load i32, ptr %b.load, align 4
  %8 = icmp eq i32 %.load, %b.load.load
  ret i1 %8
}

declare %never @_CX1N3std10assertFailEM3std5never1_PKM3std4char(ptr) #0

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "optional-type-comparison.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 5, column: 7, scope: !4)
!8 = distinct !DISubprogram(name: "==", linkageName: "_CX1N3stdo2eqIM3std5int32EEM3std4bool2_ROM3std5int32RM3std5int32", scope: !9, file: !9, line: 15, type: !5, scopeLine: 15, spFlags: DISPFlagDefinition, unit: !2)
!9 = !DIFile(filename: "Optional.cx")
!10 = !DILocation(line: 15, column: 6, scope: !8)
