
declare void @foo()

declare void @bar()

define i32 @main() {
  br i1 false, label %then, label %else

then:                                             ; preds = %0
  call void @foo()
  br label %endif

else:                                             ; preds = %0
  call void @bar()
  br label %endif

endif:                                            ; preds = %else, %then
  ret i32 0
}
