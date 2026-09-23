
%vec2 = type { i32, i32 }

@0 = private unnamed_addr constant [51 x i8] c"integer overflow at operator-overloading.cx:13:21\0A\00", align 1
@1 = private unnamed_addr constant [51 x i8] c"integer overflow at operator-overloading.cx:13:32\0A\00", align 1

define %vec2 @_EN4mainmlE4vec24vec2(%vec2 %a, %vec2 %b) #0 !dbg !4 {
  %a1 = alloca %vec2, align 8
  %b2 = alloca %vec2, align 8
  %1 = alloca %vec2, align 8
  store %vec2 %a, ptr %a1, align 4
  store %vec2 %b, ptr %b2, align 4
  %x = getelementptr inbounds %vec2, ptr %a1, i32 0, i32 0
  %x.load = load i32, ptr %x, align 4
  %x3 = getelementptr inbounds %vec2, ptr %b2, i32 0, i32 0
  %x.load4 = load i32, ptr %x3, align 4
  %2 = sext i32 %x.load to i64
  %3 = sext i32 %x.load4 to i64
  %4 = mul i64 %2, %3
  %5 = trunc i64 %4 to i32
  %6 = sext i32 %5 to i64
  %7 = icmp ne i64 %4, %6
  %8 = xor i1 %7, true
  %overflow.condition = icmp eq i1 %8, false
  br i1 %overflow.condition, label %overflow.fail, label %overflow.success

overflow.fail:                                    ; preds = %0
  call void @_EN3std10assertFailEP4char(ptr @0), !dbg !7
  unreachable

overflow.success:                                 ; preds = %0
  %y = getelementptr inbounds %vec2, ptr %a1, i32 0, i32 1
  %y.load = load i32, ptr %y, align 4
  %y5 = getelementptr inbounds %vec2, ptr %b2, i32 0, i32 1
  %y.load6 = load i32, ptr %y5, align 4
  %9 = sext i32 %y.load to i64
  %10 = sext i32 %y.load6 to i64
  %11 = mul i64 %9, %10
  %12 = trunc i64 %11 to i32
  %13 = sext i32 %12 to i64
  %14 = icmp ne i64 %11, %13
  %15 = xor i1 %14, true
  %overflow.condition7 = icmp eq i1 %15, false
  br i1 %overflow.condition7, label %overflow.fail8, label %overflow.success9

overflow.fail8:                                   ; preds = %overflow.success
  call void @_EN3std10assertFailEP4char(ptr @1), !dbg !7
  unreachable

overflow.success9:                                ; preds = %overflow.success
  call void @_EN4main4vec24initE3int3int(ptr %1, i32 %5, i32 %12), !dbg !8
  %.load = load %vec2, ptr %1, align 4
  ret %vec2 %.load
}

declare void @_EN3std10assertFailEP4char(ptr) #0

define void @_EN4main4vec24initE3int3int(ptr %this, i32 %x, i32 %y) #0 !dbg !9 {
  %x1 = alloca i32, align 4
  %y2 = alloca i32, align 4
  store i32 %x, ptr %x1, align 4
  store i32 %y, ptr %y2, align 4
  %x3 = getelementptr inbounds %vec2, ptr %this, i32 0, i32 0
  %x.load = load i32, ptr %x1, align 4
  store i32 %x.load, ptr %x3, align 4
  %y4 = getelementptr inbounds %vec2, ptr %this, i32 0, i32 1
  %y.load = load i32, ptr %y2, align 4
  store i32 %y.load, ptr %y4, align 4
  ret void
}

define i32 @main() #0 !dbg !10 {
  %v = alloca %vec2, align 8
  %1 = alloca %vec2, align 8
  %2 = alloca %vec2, align 8
  call void @_EN4main4vec24initE3int3int(ptr %v, i32 3, i32 2), !dbg !11
  call void @_EN4main4vec24initE3int3int(ptr %1, i32 2, i32 4), !dbg !12
  %.load = load %vec2, ptr %1, align 4
  %v.load = load %vec2, ptr %v, align 4
  %3 = call %vec2 @_EN4mainmlE4vec24vec2(%vec2 %.load, %vec2 %v.load), !dbg !13
  store %vec2 %3, ptr %v, align 4
  %v.load1 = load %vec2, ptr %v, align 4
  call void @_EN4main4vec24initE3int3int(ptr %2, i32 -1, i32 3), !dbg !14
  %.load2 = load %vec2, ptr %2, align 4
  %4 = call i1 @_EN4maineqE4vec24vec2(%vec2 %v.load1, %vec2 %.load2), !dbg !15
  %x = getelementptr inbounds %vec2, ptr %v, i32 0, i32 0
  %x.load = load i32, ptr %x, align 4
  %5 = call i32 @_EN4main4vec2ixE3int(ptr %v, i32 %x.load), !dbg !16
  ret i32 0
}

define i1 @_EN4maineqE4vec24vec2(%vec2 %a, %vec2 %b) #0 !dbg !17 {
  %a1 = alloca %vec2, align 8
  %b2 = alloca %vec2, align 8
  store %vec2 %a, ptr %a1, align 4
  store %vec2 %b, ptr %b2, align 4
  %x = getelementptr inbounds %vec2, ptr %a1, i32 0, i32 0
  %x.load = load i32, ptr %x, align 4
  %x3 = getelementptr inbounds %vec2, ptr %b2, i32 0, i32 0
  %x.load4 = load i32, ptr %x3, align 4
  %1 = icmp eq i32 %x.load, %x.load4
  ret i1 %1
}

define i32 @_EN4main4vec2ixE3int(ptr %this, i32 %index) #0 !dbg !18 {
  %index1 = alloca i32, align 4
  store i32 %index, ptr %index1, align 4
  %index.load = load i32, ptr %index1, align 4
  ret i32 %index.load
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "operator-overloading.cx")
!4 = distinct !DISubprogram(name: "*", linkageName: "_EN4mainmlE4vec24vec2", scope: !3, file: !3, line: 12, type: !5, scopeLine: 12, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 12, column: 6, scope: !4)
!8 = !DILocation(line: 13, column: 12, scope: !4)
!9 = distinct !DISubprogram(name: "init", linkageName: "_EN4main4vec24initE3int3int", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!10 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 16, type: !5, scopeLine: 16, spFlags: DISPFlagDefinition, unit: !2)
!11 = !DILocation(line: 17, column: 13, scope: !10)
!12 = !DILocation(line: 18, column: 9, scope: !10)
!13 = !DILocation(line: 18, column: 20, scope: !10)
!14 = !DILocation(line: 19, column: 14, scope: !10)
!15 = !DILocation(line: 19, column: 11, scope: !10)
!16 = !DILocation(line: 20, column: 10, scope: !10)
!17 = distinct !DISubprogram(name: "==", linkageName: "_EN4maineqE4vec24vec2", scope: !3, file: !3, line: 23, type: !5, scopeLine: 23, spFlags: DISPFlagDefinition, unit: !2)
!18 = distinct !DISubprogram(name: "[]", linkageName: "_EN4main4vec2ixE3int", scope: !3, file: !3, line: 7, type: !5, scopeLine: 7, spFlags: DISPFlagDefinition, unit: !2)
