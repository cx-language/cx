
%Foo = type { i32 }

define i32 @main() #0 !dbg !4 {
  %f = alloca ptr, align 8
  %a = alloca i32, align 4
  %b = alloca ptr, align 8
  %c = alloca ptr, align 8
  store ptr null, ptr %f, align 8
  %f.load = load ptr, ptr %f, align 8
  %i = getelementptr inbounds %Foo, ptr %f.load, i32 0, i32 0
  %f.load1 = load ptr, ptr %f, align 8
  %i2 = getelementptr inbounds %Foo, ptr %f.load1, i32 0, i32 0
  %i.load = load i32, ptr %i2, align 4
  store i32 %i.load, ptr %a, align 4
  %f.load3 = load ptr, ptr %f, align 8
  %i4 = getelementptr inbounds %Foo, ptr %f.load3, i32 0, i32 0
  store ptr %i4, ptr %b, align 8
  %f.load5 = load ptr, ptr %f, align 8
  %i6 = getelementptr inbounds %Foo, ptr %f.load5, i32 0, i32 0
  store ptr %i6, ptr %c, align 8
  %f.load7 = load ptr, ptr %f, align 8
  call void @_EN4main3Foo3barE(ptr %f.load7), !dbg !7
  %f.load8 = load ptr, ptr %f, align 8
  %f.load9 = load ptr, ptr %f, align 8
  %f.load.load = load %Foo, ptr %f.load9, align 4
  store %Foo %f.load.load, ptr %f.load8, align 4
  ret i32 0
}

define void @_EN4main3Foo3barE(ptr %this) #0 !dbg !8 {
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "optional-pointer-access-without-unwrapping.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 8, type: !5, scopeLine: 8, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 14, column: 7, scope: !4)
!8 = distinct !DISubprogram(name: "bar", linkageName: "_EN4main3Foo3barE", scope: !3, file: !3, line: 5, type: !5, scopeLine: 5, spFlags: DISPFlagDefinition, unit: !2)
