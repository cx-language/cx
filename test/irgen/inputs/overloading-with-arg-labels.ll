%Foo = type { i32 }

define %Foo @Foo.init(i32 %i) {
  %1 = alloca %Foo
  %2 = load %Foo, %Foo* %1
  ret %Foo %2
}

define %Foo @"Foo.init$qux"(i32 %i) {
  %1 = alloca %Foo
  %2 = load %Foo, %Foo* %1
  ret %Foo %2
}

define void @foo(i32 %i) {
  ret void
}

define void @"foo$qux"(i32 %i) {
  ret void
}

define void @Foo.foo(%Foo* %this, i32 %i) {
  ret void
}

define void @"Foo.foo$qux"(%Foo* %this, i32 %i) {
  ret void
}
