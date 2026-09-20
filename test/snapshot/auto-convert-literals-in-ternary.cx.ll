
define i8 @_EN4main3fooE4bool(i1 %b) {
  %b1 = alloca i1, align 1
  store i1 %b, ptr %b1, align 1
  %b.load = load i1, ptr %b1, align 1
  br i1 %b.load, label %if.then, label %if.else

if.then:                                          ; preds = %0
  br label %if.end

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %if.result = phi i8 [ -1, %if.then ], [ -18, %if.else ]
  ret i8 %if.result
}
