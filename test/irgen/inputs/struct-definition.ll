%Foo = type { i32, i1 }

define %Foo @__init_Foo(i32 %a, i1 %b) {
  %1 = alloca %Foo
  store %Foo undef, %Foo* %1
  %2 = load %Foo, %Foo* %1
  ret %Foo %2
}

define void @main() {
  %f = alloca %Foo
  %1 = call %Foo @__init_Foo(i32 666, i1 true)
  store %Foo %1, %Foo* %f
  ret void
}
