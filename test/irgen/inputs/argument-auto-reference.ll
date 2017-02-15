%Foo = type { i32 }

define void @ref(%Foo* %r) {
  ret void
}

define void @ptr(%Foo* %p) {
  ret void
}

define i32 @main() {
  %f = alloca %Foo
  call void @ref(%Foo* %f)
  call void @ptr(%Foo* %f)
  ret i32 0
}
