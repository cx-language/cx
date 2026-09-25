
%Foo = type { float }

define i32 @main() #0 !dbg !4 {
  %f = alloca %Foo, align 8
  call void @_CX1N4mainM4main3Foo4initE4void1_M3std5int32(ptr %f, i32 5), !dbg !7
  call void @_CX1N4mainM4main3Foo6deinitE4void0_(ptr %f), !dbg !8
  ret i32 0
}

define void @_CX1N4mainM4main3Foo4initE4void1_M3std5int32(ptr %this, i32 %i) #0 !dbg !9 {
  %i1 = alloca i32, align 4
  store i32 %i, ptr %i1, align 4
  %f = getelementptr inbounds %Foo, ptr %this, i32 0, i32 0
  store float 0.000000e+00, ptr %f, align 4
  ret void
}

define void @_CX1N4mainM4main3Foo6deinitE4void0_(ptr %this) #0 !dbg !10 {
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "constructor-call-before-declaration.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 3, type: !5, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 4, column: 13, scope: !4)
!8 = !DILocation(line: 3, column: 6, scope: !4)
!9 = distinct !DISubprogram(name: "init", linkageName: "_CX1N4mainM4main3Foo4initE4void1_M3std5int32", scope: !3, file: !3, line: 9, type: !5, scopeLine: 9, spFlags: DISPFlagDefinition, unit: !2)
!10 = distinct !DISubprogram(name: "deinit", linkageName: "_CX1N4mainM4main3Foo6deinitE4void0_", scope: !3, file: !3, line: 10, type: !5, scopeLine: 10, spFlags: DISPFlagDefinition, unit: !2)
