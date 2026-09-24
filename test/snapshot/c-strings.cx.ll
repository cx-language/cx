
%S = type { [1024 x i8] }
%string = type { %"Slice<char>" }
%"Slice<char>" = type { ptr, i32 }
%StringBuf = type { %"List<char>" }
%"List<char>" = type { ptr, i32, i32 }
%StringIterator = type { ptr, ptr }

@0 = private unnamed_addr constant [2 x i8] c"x\00", align 1
@1 = private unnamed_addr constant [2 x i8] c"x\00", align 1
@2 = private unnamed_addr constant [40 x i8] c"integer overflow at StringBuf.cx:20:47\0A\00", align 1

define i32 @main() #0 !dbg !4 {
  %s = alloca %S, align 8
  %__str = alloca %string, align 8
  %1 = alloca %StringBuf, align 8
  %__str1 = alloca %string, align 8
  %a = getelementptr inbounds %S, ptr %s, i32 0, i32 0
  call void @_EN3std6string4initEP4char3int(ptr %__str, ptr @0, i32 1), !dbg !7
  %__str.load = load %string, ptr %__str, align 8
  %2 = call i1 @_EN3stdeqEP4char6string(ptr %a, %string %__str.load), !dbg !8
  %a2 = getelementptr inbounds %S, ptr %s, i32 0, i32 0
  call void @_EN3std6string4initEP4char3int(ptr %__str1, ptr @1, i32 1), !dbg !7
  %__str.load3 = load %string, ptr %__str1, align 8
  call void @_EN3std9StringBuf4initE6string(ptr %1, %string %__str.load3), !dbg !9
  %3 = getelementptr inbounds %StringBuf, ptr %1, i32 0, i32 0
  %4 = getelementptr inbounds %"List<char>", ptr %3, i32 0, i32 0
  %.load = load ptr, ptr %4, align 8
  %5 = getelementptr inbounds %"List<char>", ptr %3, i32 0, i32 1
  %.load4 = load i32, ptr %5, align 4
  %6 = sub i32 %.load4, 1
  %7 = insertvalue %"Slice<char>" undef, ptr %.load, 0
  %8 = insertvalue %"Slice<char>" %7, i32 %6, 1
  %9 = insertvalue %string undef, %"Slice<char>" %8, 0
  %10 = call i1 @_EN3stdeqEP4char6string(ptr %a2, %string %9), !dbg !10
  %11 = xor i1 %10, true
  ret i32 0
}

declare void @_EN3std6string4initEP4char3int(ptr, ptr, i32) #0

declare i1 @_EN3stdeqEP4char6string(ptr, %string) #0

define void @_EN3std9StringBuf4initE6string(ptr %this, %string %s) #0 !dbg !11 {
  %s1 = alloca %string, align 8
  %1 = alloca %"List<char>", align 8
  %__iterator = alloca %StringIterator, align 8
  %ch = alloca ptr, align 8
  store %string %s, ptr %s1, align 8
  %characters = getelementptr inbounds %StringBuf, ptr %this, i32 0, i32 0
  %2 = call i32 @_EN3std6string4sizeE(ptr %s1), !dbg !13
  %3 = sext i32 %2 to i64
  %4 = add i64 %3, 1
  %5 = trunc i64 %4 to i32
  %6 = sext i32 %5 to i64
  %7 = icmp ne i64 %4, %6
  %8 = xor i1 %7, true
  %overflow.condition = icmp eq i1 %8, false
  br i1 %overflow.condition, label %overflow.fail, label %overflow.success

overflow.fail:                                    ; preds = %0
  call void @_EN3std10assertFailEP4char(ptr @2), !dbg !14
  unreachable

overflow.success:                                 ; preds = %0
  call void @_EN3std4ListI4charE4initE8capacity3int(ptr %1, i32 %5), !dbg !15
  %.load = load %"List<char>", ptr %1, align 8
  store %"List<char>" %.load, ptr %characters, align 8
  %9 = call %StringIterator @_EN3std6string8iteratorE(ptr %s1), !dbg !16
  store %StringIterator %9, ptr %__iterator, align 8
  br label %loop.condition

