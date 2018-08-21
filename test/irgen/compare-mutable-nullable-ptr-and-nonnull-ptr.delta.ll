
define i32 @main() {
  %foo = alloca i32*
  %bar = alloca i32*
  %foo1 = load i32*, i32** %foo
  %bar2 = load i32*, i32** %bar
  %1 = icmp eq i32* %foo1, %bar2
  br i1 %1, label %then, label %else

then:                                             ; preds = %0
  br label %endif

else:                                             ; preds = %0
  br label %endif

endif:                                            ; preds = %else, %then
  ret i32 0
}
