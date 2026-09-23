
@0 = private unnamed_addr constant [33 x i8] c"Unwrap failed at unwrap.cx:6:37\0A\00", align 1
@1 = private unnamed_addr constant [32 x i8] c"Unwrap failed at unwrap.cx:8:8\0A\00", align 1

declare ptr @f() #0

define i32 @main() #0 !dbg !4 {
  %byte = alloca ptr, align 8
  %ptr = alloca ptr, align 8
  %1 = call ptr @f(), !dbg !7
  %assert.condition = icmp eq ptr %1, null
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %0
  call void @_EN3std10assertFailEP4char(ptr @0), !dbg !8
  unreachable

assert.success:                                   ; preds = %0
  store ptr %1, ptr %byte, align 8
  %2 = call ptr @f(), !dbg !9
  store ptr %2, ptr %ptr, align 8
  %ptr.load = load ptr, ptr %ptr, align 8
  %assert.condition1 = icmp eq ptr %ptr.load, null
  br i1 %assert.condition1, label %assert.fail2, label %assert.success3

assert.fail2:                                     ; preds = %assert.success
  call void @_EN3std10assertFailEP4char(ptr @1), !dbg !8
  unreachable

assert.success3:                                  ; preds = %assert.success
  %3 = getelementptr inbounds [1 x i8], ptr %ptr.load, i32 0, i32 0
  store i8 1, ptr %3, align 1
  ret i32 0
}

declare void @_EN3std10assertFailEP4char(ptr) #0

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "unwrap.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 5, type: !5, scopeLine: 5, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 6, column: 33, scope: !4)
!8 = !DILocation(line: 5, column: 6, scope: !4)
!9 = !DILocation(line: 7, column: 39, scope: !4)
