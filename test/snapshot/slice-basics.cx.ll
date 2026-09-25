
%"Slice<int32>" = type { ptr, i32 }
%string = type { %"Slice<char>" }
%"Slice<char>" = type { ptr, i32 }
%never = type {}
%OutputFileStream = type { ptr, i1 }

@0 = private unnamed_addr constant [11 x i8] c"operator[]\00", align 1
@1 = private unnamed_addr constant [7 x i8] c"Slice.\00", align 1
@2 = private unnamed_addr constant [9 x i8] c": index \00", align 1
@3 = private unnamed_addr constant [28 x i8] c" is out of bounds, size is \00", align 1

define i32 @main() #0 !dbg !4 {
  %three = alloca [3 x i32], align 4
  %b = alloca i32, align 4
  %ref = alloca %"Slice<int32>", align 8
  %bb = alloca i32, align 4
  store [3 x i32] [i32 0, i32 42, i32 0], ptr %three, align 4
  %1 = getelementptr inbounds [3 x i32], ptr %three, i32 0, i32 0
  %2 = insertvalue %"Slice<int32>" undef, ptr %1, 0
  %3 = insertvalue %"Slice<int32>" %2, i32 3, 1
  call void @_CX1N4main3fooE4void1_M3std5SliceIM3std5int32E(%"Slice<int32>" %3), !dbg !7
  call void @_CX1N4main3barE4void1_PM3std5ArrayIM3std5int32N3_E(ptr %three), !dbg !8
  store i32 3, ptr %b, align 4
  %4 = getelementptr inbounds [3 x i32], ptr %three, i32 0, i32 0
  %5 = insertvalue %"Slice<int32>" undef, ptr %4, 0
  %6 = insertvalue %"Slice<int32>" %5, i32 3, 1
  store %"Slice<int32>" %6, ptr %ref, align 8
  %7 = call i32 @_CX1N3stdM3std5SliceIM3std5int32E4sizeEM3std5int320_(ptr %ref), !dbg !9
  store i32 %7, ptr %bb, align 4
  call void @_CX1N3std10checkLeaksE4void0_(), !dbg !10
  ret i32 0
}

define void @_CX1N4main3fooE4void1_M3std5SliceIM3std5int32E(%"Slice<int32>" %ints) #0 !dbg !11 {
  %ints1 = alloca %"Slice<int32>", align 8
  %a = alloca i32, align 4
  %b = alloca i32, align 4
  %c = alloca %"Slice<int32>", align 8
  store %"Slice<int32>" %ints, ptr %ints1, align 8
  %1 = call ptr @_CX1N3stdM3std5SliceIM3std5int32Eo2ixERM3std5int321_M3std5int32(ptr %ints1, i32 1), !dbg !12
  %.load = load i32, ptr %1, align 4
  store i32 %.load, ptr %a, align 4
  %2 = call i32 @_CX1N3stdM3std5SliceIM3std5int32E4sizeEM3std5int320_(ptr %ints1), !dbg !13
  store i32 %2, ptr %b, align 4
  %ints.load = load %"Slice<int32>", ptr %ints1, align 8
  store %"Slice<int32>" %ints.load, ptr %c, align 8
  ret void
}

define void @_CX1N4main3barE4void1_PM3std5ArrayIM3std5int32N3_E(ptr %ints) #0 !dbg !14 {
  %ints1 = alloca ptr, align 8
  %b = alloca i32, align 4
  %ref = alloca %"Slice<int32>", align 8
  store ptr %ints, ptr %ints1, align 8
  store i32 3, ptr %b, align 4
  %ints.load = load ptr, ptr %ints1, align 8
  %1 = getelementptr inbounds [3 x i32], ptr %ints.load, i32 0, i32 0
  %2 = insertvalue %"Slice<int32>" undef, ptr %1, 0
  %3 = insertvalue %"Slice<int32>" %2, i32 3, 1
  store %"Slice<int32>" %3, ptr %ref, align 8
  ret void
}

define i32 @_CX1N3stdM3std5SliceIM3std5int32E4sizeEM3std5int320_(ptr %this) #0 !dbg !15 {
  %size = getelementptr inbounds %"Slice<int32>", ptr %this, i32 0, i32 1
  %size.load = load i32, ptr %size, align 4
  ret i32 %size.load
}

