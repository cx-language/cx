%Foo = type { i32 }

define %Foo @Foo.init() {
  %1 = alloca %Foo
  %2 = getelementptr inbounds %Foo, %Foo* %1, i32 0, i32 0
  store i32 42, i32* %2
  %3 = load %Foo, %Foo* %1
  ret %Foo %3
}

define void @bar(%Foo %this) {
  ret void
}

define i32 @qux(%Foo %this) {
  %1 = extractvalue %Foo %this, 0
  ret i32 %1
}

define i32 @main() {
  %foo = alloca %Foo
  %i = alloca i32
  %1 = call %Foo @Foo.init()
  store %Foo %1, %Foo* %foo
  %foo1 = load %Foo, %Foo* %foo
  call void @bar(%Foo %foo1)
  %foo2 = load %Foo, %Foo* %foo
  %2 = call i32 @qux(%Foo %foo2)
  store i32 %2, i32* %i
  ret i32 0
}
