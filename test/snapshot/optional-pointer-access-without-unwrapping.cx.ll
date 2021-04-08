
%Foo = type { i32 }

define i32 @main() {
  %f = alloca %Foo*, align 8
  %a = alloca i32, align 4
  %b = alloca i32*, align 8
  %c = alloca i32*, align 8
  store %Foo* null, %Foo** %f, align 8
  %f.load = load %Foo*, %Foo** %f, align 8
  %i = getelementptr inbounds %Foo, %Foo* %f.load, i32 0, i32 0
  %f.load1 = load %Foo*, %Foo** %f, align 8
  %i2 = getelementptr inbounds %Foo, %Foo* %f.load1, i32 0, i32 0
  %i.load = load i32, i32* %i2, align 4
  store i32 %i.load, i32* %a, align 4
  %f.load3 = load %Foo*, %Foo** %f, align 8
  %i4 = getelementptr inbounds %Foo, %Foo* %f.load3, i32 0, i32 0
  store i32* %i4, i32** %b, align 8
  %f.load5 = load %Foo*, %Foo** %f, align 8
  %i6 = getelementptr inbounds %Foo, %Foo* %f.load5, i32 0, i32 0
  store i32* %i6, i32** %c, align 8
  %f.load7 = load %Foo*, %Foo** %f, align 8
  call void @_EN4main3Foo3barE(%Foo* %f.load7)
  %f.load8 = load %Foo*, %Foo** %f, align 8
  %f.load9 = load %Foo*, %Foo** %f, align 8
  %f.load.load = load %Foo, %Foo* %f.load9, align 4
  store %Foo %f.load.load, %Foo* %f.load8, align 4
  ret i32 0
}

define void @_EN4main3Foo3barE(%Foo* %this) {
  ret void
}