declare void @_CX1N3std10checkLeaksE4void0_() #0

define ptr @_CX1N3stdM3std5SliceIM3std5int32Eo2ixERM3std5int321_M3std5int32(ptr %this, i32 %index) #0 !dbg !17 {
  %index1 = alloca i32, align 4
  %__str = alloca %string, align 8
  store i32 %index, ptr %index1, align 4
  %index.load = load i32, ptr %index1, align 4
  %1 = icmp slt i32 %index.load, 0
  br i1 %1, label %or.end, label %or.rhs

or.rhs:                                           ; preds = %0
  %index.load2 = load i32, ptr %index1, align 4
  %2 = call i32 @_CX1N3stdM3std5SliceIM3std5int32E4sizeEM3std5int320_(ptr %this), !dbg !18
  %3 = icmp sge i32 %index.load2, %2
  br label %or.end

or.end:                                           ; preds = %or.rhs, %0
  %or = phi i1 [ %1, %0 ], [ %3, %or.rhs ]
  br i1 %or, label %if.then, label %if.else

if.then:                                          ; preds = %or.end
  call void @_CX1N3stdM3std6string4initE4void2_PM3std4charM3std5int32(ptr %__str, ptr @0, i32 10), !dbg !19
  %__str.load = load %string, ptr %__str, align 8
  %index.load3 = load i32, ptr %index1, align 4
  call void @_CX1N3stdM3std5SliceIM3std5int32E16indexOutOfBoundsE4void2_M3std6stringM3std5int32(ptr %this, %string %__str.load, i32 %index.load3), !dbg !20
  br label %if.end

if.else:                                          ; preds = %or.end
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %data = getelementptr inbounds %"Slice<int32>", ptr %this, i32 0, i32 0
  %data.load = load ptr, ptr %data, align 8
  %index.load4 = load i32, ptr %index1, align 4
  %4 = getelementptr inbounds i32, ptr %data.load, i32 %index.load4
  ret ptr %4
}

declare void @_CX1N3stdM3std6string4initE4void2_PM3std4charM3std5int32(ptr, ptr, i32) #0

define void @_CX1N3stdM3std5SliceIM3std5int32E16indexOutOfBoundsE4void2_M3std6stringM3std5int32(ptr %this, %string %function, i32 %index) #0 !dbg !21 {
  %function1 = alloca %string, align 8
  %index2 = alloca i32, align 4
  %__str = alloca %string, align 8
  %__str3 = alloca %string, align 8
  %__str4 = alloca %string, align 8
  store %string %function, ptr %function1, align 8
  store i32 %index, ptr %index2, align 4
  call void @_CX1N3stdM3std6string4initE4void2_PM3std4charM3std5int32(ptr %__str, ptr @1, i32 6), !dbg !22
  %__str.load = load %string, ptr %__str, align 8
  %function.load = load %string, ptr %function1, align 8
  call void @_CX1N3stdM3std6string4initE4void2_PM3std4charM3std5int32(ptr %__str3, ptr @2, i32 8), !dbg !22
  %__str.load5 = load %string, ptr %__str3, align 8
  %index.load = load i32, ptr %index2, align 4
  call void @_CX1N3stdM3std6string4initE4void2_PM3std4charM3std5int32(ptr %__str4, ptr @3, i32 27), !dbg !22
  %__str.load6 = load %string, ptr %__str4, align 8
  %1 = call i32 @_CX1N3stdM3std5SliceIM3std5int32E4sizeEM3std5int320_(ptr %this), !dbg !23
  %2 = call %never @_CX1N3std5abortIM3std6stringM3std6stringM3std6stringM3std5int32M3std6stringM3std5int32EVEM3std5never6_M3std6stringM3std6stringM3std6stringM3std5int32M3std6stringM3std5int32(%string %__str.load, %string %function.load, %string %__str.load5, i32 %index.load, %string %__str.load6, i32 %1), !dbg !24
  ret void
}

