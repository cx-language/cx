
define i32 @main() {
  %foo = alloca i1*
  %b = alloca i1
  %a = alloca i32
  %foo.load = load i1*, i1** %foo
  %1 = icmp ne i1* %foo.load, null
  %2 = xor i1 %1, true
  store i1 %2, i1* %b
  %foo.load1 = load i1*, i1** %foo
  %3 = icmp ne i1* %foo.load1, null
  br i1 %3, label %if.then, label %if.else

if.then:                                          ; preds = %0
  br label %if.end

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  br label %loop.condition

loop.condition:                                   ; preds = %loop.body, %if.end
  %foo.load2 = load i1*, i1** %foo
  %4 = icmp ne i1* %foo.load2, null
  br i1 %4, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  br label %loop.condition

loop.end:                                         ; preds = %loop.condition
  %foo.load3 = load i1*, i1** %foo
  %5 = icmp ne i1* %foo.load3, null
  br i1 %5, label %if.then4, label %if.else5

if.then4:                                         ; preds = %loop.end
  br label %if.end6

if.else5:                                         ; preds = %loop.end
  br label %if.end6

if.end6:                                          ; preds = %if.else5, %if.then4
  %phi = phi i32 [ 1, %if.then4 ], [ 2, %if.else5 ]
  store i32 %phi, i32* %a
  ret i32 0
}
