
%Foo = type { i32 }
%Bar = type { i32 }

define void @_EN4main3Foo7doStuffE(%Foo* %this) {
  ret void
}

define void @_EN4main3Foo4initE1i3int(%Foo* %this, i32 %i) {
  %i1 = getelementptr inbounds %Foo, %Foo* %this, i32 0, i32 0
  store i32 %i, i32* %i1
  ret void
}

define void @_EN4main3Bar7doStuffE(%Bar* %this) {
  ret void
}

define void @_EN4main3Bar4initE1i3int(%Bar* %this, i32 %i) {
  %i1 = getelementptr inbounds %Bar, %Bar* %this, i32 0, i32 0
  store i32 %i, i32* %i1
  ret void
}
