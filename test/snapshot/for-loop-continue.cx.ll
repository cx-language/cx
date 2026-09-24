
%StringIterator = type { ptr, ptr }
%string = type { %"Slice<char>" }
%"Slice<char>" = type { ptr, i32 }

@0 = private unnamed_addr constant [4 x i8] c"abc\00", align 1

define i32 @main() #0 !dbg !4 {
  %__iterator = alloca %StringIterator, align 8
  %__str = alloca %string, align 8
  %ch = alloca ptr, align 8
  call void @_EN3std6string4initEP4char5int32(ptr %__str, ptr @0, i32 3), !dbg !7
  %1 = call %StringIterator @_EN3std6string8iteratorE(ptr %__str), !dbg !8
  store %StringIterator %1, ptr %__iterator, align 8
  br label %loop.condition

loop.condition:                                   ; preds = %loop.increment, %0
  %2 = call i1 @_EN3std14StringIterator8hasValueE(ptr %__iterator), !dbg !8
  br i1 %2, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %3 = call ptr @_EN3std14StringIterator5valueE(ptr %__iterator), !dbg !8
  store ptr %3, ptr %ch, align 8
  %ch.load = load ptr, ptr %ch, align 8
  %ch.load.load = load i8, ptr %ch.load, align 1
  %4 = icmp eq i8 %ch.load.load, 98
  br i1 %4, label %if.then, label %if.else

loop.increment:                                   ; preds = %if.end, %if.then
  call void @_EN3std14StringIterator9incrementE(ptr %__iterator), !dbg !8
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  ret i32 0

if.then:                                          ; preds = %loop.body
  br label %loop.increment

if.else:                                          ; preds = %loop.body
  br label %if.end

if.end:                                           ; preds = %if.else
  br label %loop.increment
}

declare void @_EN3std6string4initEP4char5int32(ptr, ptr, i32) #0

declare %StringIterator @_EN3std6string8iteratorE(ptr) #0

declare i1 @_EN3std14StringIterator8hasValueE(ptr) #0

declare ptr @_EN3std14StringIterator5valueE(ptr) #0

declare void @_EN3std14StringIterator9incrementE(ptr) #0

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "for-loop-continue.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 3, column: 6, scope: !4)
!8 = !DILocation(line: 4, column: 5, scope: !4)
