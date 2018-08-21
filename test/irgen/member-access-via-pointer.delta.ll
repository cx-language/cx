
%Foo = type { i32 }

define void @_EN4main3getE1fP3Foo(%Foo* %f) {
  %m = alloca i32
  %i = getelementptr inbounds %Foo, %Foo* %f, i32 0, i32 0
  %1 = load i32, i32* %i
  store i32 %1, i32* %m
  ret void
}

define i32 @main() {
  %f = alloca %Foo
  %rf = alloca %Foo*
  %n = alloca i32
  store %Foo* %f, %Foo** %rf
  %1 = load %Foo*, %Foo** %rf
  %i = getelementptr inbounds %Foo, %Foo* %1, i32 0, i32 0
  %2 = load i32, i32* %i
  store i32 %2, i32* %n
  %rf1 = load %Foo*, %Foo** %rf
  call void @_EN4main3getE1fP3Foo(%Foo* %rf1)
  ret i32 0
}
