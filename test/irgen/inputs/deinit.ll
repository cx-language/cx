%Foo = type { i32 }

define void @__deinit_Foo(%Foo* %this) {
  ret void
}

define i32 @main() {
  %f = alloca %Foo
  %i = alloca i32
  store i32 1, i32* %i
  call void @__deinit_Foo(%Foo* %f)
  ret i32 0
}
