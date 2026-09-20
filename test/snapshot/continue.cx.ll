
define void @_EN4main3bazE4bool(i1 %foo) {
  %foo1 = alloca i1, align 1
  store i1 %foo, ptr %foo1, align 1
  br label %loop.condition

loop.condition:                                   ; preds = %if.end, %if.then, %0
  %foo.load = load i1, ptr %foo1, align 1
  br i1 %foo.load, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  %foo.load2 = load i1, ptr %foo1, align 1
  br i1 %foo.load2, label %if.then, label %if.else

loop.end:                                         ; preds = %loop.condition
  ret void

if.then:                                          ; preds = %loop.body
  %foo.load3 = load i1, ptr %foo1, align 1
  call void @_EN4main3bazE4bool(i1 %foo.load3)
  br label %loop.condition

if.else:                                          ; preds = %loop.body
  br label %if.end

if.end:                                           ; preds = %if.else
  br label %loop.condition
}
