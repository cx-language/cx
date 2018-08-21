
%Foo = type { i32 }
%Bar = type { i32 }

define void @_ENM4main3Foo6deinitE(%Foo* %this) {
  %i = getelementptr inbounds %Foo, %Foo* %this, i32 0, i32 0
  store i32 0, i32* %i
  call void @_ENM4main3Foo1fE(%Foo* %this)
  call void @_ENM4main3Foo1fE(%Foo* %this)
  ret void
}

define void @_ENM4main3Foo1fE(%Foo* %this) {
  ret void
}

define void @_ENM4main3Bar6deinitE(%Bar* %this) {
  %i = getelementptr inbounds %Bar, %Bar* %this, i32 0, i32 0
  store i32 0, i32* %i
  call void @_ENM4main3Bar1fE(%Bar* %this)
  call void @_ENM4main3Bar1fE(%Bar* %this)
  ret void
}

define void @_ENM4main3Bar1fE(%Bar* %this) {
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
  call void @_ENM4main3Foo6deinitE(%Foo* %f2)
  br label %endif

else:                                             ; preds = %0
  br label %endif

endif:                                            ; preds = %else, %then
  br label %while

while:                                            ; preds = %endif
  br i1 false, label %body, label %endwhile

body:                                             ; preds = %while
  call void @_ENM4main3Bar6deinitE(%Bar* %b2)
  call void @_ENM4main3Bar6deinitE(%Bar* %b)
  call void @_ENM4main3Foo6deinitE(%Foo* %f)
  ret i32 0

endwhile:                                         ; preds = %while
  store i32 1, i32* %i
  call void @_ENM4main3Bar6deinitE(%Bar* %b)
  call void @_ENM4main3Foo6deinitE(%Foo* %f)
  ret i32 0
}
