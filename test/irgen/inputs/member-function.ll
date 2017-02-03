%Foo = type { i32 }

define %Foo @__init_Foo() {
  %1 = alloca %Foo
  store %Foo undef, %Foo* %1
  %2 = load %Foo, %Foo* %1
  ret %Foo %2
}

define void @bar(%Foo %this) {
  ret void
}

define void @main() {
  %foo = alloca %Foo
  %1 = call %Foo @__init_Foo()
  store %Foo %1, %Foo* %foo
  %foo1 = load %Foo, %Foo* %foo
  call void @bar(%Foo %foo1)
  ret void
}
