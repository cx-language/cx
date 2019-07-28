
%Foo = type { i32 }

define void @_EN4main3Foo3barE(%Foo* %this) {
  ret void
}

define void @_EN4main3Foo4initE1i3int(%Foo* %this, i32 %i) {
  %i1 = getelementptr inbounds %Foo, %Foo* %this, i32 0, i32 0
  store i32 %i, i32* %i1
  ret void
}

define i32 @main() {
  %f = alloca %Foo
  %rf = alloca %Foo*
  store %Foo* %f, %Foo** %rf
  %rf.load = load %Foo*, %Foo** %rf
  call void @_EN4main3Foo3barE(%Foo* %rf.load)
  ret i32 0
}
