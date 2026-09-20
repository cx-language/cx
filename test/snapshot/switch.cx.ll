
%S = type { i32 }

define void @_EN4main3fooE3int(i32 %i) {
  %i1 = alloca i32, align 4
  store i32 %i, ptr %i1, align 4
  %i.load = load i32, ptr %i1, align 4
  switch i32 %i.load, label %switch.default [
    i32 0, label %switch.case.0
    i32 -1, label %switch.case.1
    i32 42, label %switch.case.2
  ]

switch.case.0:                                    ; preds = %0
  br label %switch.end

switch.case.1:                                    ; preds = %0
  %i.load2 = load i32, ptr %i1, align 4
  call void @_EN4main3fooE3int(i32 %i.load2)
  %i.load3 = load i32, ptr %i1, align 4
  call void @_EN4main3fooE3int(i32 %i.load3)
  br label %switch.end

switch.case.2:                                    ; preds = %0
  br label %switch.end

switch.default:                                   ; preds = %0
  br label %switch.end

switch.end:                                       ; preds = %switch.default, %switch.case.2, %switch.case.1, %switch.case.0
  %i.load4 = load i32, ptr %i1, align 4
  switch i32 %i.load4, label %switch.default6 [
    i32 0, label %switch.case.05
  ]

switch.case.05:                                   ; preds = %switch.end
  br label %switch.end8

switch.default6:                                  ; preds = %switch.end
  %i.load7 = load i32, ptr %i1, align 4
  call void @_EN4main3fooE3int(i32 %i.load7)
  br label %switch.end8

switch.end8:                                      ; preds = %switch.default6, %switch.case.05
  ret void
}

define i32 @_EN4main1fEP1S(ptr %s) {
  %s1 = alloca ptr, align 8
  store ptr %s, ptr %s1, align 8
  %s.load = load ptr, ptr %s1, align 8
  %i = getelementptr inbounds %S, ptr %s.load, i32 0, i32 0
  %i.load = load i32, ptr %i, align 4
  switch i32 %i.load, label %switch.default [
    i32 0, label %switch.case.0
  ]

switch.case.0:                                    ; preds = %0
  ret i32 0

switch.default:                                   ; preds = %0
  ret i32 1
}
