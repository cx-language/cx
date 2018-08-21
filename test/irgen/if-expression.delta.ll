
declare i1 @a()

declare i1 @b()

declare i1 @c()

declare i1 @d()

declare i1 @e()

define i32 @main() {
  %x = alloca i1
  %1 = call i1 @a()
  br i1 %1, label %then, label %else

then:                                             ; preds = %0
  %2 = call i1 @b()
  br label %endif3

else:                                             ; preds = %0
  %3 = call i1 @c()
  br i1 %3, label %then1, label %else2

then1:                                            ; preds = %else
  %4 = call i1 @d()
  br label %endif

else2:                                            ; preds = %else
  %5 = call i1 @e()
  br label %endif

endif:                                            ; preds = %else2, %then1
  %phi = phi i1 [ %4, %then1 ], [ %5, %else2 ]
  br label %endif3

endif3:                                           ; preds = %endif, %then
  %phi4 = phi i1 [ %2, %then ], [ %phi, %endif ]
  store i1 %phi4, i1* %x
  ret i32 0
}
