
%Big = type { i32, i32, i32, i32, i32 }
%Outer = type { %Big, i32 }
%Wide = type { i128, i128 }

@0 = private unnamed_addr constant [51 x i8] c"integer overflow at large-struct-passing.cx:40:16\0A\00", align 1
@1 = private unnamed_addr constant [51 x i8] c"integer overflow at large-struct-passing.cx:40:37\0A\00", align 1
@2 = private unnamed_addr constant [51 x i8] c"integer overflow at large-struct-passing.cx:40:55\0A\00", align 1

define void @_EN4main5identE3Big(ptr sret(%Big) align 4 %sret.arg, ptr byval(%Big) align 4 %x) #0 !dbg !4 {
  %x1 = alloca %Big, align 8
  call void @llvm.memcpy.p0.p0.i64(ptr align 4 %x1, ptr align 4 %x, i64 20, i1 false)
  %x.load = alloca %Big, align 8
  call void @llvm.memcpy.p0.p0.i64(ptr align 4 %x.load, ptr align 4 %x1, i64 20, i1 false)
  call void @llvm.memcpy.p0.p0.i64(ptr align 4 %sret.arg, ptr align 4 %x.load, i64 20, i1 false)
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias writeonly captures(none), ptr noalias readonly captures(none), i64, i1 immarg) #1

define void @_EN4main9makeOuterE(ptr sret(%Outer) align 4 %sret.arg) #0 !dbg !7 {
  %1 = alloca %Outer, align 8
  %2 = alloca %Big, align 8
  call void @_EN4main3Big4initE3int3int3int3int3int(ptr %2, i32 1, i32 2, i32 3, i32 4, i32 5), !dbg !8
  %.load = alloca %Big, align 8
  call void @llvm.memcpy.p0.p0.i64(ptr align 4 %.load, ptr align 4 %2, i64 20, i1 false)
  call void @_EN4main5Outer4initE3Big3int(ptr %1, ptr byval(%Big) align 4 %.load, i32 7), !dbg !9
  %.load1 = alloca %Outer, align 8
  call void @llvm.memcpy.p0.p0.i64(ptr align 4 %.load1, ptr align 4 %1, i64 24, i1 false)
  call void @llvm.memcpy.p0.p0.i64(ptr align 4 %sret.arg, ptr align 4 %.load1, i64 24, i1 false)
  ret void
}

define void @_EN4main3Big4initE3int3int3int3int3int(ptr %this, i32 %a, i32 %b, i32 %c, i32 %d, i32 %e) #0 !dbg !10 {
  %a1 = alloca i32, align 4
  %b2 = alloca i32, align 4
  %c3 = alloca i32, align 4
  %d4 = alloca i32, align 4
  %e5 = alloca i32, align 4
  store i32 %a, ptr %a1, align 4
  store i32 %b, ptr %b2, align 4
  store i32 %c, ptr %c3, align 4
  store i32 %d, ptr %d4, align 4
  store i32 %e, ptr %e5, align 4
  %a6 = getelementptr inbounds %Big, ptr %this, i32 0, i32 0
  %a.load = load i32, ptr %a1, align 4
  store i32 %a.load, ptr %a6, align 4
  %b7 = getelementptr inbounds %Big, ptr %this, i32 0, i32 1
  %b.load = load i32, ptr %b2, align 4
  store i32 %b.load, ptr %b7, align 4
  %c8 = getelementptr inbounds %Big, ptr %this, i32 0, i32 2
  %c.load = load i32, ptr %c3, align 4
  store i32 %c.load, ptr %c8, align 4
  %d9 = getelementptr inbounds %Big, ptr %this, i32 0, i32 3
  %d.load = load i32, ptr %d4, align 4
  store i32 %d.load, ptr %d9, align 4
  %e10 = getelementptr inbounds %Big, ptr %this, i32 0, i32 4
  %e.load = load i32, ptr %e5, align 4
  store i32 %e.load, ptr %e10, align 4
  ret void
}

define void @_EN4main5Outer4initE3Big3int(ptr %this, ptr byval(%Big) align 4 %item, i32 %tag) #0 !dbg !11 {
  %item1 = alloca %Big, align 8
  %tag2 = alloca i32, align 4
  call void @llvm.memcpy.p0.p0.i64(ptr align 4 %item1, ptr align 4 %item, i64 20, i1 false)
  store i32 %tag, ptr %tag2, align 4
  %item3 = getelementptr inbounds %Outer, ptr %this, i32 0, i32 0
  %item.load = alloca %Big, align 8
  call void @llvm.memcpy.p0.p0.i64(ptr align 4 %item.load, ptr align 4 %item1, i64 20, i1 false)
  call void @llvm.memcpy.p0.p0.i64(ptr align 4 %item3, ptr align 4 %item.load, i64 20, i1 false)
  %tag4 = getelementptr inbounds %Outer, ptr %this, i32 0, i32 1
  %tag.load = load i32, ptr %tag2, align 4
  store i32 %tag.load, ptr %tag4, align 4
  ret void
}

