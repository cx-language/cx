
%Foo = type { i32 }

define void @_EN4main3Foo3barE(%Foo* %this) {
  ret void
}

define i32 @main() {
  %f = alloca %Foo*
  store %Foo* null, %Foo** %f
  %1 = load %Foo*, %Foo** %f
  %i = getelementptr inbounds %Foo, %Foo* %1, i32 0, i32 0
  %2 = load i32, i32* %i
  %f1 = load %Foo*, %Foo** %f
  call void @_EN4main3Foo3barE(%Foo* %f1)
  %f2 = load %Foo*, %Foo** %f
  %f3 = load %Foo*, %Foo** %f
  %3 = load %Foo, %Foo* %f3
  store %Foo %3, %Foo* %f2
  ret i32 0
}
