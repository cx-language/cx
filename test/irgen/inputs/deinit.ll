%Foo = type { i32 }
%Bar = type { i32 }

define void @Foo.deinit(%Foo* %this) {
  ret void
}

define void @Bar.deinit(%Bar %this) {
  ret void
}

define i32 @main() {
  %f = alloca %Foo
  %f2 = alloca %Foo
  %b = alloca %Bar
  %b2 = alloca %Bar
  %i = alloca i32
  br i1 false, label %then, label %else

then:                                             ; preds = %0
  call void @Foo.deinit(%Foo* %f2)
  br label %endif

else:                                             ; preds = %0
  br label %endif

endif:                                            ; preds = %else, %then
  br label %while

while:                                            ; preds = %endif
  br i1 false, label %body, label %endwhile

body:                                             ; preds = %while
  %1 = load %Bar, %Bar* %b2
  call void @Bar.deinit(%Bar %1)
  %2 = load %Bar, %Bar* %b
  call void @Bar.deinit(%Bar %2)
  call void @Foo.deinit(%Foo* %f)
  ret i32 0

endwhile:                                         ; preds = %while
  store i32 1, i32* %i
  %3 = load %Bar, %Bar* %b
  call void @Bar.deinit(%Bar %3)
  call void @Foo.deinit(%Foo* %f)
  ret i32 0
}
