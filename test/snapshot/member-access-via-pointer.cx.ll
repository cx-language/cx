
%Foo = type { i32 }

define void @_EN4main3getEP3Foo(ptr %f) #0 !dbg !4 {
  %f1 = alloca ptr, align 8
  %m = alloca i32, align 4
  store ptr %f, ptr %f1, align 8
  %f.load = load ptr, ptr %f1, align 8
  %i = getelementptr inbounds %Foo, ptr %f.load, i32 0, i32 0
  %i.load = load i32, ptr %i, align 4
  store i32 %i.load, ptr %m, align 4
  ret void
}

define i32 @main() #0 !dbg !7 {
  %f = alloca %Foo, align 8
  %rf = alloca ptr, align 8
  %n = alloca i32, align 4
  store ptr %f, ptr %rf, align 8
  %rf.load = load ptr, ptr %rf, align 8
  %i = getelementptr inbounds %Foo, ptr %rf.load, i32 0, i32 0
  %i.load = load i32, ptr %i, align 4
  store i32 %i.load, ptr %n, align 4
  %rf.load1 = load ptr, ptr %rf, align 8
  call void @_EN4main3getEP3Foo(ptr %rf.load1), !dbg !8
  ret i32 0
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "member-access-via-pointer.cx")
!4 = distinct !DISubprogram(name: "get", linkageName: "_EN4main3getEP3Foo", scope: !3, file: !3, line: 7, type: !5, scopeLine: 7, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 11, type: !5, scopeLine: 11, spFlags: DISPFlagDefinition, unit: !2)
!8 = !DILocation(line: 15, column: 5, scope: !7)
