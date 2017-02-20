%Foo = type { i32 }

define void @get(%Foo* %f) {
  %m = alloca i32
  %1 = getelementptr inbounds %Foo, %Foo* %f, i32 0, i32 0
  %2 = load i32, i32* %1
  store i32 %2, i32* %m
  ret void
}

define i32 @main() {
  %f = alloca %Foo
  %rf = alloca %Foo*
  %n = alloca i32
  store %Foo* %f, %Foo** %rf
  %1 = load %Foo*, %Foo** %rf
  %2 = getelementptr inbounds %Foo, %Foo* %1, i32 0, i32 0
  %3 = load i32, i32* %2
  store i32 %3, i32* %n
  ret i32 0
}
