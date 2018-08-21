
define void @_EN4main3bazE3foo4bool(i1 %foo) {
  br label %while

while:                                            ; preds = %endif, %then, %0
  br i1 %foo, label %body, label %endwhile

body:                                             ; preds = %while
  br i1 %foo, label %then, label %else

endwhile:                                         ; preds = %while
  ret void

then:                                             ; preds = %body
  call void @_EN4main3bazE3foo4bool(i1 %foo)
  br label %while

else:                                             ; preds = %body
  br label %endif

endif:                                            ; preds = %else
  br label %while
}
