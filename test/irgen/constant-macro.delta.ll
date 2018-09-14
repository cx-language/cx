
define void @_EN4main3fooE1i3int(i32 %i) {
  switch i32 %i, label %default [
    i32 42, label %1
  ]

; <label>:1:                                      ; preds = %0
  ret void

default:                                          ; preds = %0
  br label %endswitch

endswitch:                                        ; preds = %default
  ret void
}