define void @_EN4main9identWideE4Wide(ptr sret(%Wide) align 16 %sret.arg, ptr byval(%Wide) align 16 %x) #0 !dbg !12 {
  %x1 = alloca %Wide, align 16
  call void @llvm.memcpy.p0.p0.i64(ptr align 16 %x1, ptr align 16 %x, i64 32, i1 false)
  %x.load = alloca %Wide, align 16
  call void @llvm.memcpy.p0.p0.i64(ptr align 16 %x.load, ptr align 16 %x1, i64 32, i1 false)
  call void @llvm.memcpy.p0.p0.i64(ptr align 16 %sret.arg, ptr align 16 %x.load, i64 32, i1 false)
  ret void
}

define i32 @main() #0 !dbg !13 {
  %b = alloca %Big, align 8
  %c = alloca %Big, align 8
  %flag = alloca i1, align 1
  %d = alloca %Big, align 8
  %w = alloca %Wide, align 16
  %v = alloca %Wide, align 16
  call void @_EN4main3Big4initE3int3int3int3int3int(ptr %b, i32 1, i32 2, i32 3, i32 4, i32 5), !dbg !14
  %b.load = alloca %Big, align 8
  call void @llvm.memcpy.p0.p0.i64(ptr align 4 %b.load, ptr align 4 %b, i64 20, i1 false)
  %sret.alloca = alloca %Big, align 8, !dbg !15
  call void @_EN4main5identE3Big(ptr sret(%Big) align 4 %sret.alloca, ptr byval(%Big) align 4 %b.load), !dbg !15
  call void @llvm.memcpy.p0.p0.i64(ptr align 4 %c, ptr align 4 %sret.alloca, i64 20, i1 false)
  store i1 true, ptr %flag, align 1
  %flag.load = load i1, ptr %flag, align 1
  br i1 %flag.load, label %if.then, label %if.else

if.then:                                          ; preds = %0
  %b.load1 = alloca %Big, align 8
  call void @llvm.memcpy.p0.p0.i64(ptr align 4 %b.load1, ptr align 4 %b, i64 20, i1 false)
  br label %if.end

if.else:                                          ; preds = %0
  %c.load = alloca %Big, align 8
  call void @llvm.memcpy.p0.p0.i64(ptr align 4 %c.load, ptr align 4 %c, i64 20, i1 false)
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %if.result = phi ptr [ %b.load1, %if.then ], [ %c.load, %if.else ]
  call void @llvm.memcpy.p0.p0.i64(ptr align 4 %d, ptr align 4 %if.result, i64 20, i1 false)
  call void @_EN4main4Wide4initE6int1286int128(ptr %w, i128 1, i128 2), !dbg !16
  %w.load = alloca %Wide, align 16
  call void @llvm.memcpy.p0.p0.i64(ptr align 16 %w.load, ptr align 16 %w, i64 32, i1 false)
  %sret.alloca2 = alloca %Wide, align 16, !dbg !17
  call void @_EN4main9identWideE4Wide(ptr sret(%Wide) align 16 %sret.alloca2, ptr byval(%Wide) align 16 %w.load), !dbg !17
  call void @llvm.memcpy.p0.p0.i64(ptr align 16 %v, ptr align 16 %sret.alloca2, i64 32, i1 false)
  %a = getelementptr inbounds %Big, ptr %d, i32 0, i32 0
  %a.load = load i32, ptr %a, align 4
  %sret.alloca3 = alloca %Outer, align 8, !dbg !18
  call void @_EN4main9makeOuterE(ptr sret(%Outer) align 4 %sret.alloca3), !dbg !18
  %item = getelementptr inbounds %Outer, ptr %sret.alloca3, i32 0, i32 0
  %extract.alloca = alloca %Big, align 8
  call void @llvm.memcpy.p0.p0.i64(ptr align 4 %extract.alloca, ptr align 4 %item, i64 20, i1 false)
  %a4 = getelementptr inbounds %Big, ptr %extract.alloca, i32 0, i32 0
  %a5 = load i32, ptr %a4, align 4
  %1 = sext i32 %a.load to i64
  %2 = sext i32 %a5 to i64
  %3 = add i64 %1, %2
  %4 = trunc i64 %3 to i32
  %5 = sext i32 %4 to i64
  %6 = icmp ne i64 %3, %5
  %7 = xor i1 %6, true
  %overflow.condition = icmp eq i1 %7, false
  br i1 %overflow.condition, label %overflow.fail, label %overflow.success

overflow.fail:                                    ; preds = %if.end
  call void @_EN3std10assertFailEP4char(ptr @0), !dbg !19
  unreachable

