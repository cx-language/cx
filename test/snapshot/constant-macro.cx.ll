
define void @_EN4main3fooE3int(i32 %i) {
  %i1 = alloca i32, align 4
  store i32 %i, ptr %i1, align 4
  %i.load = load i32, ptr %i1, align 4
  switch i32 %i.load, label %switch.default [
    i32 42, label %switch.case.0
  ]

switch.case.0:                                    ; preds = %0
  ret void

switch.default:                                   ; preds = %0
  br label %switch.end

switch.end:                                       ; preds = %switch.default
  ret void
}
