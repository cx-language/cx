
%"S<int32>" = type { i32 }
%"S<bool>" = type { i1 }

define void @_CX1N4main1fE4void2_M4main1SIM3std5int32EM4main1SIM3std5int32E(%"S<int32>" %c, %"S<int32>" %d) #0 !dbg !4 {
  %c1 = alloca %"S<int32>", align 8
  %d2 = alloca %"S<int32>", align 8
  store %"S<int32>" %c, ptr %c1, align 4
  store %"S<int32>" %d, ptr %d2, align 4
  %c.load = load %"S<int32>", ptr %c1, align 4
  %d.load = load %"S<int32>", ptr %d2, align 4
  %1 = call i1 @_CX1N4maino2eqIM3std5int32EEM3std4bool2_M4main1SIM3std5int32EM4main1SIM3std5int32E(%"S<int32>" %c.load, %"S<int32>" %d.load), !dbg !7
  ret void
}

define i1 @_CX1N4maino2eqIM3std5int32EEM3std4bool2_M4main1SIM3std5int32EM4main1SIM3std5int32E(%"S<int32>" %a, %"S<int32>" %b) #0 !dbg !8 {
  %a1 = alloca %"S<int32>", align 8
  %b2 = alloca %"S<int32>", align 8
  store %"S<int32>" %a, ptr %a1, align 4
  store %"S<int32>" %b, ptr %b2, align 4
  ret i1 true
}

define void @_CX1N4main1fE4void2_PM4main1SIM3std4boolEPM4main1SIM3std4boolE(ptr %c, ptr %d) #0 !dbg !9 {
  %c1 = alloca ptr, align 8
  %d2 = alloca ptr, align 8
  store ptr %c, ptr %c1, align 8
  store ptr %d, ptr %d2, align 8
  %c.load = load ptr, ptr %c1, align 8
  %c.load.load = load %"S<bool>", ptr %c.load, align 1
  %d.load = load ptr, ptr %d2, align 8
  %d.load.load = load %"S<bool>", ptr %d.load, align 1
  %1 = call i1 @_CX1N4maino2eqIM3std4boolEEM3std4bool2_M4main1SIM3std4boolEM4main1SIM3std4boolE(%"S<bool>" %c.load.load, %"S<bool>" %d.load.load), !dbg !10
  ret void
}

define i1 @_CX1N4maino2eqIM3std4boolEEM3std4bool2_M4main1SIM3std4boolEM4main1SIM3std4boolE(%"S<bool>" %a, %"S<bool>" %b) #0 !dbg !11 {
  %a1 = alloca %"S<bool>", align 8
  %b2 = alloca %"S<bool>", align 8
  store %"S<bool>" %a, ptr %a1, align 1
  store %"S<bool>" %b, ptr %b2, align 1
  ret i1 true
}

attributes #0 = { "frame-pointer"="all" }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C, file: !3, producer: "cx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!3 = !DIFile(filename: "generic-argument-inference-deep.cx")
!4 = distinct !DISubprogram(name: "f", linkageName: "_CX1N4main1fE4void2_M4main1SIM3std5int32EM4main1SIM3std5int32E", scope: !3, file: !3, line: 11, type: !5, scopeLine: 11, spFlags: DISPFlagDefinition, unit: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 12, column: 11, scope: !4)
!8 = distinct !DISubprogram(name: "==", linkageName: "_CX1N4maino2eqIM3std5int32EEM3std4bool2_M4main1SIM3std5int32EM4main1SIM3std5int32E", scope: !3, file: !3, line: 7, type: !5, scopeLine: 7, spFlags: DISPFlagDefinition, unit: !2)
!9 = distinct !DISubprogram(name: "f", linkageName: "_CX1N4main1fE4void2_PM4main1SIM3std4boolEPM4main1SIM3std4boolE", scope: !3, file: !3, line: 15, type: !5, scopeLine: 15, spFlags: DISPFlagDefinition, unit: !2)
!10 = !DILocation(line: 16, column: 12, scope: !9)
!11 = distinct !DISubprogram(name: "==", linkageName: "_CX1N4maino2eqIM3std4boolEEM3std4bool2_M4main1SIM3std4boolEM4main1SIM3std4boolE", scope: !3, file: !3, line: 7, type: !5, scopeLine: 7, spFlags: DISPFlagDefinition, unit: !2)
