define void @foo(i32 %i) {
  switch i32 %i, label %default [
    i32 0, label %1
    i32 -1, label %2
    i32 42, label %3
  ]

; <label>:1:                                      ; preds = %0
  br label %endswitch

; <label>:2:                                      ; preds = %0
  call void @foo(i32 %i)
  call void @foo(i32 %i)
  br label %endswitch

; <label>:3:                                      ; preds = %0
  br label %endswitch

default:                                          ; preds = %0
  br label %endswitch

endswitch:                                        ; preds = %default, %3, %2, %1
  switch i32 %i, label %default1 [
    i32 0, label %4
  ]

; <label>:4:                                      ; preds = %endswitch
  br label %endswitch2

default1:                                         ; preds = %endswitch
  call void @foo(i32 %i)
  br label %endswitch2

endswitch2:                                       ; preds = %default1, %4
  ret void
}
