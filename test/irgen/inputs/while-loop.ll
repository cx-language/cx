declare i1 @foo()

define void @main() {
  br label %while

while:                                            ; preds = %body, %0
  %1 = call i1 @foo()
  br i1 %1, label %body, label %endwhile

body:                                             ; preds = %while
  %2 = call i1 @foo()
  br label %while

endwhile:                                         ; preds = %while
  ret void
}
