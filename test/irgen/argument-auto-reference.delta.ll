
%Foo = type { i32 }

define void @_EN4main3ptrE1pP3Foo(%Foo* %p) {
  ret void
}

define i32 @main() {
  %f = alloca %Foo
  call void @_EN4main3ptrE1pP3Foo(%Foo* %f)
  ret i32 0
}
