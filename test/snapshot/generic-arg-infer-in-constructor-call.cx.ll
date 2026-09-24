
%"Foo<int32>" = type { i32 }
%"Foo<string>" = type { %string }
%string = type { %"Slice<char>" }
%"Slice<char>" = type { ptr, i32 }

@0 = private unnamed_addr constant [1 x i8] zeroinitializer, align 1

define i32 @main() #0 !dbg !4 {
  %i = alloca %"Foo<int32>", align 8
  %b = alloca %"Foo<string>", align 8
  %__str = alloca %string, align 8
  call void @_EN4main3FooI5int32E4initE5int32(ptr %i, i32 42), !dbg !7
  call void @_EN3std6string4initEP4char5int32(ptr %__str, ptr @0, i32 0), !dbg !8
  %__str.load = load %string, ptr %__str, align 8
  call void @_EN4main3FooI6stringE4initE6string(ptr %b, %string %__str.load), !dbg !9
  ret i32 0
}

define void @_EN4main3FooI5int32E4initE5int32(ptr %this, i32 %t) #0 !dbg !10 {
  %t1 = alloca i32, align 4
  store i32 %t, ptr %t1, align 4
  %t2 = getelementptr inbounds %"Foo<int32>", ptr %this, i32 0, i32 0
  %t.load = load i32, ptr %t1, align 4
  store i32 %t.load, ptr %t2, align 4
  ret void
}

declare void @_EN3std6string4initEP4char5int32(ptr, ptr, i32) #0

define void @_EN4main3FooI6stringE4initE6string(ptr %this, %string %t) #0 !dbg !11 {
  %t1 = alloca %string, align 8
  store %string %t, ptr %t1, align 8
  %t2 = getelementptr inbounds %"Foo<string>", ptr %this, i32 0, i32 0
  %t.load = load %string, ptr %t1, align 8
  store %string %t.load, ptr %t2, align 8
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "generic-arg-infer-in-constructor-call.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 9, type: !5, scopeLine: 9, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 10, column: 13, scope: !4)
!8 = !DILocation(line: 9, column: 6, scope: !4)
!9 = !DILocation(line: 11, column: 13, scope: !4)
!10 = distinct !DISubprogram(name: "init", linkageName: "_EN4main3FooI5int32E4initE5int32", scope: !3, file: !3, line: 6, type: !5, scopeLine: 6, spFlags: DISPFlagDefinition, unit: !2)
!11 = distinct !DISubprogram(name: "init", linkageName: "_EN4main3FooI6stringE4initE6string", scope: !3, file: !3, line: 6, type: !5, scopeLine: 6, spFlags: DISPFlagDefinition, unit: !2)
