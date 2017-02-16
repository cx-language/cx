%Foo = type { i32 }
%Bar = type { i32 }

define void @foo(%Foo* %this) {
  ret void
}

define void @bar(%Bar %this) {
  ret void
}

define void @qux(%Foo* %f, %Bar* %b) {
  call void @foo(%Foo* %f)
  %1 = load %Bar, %Bar* %b
  call void @bar(%Bar %1)
  ret void
}
