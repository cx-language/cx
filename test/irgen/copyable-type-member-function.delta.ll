
%Foo = type { i32 }

define void @_ENM4main3Foo4initE(%Foo* %this) {
  %baz = getelementptr inbounds %Foo, %Foo* %this, i32 0, i32 0
  store i32 42, i32* %baz
  ret void
}

define void @_EN4main3Foo3barE(%Foo %this) {
  ret void
}

define i32 @_EN4main3Foo3quxE(%Foo %this) {
  %baz = extractvalue %Foo %this, 0
  ret i32 %baz
}

define i32 @main() {
  %foo = alloca %Foo
  %i = alloca i32
  call void @_ENM4main3Foo4initE(%Foo* %foo)
  %foo.load = load %Foo, %Foo* %foo
  call void @_EN4main3Foo3barE(%Foo %foo.load)
  %foo.load1 = load %Foo, %Foo* %foo
  %1 = call i32 @_EN4main3Foo3quxE(%Foo %foo.load1)
  store i32 %1, i32* %i
  ret i32 0
}
