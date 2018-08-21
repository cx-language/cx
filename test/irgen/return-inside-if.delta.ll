
declare i1 @foo()

define void @_EN4main3barE() {
  %1 = call i1 @foo()
  br i1 %1, label %then, label %else

then:                                             ; preds = %0
  ret void

else:                                             ; preds = %0
  ret void

endif:                                            ; No predecessors!
  %2 = call i1 @foo()
  br i1 %2, label %then1, label %else2

then1:                                            ; preds = %endif
  ret void

else2:                                            ; preds = %endif
  ret void

endif3:                                           ; No predecessors!
  br label %while

while:                                            ; preds = %endif3
  %3 = call i1 @foo()
  br i1 %3, label %body, label %endwhile

body:                                             ; preds = %while
  ret void

endwhile:                                         ; preds = %while
  br label %while4

while4:                                           ; preds = %endwhile
  %4 = call i1 @foo()
  br i1 %4, label %body5, label %endwhile6

body5:                                            ; preds = %while4
  ret void

endwhile6:                                        ; preds = %while4
  ret void
}
