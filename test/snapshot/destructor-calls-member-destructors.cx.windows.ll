
%A = type { %B, %B, i32 }
%B = type { i32 }
%C = type { %B, %B }

declare i1 @f() #0

define i32 @main() #0 !dbg !4 {
  %1 = call ptr @_set_thread_local_invalid_parameter_handler(ptr @__cx_noop_invalid_parameter_handler), !dbg !7
  %a = alloca %A, align 8
  %c = alloca %C, align 8
  %2 = call ptr @_CX1N3std11ambientRootERM3std7Context0_(ptr null), !dbg !7
  call void @_CX1N4mainM4main1A4initE4void0_(ptr %2, ptr %a), !dbg !8
  call void @_CX1N4mainM4main1C4initE4void0_(ptr %2, ptr %c), !dbg !9
  call void @_CX1N4mainM4main1C6deinitE4void0_(ptr %2, ptr %c), !dbg !7
  call void @_CX1N4mainM4main1A6deinitE4void0_(ptr %2, ptr %a), !dbg !7
  call void @_CX1N3std10checkLeaksE4void0_(ptr %2), !dbg !7
  ret i32 0
}

define private void @__cx_noop_invalid_parameter_handler(ptr %0, ptr %1, ptr %2, i32 %3, i64 %4) {
  ret void
}

declare ptr @_set_thread_local_invalid_parameter_handler(ptr)

declare ptr @_CX1N3std11ambientRootERM3std7Context0_(ptr) #0

define void @_CX1N4mainM4main1A4initE4void0_(ptr %__context, ptr %this) #0 !dbg !10 {
  %this1 = alloca ptr, align 8
  store ptr %this, ptr %this1, align 8
  ret void
}

define void @_CX1N4mainM4main1C4initE4void0_(ptr %__context, ptr %this) #0 !dbg !11 {
  %this1 = alloca ptr, align 8
  store ptr %this, ptr %this1, align 8
  ret void
}

define void @_CX1N4mainM4main1C6deinitE4void0_(ptr %__context, ptr %this) #0 !dbg !12 {
  %this1 = alloca ptr, align 8
  store ptr %this, ptr %this1, align 8
  %b = getelementptr inbounds %C, ptr %this, i32 0, i32 0
  %bb = getelementptr inbounds %C, ptr %this, i32 0, i32 1
  call void @_CX1N4mainM4main1B6deinitE4void0_(ptr %__context, ptr %bb), !dbg !13
  call void @_CX1N4mainM4main1B6deinitE4void0_(ptr %__context, ptr %b), !dbg !13
  ret void
}

define void @_CX1N4mainM4main1A6deinitE4void0_(ptr %__context, ptr %this) #0 !dbg !14 {
  %this1 = alloca ptr, align 8
  store ptr %this, ptr %this1, align 8
  %b = getelementptr inbounds %A, ptr %this, i32 0, i32 0
  %bb = getelementptr inbounds %A, ptr %this, i32 0, i32 1
  %1 = call i1 @f(), !dbg !15
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  call void @_CX1N4mainM4main1B6deinitE4void0_(ptr %__context, ptr %bb), !dbg !16
  call void @_CX1N4mainM4main1B6deinitE4void0_(ptr %__context, ptr %b), !dbg !16
  ret void

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else
  call void @_CX1N4mainM4main1B6deinitE4void0_(ptr %__context, ptr %bb), !dbg !16
  call void @_CX1N4mainM4main1B6deinitE4void0_(ptr %__context, ptr %b), !dbg !16
  ret void
}

declare void @_CX1N3std10checkLeaksE4void0_(ptr) #0

define void @_CX1N4mainM4main1B6deinitE4void0_(ptr %__context, ptr %this) #0 !dbg !17 {
  %this1 = alloca ptr, align 8
  store ptr %this, ptr %this1, align 8
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"DEBUG-FORMAT", i32 0}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "destructor-calls-member-destructors.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 38, type: !5, scopeLine: 38, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 38, column: 6, scope: !4)
!8 = !DILocation(line: 39, column: 13, scope: !4)
!9 = !DILocation(line: 40, column: 13, scope: !4)
!10 = distinct !DISubprogram(name: "init", linkageName: "_CX1N4mainM4main1A4initE4void0_", scope: !3, file: !3, line: 10, type: !5, scopeLine: 10, spFlags: DISPFlagDefinition, unit: !2)
!11 = distinct !DISubprogram(name: "init", linkageName: "_CX1N4mainM4main1C4initE4void0_", scope: !3, file: !3, line: 32, type: !5, scopeLine: 32, spFlags: DISPFlagDefinition, unit: !2)
!12 = distinct !DISubprogram(name: "deinit", linkageName: "_CX1N4mainM4main1C6deinitE4void0_", scope: !3, file: !3, line: 29, type: !5, scopeLine: 29, spFlags: DISPFlagDefinition, unit: !2)
!13 = !DILocation(line: 29, column: 8, scope: !12)
!14 = distinct !DISubprogram(name: "deinit", linkageName: "_CX1N4mainM4main1A6deinitE4void0_", scope: !3, file: !3, line: 16, type: !5, scopeLine: 16, spFlags: DISPFlagDefinition, unit: !2)
!15 = !DILocation(line: 17, column: 13, scope: !14)
!16 = !DILocation(line: 16, column: 5, scope: !14)
!17 = distinct !DISubprogram(name: "deinit", linkageName: "_CX1N4mainM4main1B6deinitE4void0_", scope: !3, file: !3, line: 26, type: !5, scopeLine: 26, spFlags: DISPFlagDefinition, unit: !2)