loop.condition:                                   ; preds = %loop.increment, %overflow.success
  %10 = call i1 @_EN3std14StringIterator8hasValueE(ptr %__iterator), !dbg !16
  br i1 %10, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %11 = call ptr @_EN3std14StringIterator5valueE(ptr %__iterator), !dbg !16
  store ptr %11, ptr %ch, align 8
  %characters2 = getelementptr inbounds %StringBuf, ptr %this, i32 0, i32 0
  %ch.load = load ptr, ptr %ch, align 8
  %ch.load.load = load i8, ptr %ch.load, align 1
  %12 = call ptr @_EN3std4ListI4charE4pushE4char(ptr %characters2, i8 %ch.load.load), !dbg !17
  br label %loop.increment

loop.increment:                                   ; preds = %loop.body
  call void @_EN3std14StringIterator9incrementE(ptr %__iterator), !dbg !16
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  %characters3 = getelementptr inbounds %StringBuf, ptr %this, i32 0, i32 0
  %13 = call ptr @_EN3std4ListI4charE4pushE4char(ptr %characters3, i8 0), !dbg !18
  ret void
}

declare i32 @_EN3std6string4sizeE(ptr) #0

declare void @_EN3std10assertFailEP4char(ptr) #0

define void @_EN3std4ListI4charE4initE8capacity3int(ptr %this, i32 %capacity) #0 !dbg !19 {
  %capacity1 = alloca i32, align 4
  store i32 %capacity, ptr %capacity1, align 4
  call void @_EN3std4ListI4charE4initE(ptr %this), !dbg !21
  %capacity.load = load i32, ptr %capacity1, align 4
  call void @_EN3std4ListI4charE7reserveE3int(ptr %this, i32 %capacity.load), !dbg !22
  ret void
}

declare %StringIterator @_EN3std6string8iteratorE(ptr) #0

declare i1 @_EN3std14StringIterator8hasValueE(ptr) #0

declare ptr @_EN3std14StringIterator5valueE(ptr) #0

declare ptr @_EN3std4ListI4charE4pushE4char(ptr, i8) #0

declare void @_EN3std14StringIterator9incrementE(ptr) #0

define void @_EN3std4ListI4charE4initE(ptr %this) #0 !dbg !23 {
  %size = getelementptr inbounds %"List<char>", ptr %this, i32 0, i32 1
  store i32 0, ptr %size, align 4
  %capacity = getelementptr inbounds %"List<char>", ptr %this, i32 0, i32 2
  store i32 0, ptr %capacity, align 4
  ret void
}

declare void @_EN3std4ListI4charE7reserveE3int(ptr, i32) #0

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "c-strings.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 7, type: !5, scopeLine: 7, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 7, column: 6, scope: !4)
!8 = !DILocation(line: 9, column: 9, scope: !4)
!9 = !DILocation(line: 10, column: 5, scope: !4)
!10 = !DILocation(line: 10, column: 20, scope: !4)
!11 = distinct !DISubprogram(name: "init", linkageName: "_EN3std9StringBuf4initE6string", scope: !12, file: !12, line: 19, type: !5, scopeLine: 19, spFlags: DISPFlagDefinition, unit: !2)
!12 = !DIFile(filename: "StringBuf.cx")
!13 = !DILocation(line: 20, column: 40, scope: !11)
!14 = !DILocation(line: 19, column: 5, scope: !11)
!15 = !DILocation(line: 20, column: 22, scope: !11)
!16 = !DILocation(line: 22, column: 9, scope: !11)
!17 = !DILocation(line: 23, column: 24, scope: !11)
!18 = !DILocation(line: 26, column: 20, scope: !11)
!19 = distinct !DISubprogram(name: "init", linkageName: "_EN3std4ListI4charE4initE8capacity3int", scope: !20, file: !20, line: 15, type: !5, scopeLine: 15, spFlags: DISPFlagDefinition, unit: !2)
!20 = !DIFile(filename: "List.cx")
!21 = !DILocation(line: 16, column: 9, scope: !19)
!22 = !DILocation(line: 17, column: 9, scope: !19)
!23 = distinct !DISubprogram(name: "init", linkageName: "_EN3std4ListI4charE4initE", scope: !20, file: !20, line: 8, type: !5, scopeLine: 8, spFlags: DISPFlagDefinition, unit: !2)
