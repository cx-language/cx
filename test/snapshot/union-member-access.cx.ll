
%U = type { i32 }

@0 = private unnamed_addr constant [50 x i8] c"integer overflow at union-member-access.cx:13:19\0A\00", align 1

define i32 @_EN4main3fooE1U(%U %u) #0 !dbg !4 {
  %u1 = alloca %U, align 8
  store %U %u, ptr %u1, align 4
  %b.load = load i32, ptr %u1, align 4
  ret i32 %b.load
}

define i32 @main() #0 !dbg !7 {
  %u = alloca %U, align 8
  store i32 21, ptr %u, align 4
  %u.load = load %U, ptr %u, align 4
  %1 = call i32 @_EN4main3fooE1U(%U %u.load), !dbg !8
  %b.load = load i32, ptr %u, align 4
  %2 = sext i32 %1 to i64
  %3 = sext i32 %b.load to i64
  %4 = add i64 %2, %3
  %5 = trunc i64 %4 to i32
  %6 = sext i32 %5 to i64
  %7 = icmp ne i64 %4, %6
  %8 = xor i1 %7, true
  %overflow.condition = icmp eq i1 %8, false
  br i1 %overflow.condition, label %overflow.fail, label %overflow.success

overflow.fail:                                    ; preds = %0
  call void @_EN3std10assertFailEP4char(ptr @0), !dbg !9
  unreachable

overflow.success:                                 ; preds = %0
  ret i32 %5
}

declare void @_EN3std10assertFailEP4char(ptr) #0

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "union-member-access.cx")
!4 = distinct !DISubprogram(name: "foo", linkageName: "_EN4main3fooE1U", scope: !3, file: !3, line: 6, type: !5, scopeLine: 6, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 10, type: !5, scopeLine: 10, spFlags: DISPFlagDefinition, unit: !2)
!8 = !DILocation(line: 13, column: 12, scope: !7)
!9 = !DILocation(line: 10, column: 5, scope: !7)
