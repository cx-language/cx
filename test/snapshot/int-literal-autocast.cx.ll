
@0 = private unnamed_addr constant [50 x i8] c"integer overflow at int-literal-autocast.cx:9:11\0A\00", align 1

declare void @foo(i8) #0

define i32 @main() #0 !dbg !4 {
  %b = alloca i64, align 8
  %c = alloca i8, align 1
  call void @foo(i8 1), !dbg !7
  store i64 42, ptr %b, align 8
  store i8 -42, ptr %c, align 1
  %b.load = load i64, ptr %b, align 8
  %1 = add i64 %b.load, 1
  %2 = icmp ult i64 %1, %b.load
  %3 = xor i1 %2, true
  %overflow.condition = icmp eq i1 %3, false
  br i1 %overflow.condition, label %overflow.fail, label %overflow.success

overflow.fail:                                    ; preds = %0
  call void @_EN3std10assertFailEP4char(ptr @0), !dbg !8
  unreachable

overflow.success:                                 ; preds = %0
  store i64 %1, ptr %b, align 8
  ret i32 0
}

declare void @_EN3std10assertFailEP4char(ptr) #0

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "int-literal-autocast.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 5, type: !5, scopeLine: 5, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 6, column: 5, scope: !4)
!8 = !DILocation(line: 5, column: 6, scope: !4)
