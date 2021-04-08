
%Foo = type { i32 }

define void @_EN4main3getEP3Foo(%Foo* %f) {
  %m = alloca i32, align 4
  %i = getelementptr inbounds %Foo, %Foo* %f, i32 0, i32 0
  %i.load = load i32, i32* %i, align 4
  store i32 %i.load, i32* %m, align 4
  ret void
}

define i32 @main() {
  %f = alloca %Foo, align 8
  %rf = alloca %Foo*, align 8
  %n = alloca i32, align 4
  store %Foo* %f, %Foo** %rf, align 8
  %rf.load = load %Foo*, %Foo** %rf, align 8
  %i = getelementptr inbounds %Foo, %Foo* %rf.load, i32 0, i32 0
  %i.load = load i32, i32* %i, align 4
  store i32 %i.load, i32* %n, align 4
  %rf.load1 = load %Foo*, %Foo** %rf, align 8
  call void @_EN4main3getEP3Foo(%Foo* %rf.load1)
  ret i32 0
}
