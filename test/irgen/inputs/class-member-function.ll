%Foo = type { i32 }

define %Foo @__init_Foo() {
  %1 = alloca %Foo
  store %Foo undef, %Foo* %1
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
  %1 = call %Foo @__init_Foo()
  store %Foo %1, %Foo* %foo
  call void @bar(%Foo* %foo)
  %i = alloca i32
  %2 = call i32 @qux(%Foo* %foo)
  store i32 %2, i32* %i
  ret i32 0
}