define %never @_CX1N3std5abortIM3std6stringM3std6stringM3std6stringM3std5int32M3std6stringM3std5int32EVEM3std5never6_M3std6stringM3std6stringM3std6stringM3std5int32M3std6stringM3std5int32(%string %args_0, %string %args_1, %string %args_2, i32 %args_3, %string %args_4, i32 %args_5) #0 !dbg !25 {
  %args_01 = alloca %string, align 8
  %args_12 = alloca %string, align 8
  %args_23 = alloca %string, align 8
  %args_34 = alloca i32, align 4
  %args_45 = alloca %string, align 8
  %args_56 = alloca i32, align 4
  %arg = alloca %string, align 8
  %1 = alloca %OutputFileStream, align 8
  %arg7 = alloca %string, align 8
  %2 = alloca %OutputFileStream, align 8
  %arg8 = alloca %string, align 8
  %3 = alloca %OutputFileStream, align 8
  %arg9 = alloca i32, align 4
  %4 = alloca %OutputFileStream, align 8
  %arg10 = alloca %string, align 8
  %5 = alloca %OutputFileStream, align 8
  %arg11 = alloca i32, align 4
  %6 = alloca %OutputFileStream, align 8
  %7 = alloca %OutputFileStream, align 8
  %8 = alloca i8, align 1
  store %string %args_0, ptr %args_01, align 8
  store %string %args_1, ptr %args_12, align 8
  store %string %args_2, ptr %args_23, align 8
  store i32 %args_3, ptr %args_34, align 4
  store %string %args_4, ptr %args_45, align 8
  store i32 %args_5, ptr %args_56, align 4
  %args_0.load = load %string, ptr %args_01, align 8
  store %string %args_0.load, ptr %arg, align 8
  %9 = call %OutputFileStream @_CX1N3std6stderrEM3std16OutputFileStream0_(), !dbg !27
  store %OutputFileStream %9, ptr %1, align 8
  call void @_CX1N3stdM3std16OutputFileStream5writeIM3std6stringEE4void1_RM3std6string(ptr %1, ptr %arg), !dbg !28
  %args_1.load = load %string, ptr %args_12, align 8
  store %string %args_1.load, ptr %arg7, align 8
  %10 = call %OutputFileStream @_CX1N3std6stderrEM3std16OutputFileStream0_(), !dbg !27
  store %OutputFileStream %10, ptr %2, align 8
  call void @_CX1N3stdM3std16OutputFileStream5writeIM3std6stringEE4void1_RM3std6string(ptr %2, ptr %arg7), !dbg !28
  %args_2.load = load %string, ptr %args_23, align 8
  store %string %args_2.load, ptr %arg8, align 8
  %11 = call %OutputFileStream @_CX1N3std6stderrEM3std16OutputFileStream0_(), !dbg !27
  store %OutputFileStream %11, ptr %3, align 8
  call void @_CX1N3stdM3std16OutputFileStream5writeIM3std6stringEE4void1_RM3std6string(ptr %3, ptr %arg8), !dbg !28
  %args_3.load = load i32, ptr %args_34, align 4
  store i32 %args_3.load, ptr %arg9, align 4
  %12 = call %OutputFileStream @_CX1N3std6stderrEM3std16OutputFileStream0_(), !dbg !27
  store %OutputFileStream %12, ptr %4, align 8
  call void @_CX1N3stdM3std16OutputFileStream5writeIM3std5int32EE4void1_RM3std5int32(ptr %4, ptr %arg9), !dbg !28
  %args_4.load = load %string, ptr %args_45, align 8
  store %string %args_4.load, ptr %arg10, align 8
  %13 = call %OutputFileStream @_CX1N3std6stderrEM3std16OutputFileStream0_(), !dbg !27
  store %OutputFileStream %13, ptr %5, align 8
  call void @_CX1N3stdM3std16OutputFileStream5writeIM3std6stringEE4void1_RM3std6string(ptr %5, ptr %arg10), !dbg !28
  %args_5.load = load i32, ptr %args_56, align 4
  store i32 %args_5.load, ptr %arg11, align 4
  %14 = call %OutputFileStream @_CX1N3std6stderrEM3std16OutputFileStream0_(), !dbg !27
  store %OutputFileStream %14, ptr %6, align 8
  call void @_CX1N3stdM3std16OutputFileStream5writeIM3std5int32EE4void1_RM3std5int32(ptr %6, ptr %arg11), !dbg !28
  %15 = call %OutputFileStream @_CX1N3std6stderrEM3std16OutputFileStream0_(), !dbg !29
  store %OutputFileStream %15, ptr %7, align 8
  store i8 10, ptr %8, align 1
  call void @_CX1N3stdM3std16OutputFileStream5writeIM3std4charEE4void1_RM3std4char(ptr %7, ptr %8), !dbg !30
  %16 = call %never @_CX1N3std12abortWrapperEM3std5never0_(), !dbg !31
  unreachable
}

