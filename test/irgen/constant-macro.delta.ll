
define i32 @main() {
  %i = alloca i32
  store i32 4, i32* %i
  %i1 = load i32, i32* %i
  switch i32 %i1, label %default [
    i32 42, label %1
  ]

; <label>:1:                                      ; preds = %0
  ret i32 0

default:                                          ; preds = %0
  br label %endswitch

endswitch:                                        ; preds = %default
  ret i32 0
}
