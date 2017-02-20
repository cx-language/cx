%Foo = type { i32 }
%Bar = type { i32 }

define void @__deinit_Foo(%Foo* %this) {
  ret void
}

define void @__deinit_Bar(%Bar %this) {
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
  call void @__deinit_Foo(%Foo* %f2)
  br label %endif

else:                                             ; preds = %0
  br label %endif

endif:                                            ; preds = %else, %then
  br label %while

while:                                            ; preds = %endif
  br i1 false, label %body, label %endwhile

body:                                             ; preds = %while
  %1 = load %Bar, %Bar* %b2
  call void @__deinit_Bar(%Bar %1)
  %2 = load %Bar, %Bar* %b
  call void @__deinit_Bar(%Bar %2)
  call void @__deinit_Foo(%Foo* %f)
  ret i32 0

endwhile:                                         ; preds = %while
  store i32 1, i32* %i
  %3 = load %Bar, %Bar* %b
  call void @__deinit_Bar(%Bar %3)
  call void @__deinit_Foo(%Foo* %f)
  ret i32 0
}
