
%Foo = type { i32 }

define void @_EN4main3Foo3barE(%Foo* %this) {
  ret void
}

define i32 @main() {
  %f = alloca %Foo*
  store %Foo* null, %Foo** %f
  %f.load = load %Foo*, %Foo** %f
  %i = getelementptr inbounds %Foo, %Foo* %f.load, i32 0, i32 0
  %i.load = load i32, i32* %i
  %f.load1 = load %Foo*, %Foo** %f
  call void @_EN4main3Foo3barE(%Foo* %f.load1)
  %f.load2 = load %Foo*, %Foo** %f
  %f.load3 = load %Foo*, %Foo** %f
  %f.load3.load = load %Foo, %Foo* %f.load3
  store %Foo %f.load3.load, %Foo* %f.load2
  ret i32 0
}
