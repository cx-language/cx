
%S = type <{ i8, i16, i8 }>

@0 = private unnamed_addr constant [43 x i8] c"Assertion failed at packed-struct.cx:11:5\0A\00", align 1

define i32 @main() #0 !dbg !4 {
  %s = alloca %S, align 8
  %a = getelementptr inbounds %S, ptr %s, i32 0, i32 0
  store i8 -85, ptr %a, align 1
  %b = getelementptr inbounds %S, ptr %s, i32 0, i32 1
  store i16 -12817, ptr %b, align 2
  %c = getelementptr inbounds %S, ptr %s, i32 0, i32 2
  store i8 0, ptr %c, align 1
  %1 = icmp eq i64 ptrtoint (ptr getelementptr (%S, ptr null, i32 1) to i64), 4
  %assert.condition = icmp eq i1 %1, false
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %0
  call void @_EN3std10assertFailEP4char(ptr @0), !dbg !7
  unreachable

assert.success:                                   ; preds = %0
  ret i32 0
}

declare void @_EN3std10assertFailEP4char(ptr) #0

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "packed-struct.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 6, type: !5, scopeLine: 6, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 6, column: 6, scope: !4)
