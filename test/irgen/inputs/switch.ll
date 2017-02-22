define void @foo(i32 %i) {
  switch i32 %i, label %endswitch [
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

endswitch:                                        ; preds = %3, %2, %1, %0
  ret void
}
