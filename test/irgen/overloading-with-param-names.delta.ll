
%Foo = type { i32 }

define void @_EN4main3Foo4initE1i3int(%Foo* %this, i32 %i) {
  ret void
}

define void @_EN4main3Foo4initE3qux3int(%Foo* %this, i32 %qux) {
  ret void
}

define void @_EN4main3Foo3fooE1i3int(%Foo* %this, i32 %i) {
  ret void
}

define void @_EN4main3Foo3fooE3qux3int(%Foo* %this, i32 %qux) {
  ret void
}

define void @_EN4main3fooE1i3int(i32 %i) {
  ret void
}

define void @_EN4main3fooE3qux3int(i32 %qux) {
  ret void
}
