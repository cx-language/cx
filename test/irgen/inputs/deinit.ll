%Foo = type { i32 }
%Bar = type { i32 }

define void @__deinit_Foo(%Foo* %this) {
  ret void
}

define void @__deinit_Bar(%Bar %this) {
  ret void
}

define i32 @main() {
  %f = alloca %Foo
  %b = alloca %Bar
  %i = alloca i32
  store i32 1, i32* %i
  %1 = load %Bar, %Bar* %b
  call void @__deinit_Bar(%Bar %1)
  call void @__deinit_Foo(%Foo* %f)
  ret i32 0
}
