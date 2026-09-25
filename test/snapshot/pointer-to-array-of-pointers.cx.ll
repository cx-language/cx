
%never = type {}

@0 = private unnamed_addr constant [55 x i8] c"Unwrap failed at pointer-to-array-of-pointers.cx:8:21\0A\00", align 1
@1 = private unnamed_addr constant [4 x i8] c"%s\0A\00", align 1

declare ptr @b() #0

define i32 @main() #0 !dbg !4 {
  %s = alloca ptr, align 8
  %i = alloca i32, align 4
  %1 = call ptr @b(), !dbg !7
  store ptr %1, ptr %s, align 8
  store i32 0, ptr %i, align 4
  %s.load = load ptr, ptr %s, align 8
  %assert.condition = icmp eq ptr %s.load, null
  br i1 %assert.condition, label %assert.fail, label %assert.success

assert.fail:                                      ; preds = %0
  %2 = call %never @_CX1N3std10assertFailEM3std5never1_PKM3std4char(ptr @0), !dbg !8
  unreachable

assert.success:                                   ; preds = %0
  %s.load1 = load ptr, ptr %s, align 8
  %i.load = load i32, ptr %i, align 4
  %3 = getelementptr inbounds ptr, ptr %s.load1, i32 %i.load
  %.load = load ptr, ptr %3, align 8
  %4 = call i32 (ptr, ...) @printf(ptr @1, ptr %.load), !dbg !9
  call void @_CX1N3std10checkLeaksE4void0_(), !dbg !8
  ret i32 0
}

declare %never @_CX1N3std10assertFailEM3std5never1_PKM3std4char(ptr) #0

declare i32 @printf(ptr, ...) #0

declare void @_CX1N3std10checkLeaksE4void0_() #0

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "pointer-to-array-of-pointers.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 5, type: !5, scopeLine: 5, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 6, column: 13, scope: !4)
!8 = !DILocation(line: 5, column: 6, scope: !4)
!9 = !DILocation(line: 8, column: 5, scope: !4)
