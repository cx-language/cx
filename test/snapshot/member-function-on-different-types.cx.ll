
%Foo = type { i32 }
%Bar = type { i32 }

define i32 @main() {
  %1 = alloca %Foo, align 8
  %2 = alloca %Bar, align 8
  call void @_EN4main3Foo4initE3int(%Foo* %1, i32 1)
  call void @_EN4main3Foo7doStuffE(%Foo* %1)
  call void @_EN4main3Bar4initE3int(%Bar* %2, i32 2)
  call void @_EN4main3Bar7doStuffE(%Bar* %2)
  ret i32 0
}

define void @_EN4main3Foo4initE3int(%Foo* %this, i32 %i) {
  %i1 = getelementptr inbounds %Foo, %Foo* %this, i32 0, i32 0
  store i32 %i, i32* %i1, align 4
  ret void
}

define void @_EN4main3Foo7doStuffE(%Foo* %this) {
  ret void
}

define void @_EN4main3Bar4initE3int(%Bar* %this, i32 %i) {
  %i1 = getelementptr inbounds %Bar, %Bar* %this, i32 0, i32 0
  store i32 %i, i32* %i1, align 4
  ret void
}

define void @_EN4main3Bar7doStuffE(%Bar* %this) {
  ret void
}
