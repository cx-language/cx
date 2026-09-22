
%0 = type { { i32 } }
%"Optional<int>" = type { i32, %0 }

declare %"Optional<int>" @a() #0

declare %"Optional<int>" @b() #0

declare i32 @c() #0

declare ptr @p() #0

declare ptr @q() #0

define i32 @main() #0 !dbg !4 {
  %x = alloca i32, align 4
  %1 = alloca %"Optional<int>", align 8
  %y = alloca %"Optional<int>", align 8
  %z = alloca i32, align 4
  %2 = alloca %"Optional<int>", align 8
  %3 = alloca %"Optional<int>", align 8
  %w = alloca ptr, align 8
  %4 = call %"Optional<int>" @a(), !dbg !7
  %5 = extractvalue %"Optional<int>" %4, 0
  %6 = icmp eq i32 %5, 1
  br i1 %6, label %coalesce.value, label %coalesce.default

coalesce.value:                                   ; preds = %0
  store %"Optional<int>" %4, ptr %1, align 4
  %7 = getelementptr inbounds %"Optional<int>", ptr %1, i32 0, i32 1
  %.load = load i32, ptr %7, align 4
  br label %coalesce.end

coalesce.default:                                 ; preds = %0
  br label %coalesce.end

coalesce.end:                                     ; preds = %coalesce.default, %coalesce.value
  %coalesce = phi i32 [ %.load, %coalesce.value ], [ 0, %coalesce.default ]
  store i32 %coalesce, ptr %x, align 4
  %8 = call %"Optional<int>" @a(), !dbg !8
  %9 = extractvalue %"Optional<int>" %8, 0
  %10 = icmp eq i32 %9, 1
  br i1 %10, label %coalesce.value1, label %coalesce.default2

coalesce.value1:                                  ; preds = %coalesce.end
  br label %coalesce.end3

coalesce.default2:                                ; preds = %coalesce.end
  %11 = call %"Optional<int>" @b(), !dbg !9
  br label %coalesce.end3

coalesce.end3:                                    ; preds = %coalesce.default2, %coalesce.value1
  %coalesce4 = phi %"Optional<int>" [ %8, %coalesce.value1 ], [ %11, %coalesce.default2 ]
  store %"Optional<int>" %coalesce4, ptr %y, align 4
  %12 = call %"Optional<int>" @a(), !dbg !10
  %13 = extractvalue %"Optional<int>" %12, 0
  %14 = icmp eq i32 %13, 1
  br i1 %14, label %coalesce.value5, label %coalesce.default7

coalesce.value5:                                  ; preds = %coalesce.end3
  store %"Optional<int>" %12, ptr %2, align 4
  %15 = getelementptr inbounds %"Optional<int>", ptr %2, i32 0, i32 1
  %.load6 = load i32, ptr %15, align 4
  br label %coalesce.end13

coalesce.default7:                                ; preds = %coalesce.end3
  %16 = call %"Optional<int>" @b(), !dbg !11
  %17 = extractvalue %"Optional<int>" %16, 0
  %18 = icmp eq i32 %17, 1
  br i1 %18, label %coalesce.value8, label %coalesce.default10

coalesce.value8:                                  ; preds = %coalesce.default7
  store %"Optional<int>" %16, ptr %3, align 4
  %19 = getelementptr inbounds %"Optional<int>", ptr %3, i32 0, i32 1
  %.load9 = load i32, ptr %19, align 4
  br label %coalesce.end11

coalesce.default10:                               ; preds = %coalesce.default7
  %20 = call i32 @c(), !dbg !12
  br label %coalesce.end11

coalesce.end11:                                   ; preds = %coalesce.default10, %coalesce.value8
  %coalesce12 = phi i32 [ %.load9, %coalesce.value8 ], [ %20, %coalesce.default10 ]
  br label %coalesce.end13

coalesce.end13:                                   ; preds = %coalesce.end11, %coalesce.value5
  %coalesce14 = phi i32 [ %.load6, %coalesce.value5 ], [ %coalesce12, %coalesce.end11 ]
  store i32 %coalesce14, ptr %z, align 4
  %21 = call ptr @p(), !dbg !13
  %22 = icmp ne ptr %21, null
  br i1 %22, label %coalesce.value15, label %coalesce.default16

coalesce.value15:                                 ; preds = %coalesce.end13
  br label %coalesce.end17

coalesce.default16:                               ; preds = %coalesce.end13
  %23 = call ptr @q(), !dbg !14
  br label %coalesce.end17

coalesce.end17:                                   ; preds = %coalesce.default16, %coalesce.value15
  %coalesce18 = phi ptr [ %21, %coalesce.value15 ], [ %23, %coalesce.default16 ]
  store ptr %coalesce18, ptr %w, align 8
  ret i32 0
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "null-coalescing.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 9, type: !5, scopeLine: 9, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 10, column: 13, scope: !4)
!8 = !DILocation(line: 11, column: 13, scope: !4)
!9 = !DILocation(line: 11, column: 20, scope: !4)
!10 = !DILocation(line: 12, column: 13, scope: !4)
!11 = !DILocation(line: 12, column: 20, scope: !4)
!12 = !DILocation(line: 12, column: 27, scope: !4)
!13 = !DILocation(line: 13, column: 13, scope: !4)
!14 = !DILocation(line: 13, column: 20, scope: !4)
