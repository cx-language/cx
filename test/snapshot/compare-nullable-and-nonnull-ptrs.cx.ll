
define void @_EN4main1fEP5int32OP5int32(ptr %foo, ptr %bar) #0 !dbg !4 {
  %foo1 = alloca ptr, align 8
  %bar2 = alloca ptr, align 8
  store ptr %foo, ptr %foo1, align 8
  store ptr %bar, ptr %bar2, align 8
  %foo.load = load ptr, ptr %foo1, align 8
  %bar.load = load ptr, ptr %bar2, align 8
  %1 = icmp eq ptr %foo.load, %bar.load
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  br label %if.end

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "compare-nullable-and-nonnull-ptrs.cx")
!4 = distinct !DISubprogram(name: "f", linkageName: "_EN4main1fEP5int32OP5int32", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
