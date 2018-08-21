
@x = private global i32 0

define i32 @main() {
  %foo = alloca i32
  store i32 2, i32* %foo
  %foo1 = load i32, i32* %foo
  %1 = icmp eq i32 %foo1, 1
  br i1 %1, label %then, label %else

then:                                             ; preds = %0
  br label %endif

else:                                             ; preds = %0
  br label %endif

endif:                                            ; preds = %else, %then
  %foo2 = load i32, i32* %foo
  %2 = icmp ne i32 %foo2, 0
  br i1 %2, label %then3, label %else4

then3:                                            ; preds = %endif
  br label %endif5

else4:                                            ; preds = %endif
  br label %endif5

endif5:                                           ; preds = %else4, %then3
  %foo6 = load i32, i32* %foo
  switch i32 %foo6, label %default [
    i32 0, label %3
    i32 1, label %4
    i32 2, label %5
  ]

; <label>:3:                                      ; preds = %endif5
  ret i32 0

; <label>:4:                                      ; preds = %endif5
  ret i32 0

; <label>:5:                                      ; preds = %endif5
  ret i32 0

default:                                          ; preds = %endif5
  br label %endswitch

endswitch:                                        ; preds = %default
  ret i32 0
}
