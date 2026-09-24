
%"RangeIterator<int32>" = type { i32, i32 }
%"Range<int32>" = type { i32, i32 }

@0 = private unnamed_addr constant [54 x i8] c"integer overflow at exclusive-range-operator.cx:7:11\0A\00", align 1

define i32 @main() #0 !dbg !4 {
  %p = alloca i32, align 4
  %__iterator = alloca %"RangeIterator<int32>", align 8
  %1 = alloca %"Range<int32>", align 8
  %i = alloca i32, align 4
  store i32 9, ptr %p, align 4
  call void @_EN3std5RangeI5int32E4initE5int325int32(ptr %1, i32 0, i32 3), !dbg !7
  %2 = call %"RangeIterator<int32>" @_EN3std5RangeI5int32E8iteratorE(ptr %1), !dbg !8
  store %"RangeIterator<int32>" %2, ptr %__iterator, align 4
  br label %loop.condition

loop.condition:                                   ; preds = %loop.increment, %0
  %3 = call i1 @_EN3std13RangeIteratorI5int32E8hasValueE(ptr %__iterator), !dbg !8
  br i1 %3, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %4 = call i32 @_EN3std13RangeIteratorI5int32E5valueE(ptr %__iterator), !dbg !8
  store i32 %4, ptr %i, align 4
  %p.load = load i32, ptr %p, align 4
  %i.load = load i32, ptr %i, align 4
  %5 = sext i32 %p.load to i64
  %6 = sext i32 %i.load to i64
  %7 = sub i64 %5, %6
  %8 = trunc i64 %7 to i32
  %9 = sext i32 %8 to i64
  %10 = icmp ne i64 %7, %9
  %11 = xor i1 %10, true
  %overflow.condition = icmp eq i1 %11, false
  br i1 %overflow.condition, label %overflow.fail, label %overflow.success

loop.increment:                                   ; preds = %overflow.success
  call void @_EN3std13RangeIteratorI5int32E9incrementE(ptr %__iterator), !dbg !8
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  %p.load1 = load i32, ptr %p, align 4
  ret i32 %p.load1

overflow.fail:                                    ; preds = %loop.body
  call void @_EN3std10assertFailEP4char(ptr @0), !dbg !9
  unreachable

overflow.success:                                 ; preds = %loop.body
  store i32 %8, ptr %p, align 4
  br label %loop.increment
}

declare void @_EN3std5RangeI5int32E4initE5int325int32(ptr, i32, i32) #0

declare %"RangeIterator<int32>" @_EN3std5RangeI5int32E8iteratorE(ptr) #0

declare i1 @_EN3std13RangeIteratorI5int32E8hasValueE(ptr) #0

declare i32 @_EN3std13RangeIteratorI5int32E5valueE(ptr) #0

declare void @_EN3std13RangeIteratorI5int32E9incrementE(ptr) #0

declare void @_EN3std10assertFailEP4char(ptr) #0

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "exclusive-range-operator.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 6, column: 15, scope: !4)
!8 = !DILocation(line: 6, column: 5, scope: !4)
!9 = !DILocation(line: 3, column: 5, scope: !4)
