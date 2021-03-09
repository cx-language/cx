
%Foo = type { i32 }
%Bar = type { i32 }

define i32 @main() {
  %f = alloca %Foo, align 8
  %f2 = alloca %Foo, align 8
  %b = alloca %Bar, align 8
  %b2 = alloca %Bar, align 8
  %i = alloca i32, align 4
  br i1 false, label %if.then, label %if.else

if.then:                                          ; preds = %0
  call void @_EN4main3Foo6deinitE(%Foo* %f2)
  br label %if.end

if.else:                                          ; preds = %0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  br label %loop.condition

loop.condition:                                   ; preds = %if.end
  br i1 false, label %loop.body, label %loop.end

loop.body:                                        ; preds = %loop.condition
  call void @_EN4main3Bar6deinitE(%Bar* %b2)
  call void @_EN4main3Bar6deinitE(%Bar* %b)
  call void @_EN4main3Foo6deinitE(%Foo* %f)
  ret i32 0

loop.end:                                         ; preds = %loop.condition
  store i32 1, i32* %i, align 4
  call void @_EN4main3Bar6deinitE(%Bar* %b)
  call void @_EN4main3Foo6deinitE(%Foo* %f)
  ret i32 0
}

define void @_EN4main3Foo6deinitE(%Foo* %this) {
  %i = getelementptr inbounds %Foo, %Foo* %this, i32 0, i32 0
  store i32 0, i32* %i, align 4
  call void @_EN4main3Foo1fE(%Foo* %this)
  call void @_EN4main3Foo1fE(%Foo* %this)
  ret void
}

define void @_EN4main3Bar6deinitE(%Bar* %this) {
  %i = getelementptr inbounds %Bar, %Bar* %this, i32 0, i32 0
  store i32 0, i32* %i, align 4
  call void @_EN4main3Bar1fE(%Bar* %this)
  call void @_EN4main3Bar1fE(%Bar* %this)
  ret void
}

define void @_EN4main3Foo1fE(%Foo* %this) {
  ret void
}

define void @_EN4main3Bar1fE(%Bar* %this) {
  ret void
}
