
%string = type { %"Slice<char>" }
%"Slice<char>" = type { ptr, i32 }

@0 = private unnamed_addr constant [4 x i8] c"bar\00", align 1

define i32 @main() #0 !dbg !4 {
  %b = alloca %string, align 8
  %__str = alloca %string, align 8
  %five = alloca i32, align 4
  call void @_CX1N4main3fooIM3std5int32EE4void1_M3std5int32(i32 1), !dbg !7
  call void @_CX1N4main3fooIM3std4boolEE4void1_M3std4bool(i1 false), !dbg !8
  call void @_CX1N4main3fooIM3std4boolEE4void1_M3std4bool(i1 true), !dbg !9
  call void @_CX1N3stdM3std6string4initE4void2_PM3std4charM3std5int32(ptr %__str, ptr @0, i32 3), !dbg !10
  %__str.load = load %string, ptr %__str, align 8
  %1 = call %string @_CX1N4main3barIM3std6stringEEM3std6string1_M3std6string(%string %__str.load), !dbg !11
  store %string %1, ptr %b, align 8
  %2 = call i32 @_CX1N4main3quxIM3std5int32EEM3std5int321_M3std5int32(i32 -5), !dbg !12
  store i32 %2, ptr %five, align 4
  call void @_CX1N3std10checkLeaksE4void0_(), !dbg !10
  ret i32 0
}

define void @_CX1N4main3fooIM3std5int32EE4void1_M3std5int32(i32 %t) #0 !dbg !13 {
  %t1 = alloca i32, align 4
  store i32 %t, ptr %t1, align 4
  ret void
}

define void @_CX1N4main3fooIM3std4boolEE4void1_M3std4bool(i1 %t) #0 !dbg !14 {
  %t1 = alloca i1, align 1
  store i1 %t, ptr %t1, align 1
  ret void
}

declare void @_CX1N3stdM3std6string4initE4void2_PM3std4charM3std5int32(ptr, ptr, i32) #0

define %string @_CX1N4main3barIM3std6stringEEM3std6string1_M3std6string(%string %t) #0 !dbg !15 {
  %t1 = alloca %string, align 8
  store %string %t, ptr %t1, align 8
  %t.load = load %string, ptr %t1, align 8
  ret %string %t.load
}

define i32 @_CX1N4main3quxIM3std5int32EEM3std5int321_M3std5int32(i32 %t) #0 !dbg !16 {
  %t1 = alloca i32, align 4
  store i32 %t, ptr %t1, align 4
  %t.load = load i32, ptr %t1, align 4
  %1 = icmp slt i32 %t.load, 0
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %0
  %t.load2 = load i32, ptr %t1, align 4
  %2 = sub i32 0, %t.load2
  ret i32 %2

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else
  %t.load3 = load i32, ptr %t1, align 4
  ret i32 %t.load3
}

declare void @_CX1N3std10checkLeaksE4void0_() #0

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "generic-function.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 12, type: !5, scopeLine: 12, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 13, column: 5, scope: !4)
!8 = !DILocation(line: 14, column: 5, scope: !4)
!9 = !DILocation(line: 15, column: 5, scope: !4)
!10 = !DILocation(line: 12, column: 6, scope: !4)
!11 = !DILocation(line: 16, column: 13, scope: !4)
!12 = !DILocation(line: 17, column: 16, scope: !4)
!13 = distinct !DISubprogram(name: "foo", linkageName: "_CX1N4main3fooIM3std5int32EE4void1_M3std5int32", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!14 = distinct !DISubprogram(name: "foo", linkageName: "_CX1N4main3fooIM3std4boolEE4void1_M3std4bool", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!15 = distinct !DISubprogram(name: "bar", linkageName: "_CX1N4main3barIM3std6stringEEM3std6string1_M3std6string", scope: !3, file: !3, line: 5, type: !5, scopeLine: 5, spFlags: DISPFlagDefinition, unit: !2)
!16 = distinct !DISubprogram(name: "qux", linkageName: "_CX1N4main3quxIM3std5int32EEM3std5int321_M3std5int32", scope: !3, file: !3, line: 7, type: !5, scopeLine: 7, spFlags: DISPFlagDefinition, unit: !2)
