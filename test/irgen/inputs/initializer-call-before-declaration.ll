%Foo = type { float }

define i32 @main() {
  %f = alloca %Foo
  %1 = call %Foo @Foo.init(i32 5)
  store %Foo %1, %Foo* %f
  %2 = load %Foo, %Foo* %f
  call void @Foo.deinit(%Foo %2)
  ret i32 0
}

define void @Foo.deinit(%Foo %this) {
  ret void
}

define %Foo @Foo.init(i32 %i) {
  %1 = alloca %Foo
  %2 = load %Foo, %Foo* %1
  ret %Foo %2
}
