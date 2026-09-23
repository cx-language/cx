
%"Slice<int>" = type { ptr, i32 }
%string = type { %"Slice<char>" }
%"Slice<char>" = type { ptr, i32 }
%never = type {}

@0 = private unnamed_addr constant [11 x i8] c"operator[]\00", align 1
@1 = private unnamed_addr constant [7 x i8] c"Slice.\00", align 1
@2 = private unnamed_addr constant [9 x i8] c": index \00", align 1
@3 = private unnamed_addr constant [28 x i8] c" is out of bounds, size is \00", align 1

define i32 @main() #0 !dbg !4 {
  %three = alloca [3 x i32], align 4
  %b = alloca i32, align 4
  %ref = alloca %"Slice<int>", align 8
  %bb = alloca i32, align 4
  store [3 x i32] [i32 0, i32 42, i32 0], ptr %three, align 4
  %1 = getelementptr inbounds [3 x i32], ptr %three, i32 0, i32 0
  %2 = insertvalue %"Slice<int>" undef, ptr %1, 0
  %3 = insertvalue %"Slice<int>" %2, i32 3, 1
  call void @_EN4main3fooE5SliceI3intE(%"Slice<int>" %3), !dbg !7
  call void @_EN4main3barEPA3_3int(ptr %three), !dbg !8
  store i32 3, ptr %b, align 4
  %4 = getelementptr inbounds [3 x i32], ptr %three, i32 0, i32 0
  %5 = insertvalue %"Slice<int>" undef, ptr %4, 0
  %6 = insertvalue %"Slice<int>" %5, i32 3, 1
  store %"Slice<int>" %6, ptr %ref, align 8
  %7 = call i32 @_EN3std5SliceI3intE4sizeE(ptr %ref), !dbg !9
  store i32 %7, ptr %bb, align 4
  ret i32 0
}

define void @_EN4main3fooE5SliceI3intE(%"Slice<int>" %ints) #0 !dbg !10 {
  %ints1 = alloca %"Slice<int>", align 8
  %a = alloca i32, align 4
  %b = alloca i32, align 4
  %c = alloca %"Slice<int>", align 8
  store %"Slice<int>" %ints, ptr %ints1, align 8
  %1 = call ptr @_EN3std5SliceI3intEixE3int(ptr %ints1, i32 1), !dbg !11
  %.load = load i32, ptr %1, align 4
  store i32 %.load, ptr %a, align 4
  %2 = call i32 @_EN3std5SliceI3intE4sizeE(ptr %ints1), !dbg !12
  store i32 %2, ptr %b, align 4
  %ints.load = load %"Slice<int>", ptr %ints1, align 8
  store %"Slice<int>" %ints.load, ptr %c, align 8
  ret void
}

define void @_EN4main3barEPA3_3int(ptr %ints) #0 !dbg !13 {
  %ints1 = alloca ptr, align 8
  %b = alloca i32, align 4
  %ref = alloca %"Slice<int>", align 8
  store ptr %ints, ptr %ints1, align 8
  store i32 3, ptr %b, align 4
  %ints.load = load ptr, ptr %ints1, align 8
  %1 = getelementptr inbounds [3 x i32], ptr %ints.load, i32 0, i32 0
  %2 = insertvalue %"Slice<int>" undef, ptr %1, 0
  %3 = insertvalue %"Slice<int>" %2, i32 3, 1
  store %"Slice<int>" %3, ptr %ref, align 8
  ret void
}

define i32 @_EN3std5SliceI3intE4sizeE(ptr %this) #0 !dbg !14 {
  %size = getelementptr inbounds %"Slice<int>", ptr %this, i32 0, i32 1
  %size.load = load i32, ptr %size, align 4
  ret i32 %size.load
}

define ptr @_EN3std5SliceI3intEixE3int(ptr %this, i32 %index) #0 !dbg !16 {
  %index1 = alloca i32, align 4
  %__str = alloca %string, align 8
  store i32 %index, ptr %index1, align 4
  %index.load = load i32, ptr %index1, align 4
  %1 = icmp slt i32 %index.load, 0
  br i1 %1, label %or.end, label %or.rhs

or.rhs:                                           ; preds = %0
  %index.load2 = load i32, ptr %index1, align 4
  %2 = call i32 @_EN3std5SliceI3intE4sizeE(ptr %this), !dbg !17
  %3 = icmp sge i32 %index.load2, %2
  br label %or.end

or.end:                                           ; preds = %or.rhs, %0
  %or = phi i1 [ %1, %0 ], [ %3, %or.rhs ]
  br i1 %or, label %if.then, label %if.else

