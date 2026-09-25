
%"X<int32>" = type { ptr }
%never = type {}
%"Y<int32>" = type { i32 }

@0 = private unnamed_addr constant [64 x i8] c"Unwrap failed at optional-type-unknown-identifier-bug.cx:11:18\0A\00", align 1

define i32 @main() #0 !dbg !4 {
  %x = alloca %"X<int32>", align 8
  call void @_CX1N4mainM4main1XIM3std5int32E4initE4void0_(ptr %x), !dbg !7
  call void @_CX1N4mainM4main1XIM3std5int32E6deinitE4void0_(ptr %x), !dbg !8
  ret i32 0
}

define void @_CX1N4mainM4main1XIM3std5int32E4initE4void0_(ptr %this) #0 !dbg !9 {
  %y = getelementptr inbounds %"X<int32>", ptr %this, i32 0, i32 0
  store ptr null, ptr %y, align 8
  ret void
}

define void @_CX1N4mainM4main1XIM3std5int32E6deinitE4void0_(ptr %this) #0 !dbg !10 {
  %a = alloca i32, align 4
  %y = getelementptr inbounds %"X<int32>", ptr %this, i32 0, i32 0
  %y.load = load ptr, ptr %y, align 8
  %assert.condition = icmp eq ptr %y.load, null
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %0
  %1 = call %never @_CX1N3std10assertFailEM3std5never1_PKM3std4char(ptr @0), !dbg !11
  unreachable

assert.success:                                   ; preds = %0
  %y.load1 = load ptr, ptr %y, align 8
  %a2 = getelementptr inbounds %"Y<int32>", ptr %y.load1, i32 0, i32 0
  %a.load = load i32, ptr %a2, align 4
  store i32 %a.load, ptr %a, align 4
  ret void
}

declare %never @_CX1N3std10assertFailEM3std5never1_PKM3std4char(ptr) #0

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "optional-type-unknown-identifier-bug.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 21, type: !5, scopeLine: 21, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 22, column: 13, scope: !4)
!8 = !DILocation(line: 21, column: 6, scope: !4)
!9 = distinct !DISubprogram(name: "init", linkageName: "_CX1N4mainM4main1XIM3std5int32E4initE4void0_", scope: !3, file: !3, line: 6, type: !5, scopeLine: 6, spFlags: DISPFlagDefinition, unit: !2)
!10 = distinct !DISubprogram(name: "deinit", linkageName: "_CX1N4mainM4main1XIM3std5int32E6deinitE4void0_", scope: !3, file: !3, line: 10, type: !5, scopeLine: 10, spFlags: DISPFlagDefinition, unit: !2)
!11 = !DILocation(line: 10, column: 5, scope: !10)
