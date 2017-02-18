declare void @foo()

declare void @bar()

define i32 @main() {
  br i1 false, label %then, label %else

then:                                             ; preds = %0
  call void @bar()
  call void @foo()
  ret i32 0

else:                                             ; preds = %0
  br label %endif

endif:                                            ; preds = %else
  br i1 false, label %then1, label %else2

then1:                                            ; preds = %endif
  call void @bar()
  br label %endif3

else2:                                            ; preds = %endif
  br label %endif3

endif3:                                           ; preds = %else2, %then1
  br label %while

while:                                            ; preds = %body, %endif3
  br i1 false, label %body, label %endwhile

body:                                             ; preds = %while
  call void @bar()
  call void @foo()
  br label %while

endwhile:                                         ; preds = %while
  br i1 true, label %then4, label %else5

then4:                                            ; preds = %endwhile
  call void @bar()
  call void @foo()
  ret i32 0

else5:                                            ; preds = %endwhile
  br label %endif6

endif6:                                           ; preds = %else5
  call void @foo()
  call void @bar()
  call void @foo()
  ret i32 0
}

define void @shouldBeEmpty() {
  ret void
}
