%Foo = type { i32 }
%Bar = type { i32 }

define void @Foo.doStuff(%Foo* %this) {
  ret void
}

define void @Bar.doStuff(%Bar* %this) {
  ret void
}
