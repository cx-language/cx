
define void @_EN4main1fE3fooP3int3barOPM3int(i32* %foo, i32* %bar) {
  %1 = icmp eq i32* %foo, %bar
  br i1 %1, label %then, label %else

then:                                             ; preds = %0
  br label %endif

else:                                             ; preds = %0
  br label %endif

endif:                                            ; preds = %else, %then
  ret void
}
