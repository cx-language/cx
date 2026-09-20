
%Foo = type { i32 }

define void @_EN4main3fooE1i3int(i32 %i) {
  %i1 = alloca i32, align 4
  %1 = alloca %Foo, align 8
  %2 = alloca %Foo, align 8
  store i32 %i, ptr %i1, align 4
  %i.load = load i32, ptr %i1, align 4
  call void @_EN4main3Foo4initE1i3int(ptr %1, i32 %i.load)
  %i.load2 = load i32, ptr %i1, align 4
  call void @_EN4main3Foo3fooE1i3int(ptr %1, i32 %i.load2)
  %i.load3 = load i32, ptr %i1, align 4
  call void @_EN4main3Foo4initE3qux3int(ptr %2, i32 %i.load3)
  %i.load4 = load i32, ptr %i1, align 4
  call void @_EN4main3Foo3fooE3qux3int(ptr %2, i32 %i.load4)
  ret void
}

define void @_EN4main3Foo4initE1i3int(ptr %this, i32 %i) {
  %i1 = alloca i32, align 4
  store i32 %i, ptr %i1, align 4
  ret void
}

define void @_EN4main3Foo3fooE1i3int(ptr %this, i32 %i) {
  %i1 = alloca i32, align 4
  store i32 %i, ptr %i1, align 4
  ret void
}

define void @_EN4main3Foo4initE3qux3int(ptr %this, i32 %qux) {
  %qux1 = alloca i32, align 4
  store i32 %qux, ptr %qux1, align 4
  ret void
}

define void @_EN4main3Foo3fooE3qux3int(ptr %this, i32 %qux) {
  %qux1 = alloca i32, align 4
  store i32 %qux, ptr %qux1, align 4
  ret void
}

define void @_EN4main3fooE3qux3int(i32 %qux) {
  %qux1 = alloca i32, align 4
  store i32 %qux, ptr %qux1, align 4
  ret void
}
