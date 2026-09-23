
%C = type { %"Slice<int>" }
%"Slice<int>" = type { ptr, i32 }
%string = type { %"Slice<char>" }
%"Slice<char>" = type { ptr, i32 }
%never = type {}

@0 = private unnamed_addr constant [11 x i8] c"operator[]\00", align 1
@1 = private unnamed_addr constant [7 x i8] c"Slice.\00", align 1
@2 = private unnamed_addr constant [9 x i8] c": index \00", align 1
@3 = private unnamed_addr constant [28 x i8] c" is out of bounds, size is \00", align 1

define i32 @main() #0 !dbg !4 {
  %1 = alloca %C, align 8
  %2 = alloca [5 x i32], align 4
  store [5 x i32] [i32 0, i32 1, i32 2, i32 3, i32 4], ptr %2, align 4
  %3 = getelementptr inbounds [5 x i32], ptr %2, i32 0, i32 0
  %4 = insertvalue %"Slice<int>" undef, ptr %3, 0
  %5 = insertvalue %"Slice<int>" %4, i32 5, 1
  call void @_EN4main1C4initE5SliceI3intE(ptr %1, %"Slice<int>" %5), !dbg !7
  call void @_EN4main1C3fooE(ptr %1), !dbg !8
  ret i32 0
}

define void @_EN4main1C4initE5SliceI3intE(ptr %this, %"Slice<int>" %a) #0 !dbg !9 {
  %a1 = alloca %"Slice<int>", align 8
  store %"Slice<int>" %a, ptr %a1, align 8
  %a2 = getelementptr inbounds %C, ptr %this, i32 0, i32 0
  %a.load = load %"Slice<int>", ptr %a1, align 8
  store %"Slice<int>" %a.load, ptr %a2, align 8
  ret void
}

define void @_EN4main1C3fooE(ptr %this) #0 !dbg !10 {
  %a = getelementptr inbounds %C, ptr %this, i32 0, i32 0
  %1 = call ptr @_EN3std5SliceI3intEixE3int(ptr %a, i32 4), !dbg !11
  ret void
}

define ptr @_EN3std5SliceI3intEixE3int(ptr %this, i32 %index) #0 !dbg !12 {
  %index1 = alloca i32, align 4
  %__str = alloca %string, align 8
  store i32 %index, ptr %index1, align 4
  %index.load = load i32, ptr %index1, align 4
  %1 = icmp slt i32 %index.load, 0
  br i1 %1, label %or.end, label %or.rhs

or.rhs:                                           ; preds = %0
  %index.load2 = load i32, ptr %index1, align 4
  %2 = call i32 @_EN3std5SliceI3intE4sizeE(ptr %this), !dbg !14
  %3 = icmp sge i32 %index.load2, %2
  br label %or.end

or.end:                                           ; preds = %or.rhs, %0
  %or = phi i1 [ %1, %0 ], [ %3, %or.rhs ]
  br i1 %or, label %if.then, label %if.else

if.then:                                          ; preds = %or.end
  call void @_EN3std6string4initEP4char3int(ptr %__str, ptr @0, i32 10), !dbg !15
  %__str.load = load %string, ptr %__str, align 8
  %index.load3 = load i32, ptr %index1, align 4
  call void @_EN3std5SliceI3intE16indexOutOfBoundsE6string3int(ptr %this, %string %__str.load, i32 %index.load3), !dbg !16
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

define i32 @_EN3std5SliceI3intE4sizeE(ptr %this) #0 !dbg !17 {
  %size = getelementptr inbounds %"Slice<int>", ptr %this, i32 0, i32 1
  %size.load = load i32, ptr %size, align 4
  ret i32 %size.load
}

declare void @_EN3std6string4initEP4char3int(ptr, ptr, i32) #0

define void @_EN3std5SliceI3intE16indexOutOfBoundsE6string3int(ptr %this, %string %function, i32 %index) #0 !dbg !18 {
  %function1 = alloca %string, align 8
  %index2 = alloca i32, align 4
  %__str = alloca %string, align 8
  %__str3 = alloca %string, align 8
  %__str4 = alloca %string, align 8
  %1 = alloca i32, align 4
  store %string %function, ptr %function1, align 8
  store i32 %index, ptr %index2, align 4
  call void @_EN3std6string4initEP4char3int(ptr %__str, ptr @1, i32 6), !dbg !19
  call void @_EN3std6string4initEP4char3int(ptr %__str3, ptr @2, i32 8), !dbg !19
  call void @_EN3std6string4initEP4char3int(ptr %__str4, ptr @3, i32 27), !dbg !19
  %2 = call i32 @_EN3std5SliceI3intE4sizeE(ptr %this), !dbg !20
  store i32 %2, ptr %1, align 4
  %3 = call %never @_EN3std5abortI6string6string6string3int6string3intEVER6stringR6stringR6stringR3intR6stringR3int(ptr %__str, ptr %function1, ptr %__str3, ptr %index2, ptr %__str4, ptr %1), !dbg !21
  ret void
}

declare %never @_EN3std5abortI6string6string6string3int6string3intEVER6stringR6stringR6stringR3intR6stringR3int(ptr, ptr, ptr, ptr, ptr, ptr) #0

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "member-array-subscript.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 11, type: !5, scopeLine: 11, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 12, column: 5, scope: !4)
!8 = !DILocation(line: 12, column: 20, scope: !4)
!9 = distinct !DISubprogram(name: "init", linkageName: "_EN4main1C4initE5SliceI3intE", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!10 = distinct !DISubprogram(name: "foo", linkageName: "_EN4main1C3fooE", scope: !3, file: !3, line: 6, type: !5, scopeLine: 6, spFlags: DISPFlagDefinition, unit: !2)
!11 = !DILocation(line: 7, column: 14, scope: !10)
!12 = distinct !DISubprogram(name: "[]", linkageName: "_EN3std5SliceI3intEixE3int", scope: !13, file: !13, line: 42, type: !5, scopeLine: 42, spFlags: DISPFlagDefinition, unit: !2)
!13 = !DIFile(filename: "Slice.cx")
!14 = !DILocation(line: 43, column: 34, scope: !12)
!15 = !DILocation(line: 42, column: 14, scope: !12)
!16 = !DILocation(line: 43, column: 41, scope: !12)
!17 = distinct !DISubprogram(name: "size", linkageName: "_EN3std5SliceI3intE4sizeE", scope: !13, file: !13, line: 31, type: !5, scopeLine: 31, spFlags: DISPFlagDefinition, unit: !2)
!18 = distinct !DISubprogram(name: "indexOutOfBounds", linkageName: "_EN3std5SliceI3intE16indexOutOfBoundsE6string3int", scope: !13, file: !13, line: 83, type: !5, scopeLine: 83, spFlags: DISPFlagDefinition, unit: !2)
!19 = !DILocation(line: 83, column: 18, scope: !18)
!20 = !DILocation(line: 84, column: 85, scope: !18)
!21 = !DILocation(line: 84, column: 9, scope: !18)
