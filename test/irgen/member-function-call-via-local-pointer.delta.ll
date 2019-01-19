
%Foo = type { i32 }

define void @_EN4main3Foo3barE(%Foo* %this) {
  ret void
}

define i32 @main() {
  %f = alloca %Foo
  %rf = alloca %Foo*
  store %Foo* %f, %Foo** %rf
  %rf.load = load %Foo*, %Foo** %rf
  call void @_EN4main3Foo3barE(%Foo* %rf.load)
  ret i32 0
}
