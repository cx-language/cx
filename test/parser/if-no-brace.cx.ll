
define i32 @main() {
  br i1 false, label %if.then, label %if.else

if.then:                                          ; preds = %0
  ret i32 1

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else
  br i1 true, label %if.then1, label %if.else2

if.then1:                                         ; preds = %if.end
  ret i32 2

if.else2:                                         ; preds = %if.end
  br label %if.end3

if.end3:                                          ; preds = %if.else2
  ret i32 3
}
