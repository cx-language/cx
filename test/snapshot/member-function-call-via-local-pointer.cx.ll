
%Foo = type { i32 }

define i32 @main() {
  %f = alloca %Foo, align 8
  %rf = alloca %Foo*, align 8
  store %Foo* %f, %Foo** %rf, align 8
  %rf.load = load %Foo*, %Foo** %rf, align 8
  call void @_EN4main3Foo3barE(%Foo* %rf.load)
  ret i32 0
}

define void @_EN4main3Foo3barE(%Foo* %this) {
  ret void
}