overflow.success:                                 ; preds = %if.end
  %sret.alloca6 = alloca %Outer, align 8, !dbg !20
  call void @_EN4main9makeOuterE(ptr sret(%Outer) align 4 %sret.alloca6), !dbg !20
  %tag = getelementptr inbounds %Outer, ptr %sret.alloca6, i32 0, i32 1
  %tag7 = load i32, ptr %tag, align 4
  %8 = sext i32 %4 to i64
  %9 = sext i32 %tag7 to i64
  %10 = add i64 %8, %9
  %11 = trunc i64 %10 to i32
  %12 = sext i32 %11 to i64
  %13 = icmp ne i64 %10, %12
  %14 = xor i1 %13, true
  %overflow.condition8 = icmp eq i1 %14, false
  br i1 %overflow.condition8, label %overflow.fail9, label %overflow.success10

overflow.fail9:                                   ; preds = %overflow.success
  call void @_EN3std10assertFailEP4char(ptr @1), !dbg !19
  unreachable

overflow.success10:                               ; preds = %overflow.success
  %a11 = getelementptr inbounds %Wide, ptr %v, i32 0, i32 0
  %a.load12 = load i128, ptr %a11, align 16
  %15 = trunc i128 %a.load12 to i32
  %16 = sext i32 %11 to i64
  %17 = sext i32 %15 to i64
  %18 = add i64 %16, %17
  %19 = trunc i64 %18 to i32
  %20 = sext i32 %19 to i64
  %21 = icmp ne i64 %18, %20
  %22 = xor i1 %21, true
  %overflow.condition13 = icmp eq i1 %22, false
  br i1 %overflow.condition13, label %overflow.fail14, label %overflow.success15

overflow.fail14:                                  ; preds = %overflow.success10
  call void @_EN3std10assertFailEP4char(ptr @2), !dbg !19
  unreachable

overflow.success15:                               ; preds = %overflow.success10
  ret i32 %19
}

define void @_EN4main4Wide4initE6int1286int128(ptr %this, i128 %a, i128 %b) #0 !dbg !21 {
  %a1 = alloca i128, align 16
  %b2 = alloca i128, align 16
  store i128 %a, ptr %a1, align 16
  store i128 %b, ptr %b2, align 16
  %a3 = getelementptr inbounds %Wide, ptr %this, i32 0, i32 0
  %a.load = load i128, ptr %a1, align 16
  store i128 %a.load, ptr %a3, align 16
  %b4 = getelementptr inbounds %Wide, ptr %this, i32 0, i32 1
  %b.load = load i128, ptr %b2, align 16
  store i128 %b.load, ptr %b4, align 16
  ret void
}

declare void @_EN3std10assertFailEP4char(ptr) #0

attributes #0 = { "frame-pointer"="all" }
attributes #1 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "large-struct-passing.cx")
!4 = distinct !DISubprogram(name: "ident", linkageName: "_EN4main5identE3Big", scope: !3, file: !3, line: 21, type: !5, scopeLine: 21, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = distinct !DISubprogram(name: "makeOuter", linkageName: "_EN4main9makeOuterE", scope: !3, file: !3, line: 25, type: !5, scopeLine: 25, spFlags: DISPFlagDefinition, unit: !2)
!8 = !DILocation(line: 26, column: 18, scope: !7)
!9 = !DILocation(line: 26, column: 12, scope: !7)
!10 = distinct !DISubprogram(name: "init", linkageName: "_EN4main3Big4initE3int3int3int3int3int", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!11 = distinct !DISubprogram(name: "init", linkageName: "_EN4main5Outer4initE3Big3int", scope: !3, file: !3, line: 11, type: !5, scopeLine: 11, spFlags: DISPFlagDefinition, unit: !2)
!12 = distinct !DISubprogram(name: "identWide", linkageName: "_EN4main9identWideE4Wide", scope: !3, file: !3, line: 29, type: !5, scopeLine: 29, spFlags: DISPFlagDefinition, unit: !2)
!13 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 33, type: !5, scopeLine: 33, spFlags: DISPFlagDefinition, unit: !2)
!14 = !DILocation(line: 34, column: 13, scope: !13)
!15 = !DILocation(line: 35, column: 13, scope: !13)
!16 = !DILocation(line: 38, column: 13, scope: !13)
!17 = !DILocation(line: 39, column: 13, scope: !13)
!18 = !DILocation(line: 40, column: 18, scope: !13)
!19 = !DILocation(line: 33, column: 5, scope: !13)
!20 = !DILocation(line: 40, column: 39, scope: !13)
!21 = distinct !DISubprogram(name: "init", linkageName: "_EN4main4Wide4initE6int1286int128", scope: !3, file: !3, line: 16, type: !5, scopeLine: 16, spFlags: DISPFlagDefinition, unit: !2)