if.then:                                          ; preds = %or.end
  call void @_EN3std6string4initEP4char3int(ptr %__str, ptr @0, i32 10), !dbg !18
  %__str.load = load %string, ptr %__str, align 8
  %index.load3 = load i32, ptr %index1, align 4
  call void @_EN3std5SliceI3intE16indexOutOfBoundsE6string3int(ptr %this, %string %__str.load, i32 %index.load3), !dbg !19
  br label %if.end

if.else:                                          ; preds = %or.end
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %data = getelementptr inbounds %"Slice<int>", ptr %this, i32 0, i32 0
  %data.load = load ptr, ptr %data, align 8
  %index.load4 = load i32, ptr %index1, align 4
  %4 = getelementptr inbounds i32, ptr %data.load, i32 %index.load4
  ret ptr %4
}

declare void @_EN3std6string4initEP4char3int(ptr, ptr, i32) #0

define void @_EN3std5SliceI3intE16indexOutOfBoundsE6string3int(ptr %this, %string %function, i32 %index) #0 !dbg !20 {
  %function1 = alloca %string, align 8
  %index2 = alloca i32, align 4
  %__str = alloca %string, align 8
  %__str3 = alloca %string, align 8
  %__str4 = alloca %string, align 8
  %1 = alloca i32, align 4
  store %string %function, ptr %function1, align 8
  store i32 %index, ptr %index2, align 4
  call void @_EN3std6string4initEP4char3int(ptr %__str, ptr @1, i32 6), !dbg !21
  call void @_EN3std6string4initEP4char3int(ptr %__str3, ptr @2, i32 8), !dbg !21
  call void @_EN3std6string4initEP4char3int(ptr %__str4, ptr @3, i32 27), !dbg !21
  %2 = call i32 @_EN3std5SliceI3intE4sizeE(ptr %this), !dbg !22
  store i32 %2, ptr %1, align 4
  %3 = call %never @_EN3std5abortI6string6string6string3int6string3intEVER6stringR6stringR6stringR3intR6stringR3int(ptr %__str, ptr %function1, ptr %__str3, ptr %index2, ptr %__str4, ptr %1), !dbg !23
  ret void
}

declare %never @_EN3std5abortI6string6string6string3int6string3intEVER6stringR6stringR6stringR3intR6stringR3int(ptr, ptr, ptr, ptr, ptr, ptr) #0

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "unsized-array.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 5, column: 5, scope: !4)
!8 = !DILocation(line: 6, column: 5, scope: !4)
!9 = !DILocation(line: 9, column: 18, scope: !4)
!10 = distinct !DISubprogram(name: "foo", linkageName: "_EN4main3fooE5SliceI3intE", scope: !3, file: !3, line: 12, type: !5, scopeLine: 12, spFlags: DISPFlagDefinition, unit: !2)
!11 = !DILocation(line: 13, column: 17, scope: !10)
!12 = !DILocation(line: 14, column: 18, scope: !10)
!13 = distinct !DISubprogram(name: "bar", linkageName: "_EN4main3barEPA3_3int", scope: !3, file: !3, line: 18, type: !5, scopeLine: 18, spFlags: DISPFlagDefinition, unit: !2)
!14 = distinct !DISubprogram(name: "size", linkageName: "_EN3std5SliceI3intE4sizeE", scope: !15, file: !15, line: 31, type: !5, scopeLine: 31, spFlags: DISPFlagDefinition, unit: !2)
!15 = !DIFile(filename: "Slice.cx")
!16 = distinct !DISubprogram(name: "[]", linkageName: "_EN3std5SliceI3intEixE3int", scope: !15, file: !15, line: 42, type: !5, scopeLine: 42, spFlags: DISPFlagDefinition, unit: !2)
!17 = !DILocation(line: 43, column: 34, scope: !16)
!18 = !DILocation(line: 42, column: 14, scope: !16)
!19 = !DILocation(line: 43, column: 41, scope: !16)
!20 = distinct !DISubprogram(name: "indexOutOfBounds", linkageName: "_EN3std5SliceI3intE16indexOutOfBoundsE6string3int", scope: !15, file: !15, line: 83, type: !5, scopeLine: 83, spFlags: DISPFlagDefinition, unit: !2)
!21 = !DILocation(line: 83, column: 18, scope: !20)
!22 = !DILocation(line: 84, column: 85, scope: !20)
!23 = !DILocation(line: 84, column: 9, scope: !20)
