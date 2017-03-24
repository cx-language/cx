define i32 @main() {
  br label %while

while:                                            ; preds = %0
  br i1 true, label %body, label %endwhile

body:                                             ; preds = %while
  br i1 true, label %then, label %else

endwhile:                                         ; preds = %endif, %then, %while
  switch i32 1, label %default [
  ]

then:                                             ; preds = %body
  br label %endwhile

else:                                             ; preds = %body
  br label %endif

endif:                                            ; preds = %else
  br label %endwhile

default:                                          ; preds = %endwhile
  br label %endswitch

endswitch:                                        ; preds = %default
  ret i32 0
}
