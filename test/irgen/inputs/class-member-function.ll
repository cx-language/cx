%Foo = type { i32 }

define %Foo @Foo.init() {
  %1 = alloca %Foo
  %2 = getelementptr inbounds %Foo, %Foo* %1, i32 0, i32 0
  store i32 42, i32* %2
  %3 = load %Foo, %Foo* %1
  ret %Foo %3
}

define void @bar(%Foo* %this) {
  %1 = getelementptr inbounds %Foo, %Foo* %this, i32 0, i32 0
  %2 = load i32, i32* %1
  %3 = add i32 %2, 1
  store i32 %3, i32* %1
  ret void
}

define i32 @qux(%Foo* %this) {
  %1 = getelementptr inbounds %Foo, %Foo* %this, i32 0, i32 0
  %2 = load i32, i32* %1
  ret i32 %2
}

define i32 @main() {
  %foo = alloca %Foo
  %i = alloca i32
  %1 = call %Foo @Foo.init()
  store %Foo %1, %Foo* %foo
  call void @bar(%Foo* %foo)
  %2 = call i32 @qux(%Foo* %foo)
  store i32 %2, i32* %i
  ret i32 0
}
