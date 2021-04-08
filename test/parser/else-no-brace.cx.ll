
define i32 @main() {
  br i1 true, label %if.then, label %if.else

if.then:                                          ; preds = %0
  ret i32 1

if.else:                                          ; preds = %0
  ret i32 0
}
