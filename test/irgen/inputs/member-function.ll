%Foo = type { i32 }

define %Foo @__init_Foo() {
  %1 = alloca %Foo
  store %Foo undef, %Foo* %1
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

define void @main() {
  %foo = alloca %Foo
  %1 = call %Foo @__init_Foo()
  store %Foo %1, %Foo* %foo
  %foo1 = load %Foo, %Foo* %foo
  call void @bar(%Foo %foo1)
  ret void
}
