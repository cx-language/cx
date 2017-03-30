%Foo = type { i32 }

define void @Foo.bar(%Foo* %this) {
  ret void
}

define i32 @main() {
  %f = alloca %Foo
  %rf = alloca %Foo*
  store %Foo* %f, %Foo** %rf
  %rf1 = load %Foo*, %Foo** %rf
  call void @Foo.bar(%Foo* %rf1)
  ret i32 0
}
