
define i32 @main() {
  %foo = alloca i1*
  %b = alloca i1
  %a = alloca i32
  %foo1 = load i1*, i1** %foo
  %1 = icmp ne i1* %foo1, null
  %2 = xor i1 %1, true
  store i1 %2, i1* %b
  %foo2 = load i1*, i1** %foo
  %3 = icmp ne i1* %foo2, null
  br i1 %3, label %then, label %else

then:                                             ; preds = %0
  br label %endif

else:                                             ; preds = %0
  br label %endif

endif:                                            ; preds = %else, %then
  br label %while

while:                                            ; preds = %body, %endif
  %foo3 = load i1*, i1** %foo
  %4 = icmp ne i1* %foo3, null
  br i1 %4, label %body, label %endwhile

body:                                             ; preds = %while
  br label %while

endwhile:                                         ; preds = %while
  %foo4 = load i1*, i1** %foo
  %5 = icmp ne i1* %foo4, null
  br i1 %5, label %then5, label %else6

then5:                                            ; preds = %endwhile
  br label %endif7

else6:                                            ; preds = %endwhile
  br label %endif7

endif7:                                           ; preds = %else6, %then5
  %phi = phi i32 [ 1, %then5 ], [ 2, %else6 ]
  store i32 %phi, i32* %a
  ret i32 0
}
