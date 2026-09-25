
%Foo = type { i32 }
%Bar = type { i32 }

define i32 @main() #0 !dbg !4 {
  %f = alloca %Foo, align 8
  %f2 = alloca %Foo, align 8
  %b = alloca %Bar, align 8
  %b2 = alloca %Bar, align 8
  %i = alloca i32, align 4
  br i1 false, label %if.then, label %if.else

if.then:                                          ; preds = %0
  call void @_CX1N4mainM4main3Foo6deinitE4void0_(ptr %f2), !dbg !7
  br label %if.end

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  br label %loop.condition

loop.condition:                                   ; preds = %if.end
  br i1 false, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  call void @_CX1N4mainM4main3Bar6deinitE4void0_(ptr %b2), !dbg !7
  call void @_CX1N4mainM4main3Bar6deinitE4void0_(ptr %b), !dbg !7
  call void @_CX1N4mainM4main3Foo6deinitE4void0_(ptr %f), !dbg !7
  ret i32 0

loop.end:                                         ; preds = %loop.condition
  store i32 1, ptr %i, align 4
  call void @_CX1N4mainM4main3Bar6deinitE4void0_(ptr %b), !dbg !7
  call void @_CX1N4mainM4main3Foo6deinitE4void0_(ptr %f), !dbg !7
  ret i32 0
}

define void @_CX1N4mainM4main3Foo6deinitE4void0_(ptr %this) #0 !dbg !8 {
  %i = getelementptr inbounds %Foo, ptr %this, i32 0, i32 0
  store i32 0, ptr %i, align 4
  call void @_CX1N4mainM4main3Foo1fE4void0_(ptr %this), !dbg !9
  call void @_CX1N4mainM4main3Foo1fE4void0_(ptr %this), !dbg !10
  ret void
}

define void @_CX1N4mainM4main3Bar6deinitE4void0_(ptr %this) #0 !dbg !11 {
  %i = getelementptr inbounds %Bar, ptr %this, i32 0, i32 0
  store i32 0, ptr %i, align 4
  call void @_CX1N4mainM4main3Bar1fE4void0_(ptr %this), !dbg !12
  call void @_CX1N4mainM4main3Bar1fE4void0_(ptr %this), !dbg !13
  ret void
}

define void @_CX1N4mainM4main3Foo1fE4void0_(ptr %this) #0 !dbg !14 {
  ret void
}

define void @_CX1N4mainM4main3Bar1fE4void0_(ptr %this) #0 !dbg !15 {
  ret void
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "destructor.cx")
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !3, file: !3, line: 27, type: !5, scopeLine: 27, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 27, column: 6, scope: !4)
!8 = distinct !DISubprogram(name: "deinit", linkageName: "_CX1N4mainM4main3Foo6deinitE4void0_", scope: !3, file: !3, line: 6, type: !5, scopeLine: 6, spFlags: DISPFlagDefinition, unit: !2)
!9 = !DILocation(line: 8, column: 9, scope: !8)
!10 = !DILocation(line: 9, column: 14, scope: !8)
!11 = distinct !DISubprogram(name: "deinit", linkageName: "_CX1N4mainM4main3Bar6deinitE4void0_", scope: !3, file: !3, line: 18, type: !5, scopeLine: 18, spFlags: DISPFlagDefinition, unit: !2)
!12 = !DILocation(line: 20, column: 9, scope: !11)
!13 = !DILocation(line: 21, column: 14, scope: !11)
!14 = distinct !DISubprogram(name: "f", linkageName: "_CX1N4mainM4main3Foo1fE4void0_", scope: !3, file: !3, line: 12, type: !5, scopeLine: 12, spFlags: DISPFlagDefinition, unit: !2)
!15 = distinct !DISubprogram(name: "f", linkageName: "_CX1N4mainM4main3Bar1fE4void0_", scope: !3, file: !3, line: 24, type: !5, scopeLine: 24, spFlags: DISPFlagDefinition, unit: !2)
