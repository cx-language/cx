%Foo = type { i32 }
%Bar = type { i32 }

define void @Foo.foo(%Foo* %this) {
  ret void
}

define void @Bar.bar(%Bar %this) {
  ret void
}

define void @qux(%Foo* %f, %Bar* %b) {
  call void @Foo.foo(%Foo* %f)
  %1 = load %Bar, %Bar* %b
  call void @Bar.bar(%Bar %1)
  ret void
}