declare %OutputFileStream @_CX1N3std6stderrEM3std16OutputFileStream0_() #0

declare void @_CX1N3stdM3std16OutputFileStream5writeIM3std6stringEE4void1_RM3std6string(ptr, ptr) #0

declare void @_CX1N3stdM3std16OutputFileStream5writeIM3std5int32EE4void1_RM3std5int32(ptr, ptr) #0

declare void @_CX1N3stdM3std16OutputFileStream5writeIM3std4charEE4void1_RM3std4char(ptr, ptr) #0

declare %never @_CX1N3std12abortWrapperEM3std5never0_() #0

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "slice-basics.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 5, column: 5, scope: !4)
!8 = !DILocation(line: 6, column: 5, scope: !4)
!9 = !DILocation(line: 9, column: 18, scope: !4)
!10 = !DILocation(line: 3, column: 6, scope: !4)
!11 = distinct !DISubprogram(name: "foo", linkageName: "_CX1N4main3fooE4void1_M3std5SliceIM3std5int32E", scope: !3, file: !3, line: 12, type: !5, scopeLine: 12, spFlags: DISPFlagDefinition, unit: !2)
!12 = !DILocation(line: 13, column: 17, scope: !11)
!13 = !DILocation(line: 14, column: 18, scope: !11)
!14 = distinct !DISubprogram(name: "bar", linkageName: "_CX1N4main3barE4void1_PM3std5ArrayIM3std5int32N3_E", scope: !3, file: !3, line: 18, type: !5, scopeLine: 18, spFlags: DISPFlagDefinition, unit: !2)
!15 = distinct !DISubprogram(name: "size", linkageName: "_CX1N3stdM3std5SliceIM3std5int32E4sizeEM3std5int320_", scope: !16, file: !16, line: 31, type: !5, scopeLine: 31, spFlags: DISPFlagDefinition, unit: !2)
!16 = !DIFile(filename: "Slice.cx")
!17 = distinct !DISubprogram(name: "[]", linkageName: "_CX1N3stdM3std5SliceIM3std5int32Eo2ixERM3std5int321_M3std5int32", scope: !16, file: !16, line: 63, type: !5, scopeLine: 63, spFlags: DISPFlagDefinition, unit: !2)
!18 = !DILocation(line: 64, column: 34, scope: !17)
!19 = !DILocation(line: 63, column: 14, scope: !17)
!20 = !DILocation(line: 64, column: 41, scope: !17)
!21 = distinct !DISubprogram(name: "indexOutOfBounds", linkageName: "_CX1N3stdM3std5SliceIM3std5int32E16indexOutOfBoundsE4void2_M3std6stringM3std5int32", scope: !16, file: !16, line: 151, type: !5, scopeLine: 151, spFlags: DISPFlagDefinition, unit: !2)
!22 = !DILocation(line: 151, column: 18, scope: !21)
!23 = !DILocation(line: 152, column: 85, scope: !21)
!24 = !DILocation(line: 152, column: 9, scope: !21)
!25 = distinct !DISubprogram(name: "abort", linkageName: "_CX1N3std5abortIM3std6stringM3std6stringM3std6stringM3std5int32M3std6stringM3std5int32EVEM3std5never6_M3std6stringM3std6stringM3std6stringM3std5int32M3std6stringM3std5int32", scope: !26, file: !26, line: 9, type: !5, scopeLine: 9, spFlags: DISPFlagDefinition, unit: !2)
!26 = !DIFile(filename: "error.cx")
!27 = !DILocation(line: 11, column: 9, scope: !25)
!28 = !DILocation(line: 11, column: 18, scope: !25)
!29 = !DILocation(line: 13, column: 5, scope: !25)
!30 = !DILocation(line: 13, column: 14, scope: !25)
!31 = !DILocation(line: 14, column: 5, scope: !25)
