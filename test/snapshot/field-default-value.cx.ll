
%Foo = type { i32 }
%Bar = type { i32, i32 }

define i32 @main() {
  %foo = alloca %Foo, align 8
  %bar = alloca %Bar, align 8
  call void @_EN4main3Foo4initE3int(ptr %foo, i32 42)
  call void @_EN4main3Bar4initE3int3int(ptr %bar, i32 -1, i32 42)
  ret i32 0
}

define void @_EN4main3Foo4initE3int(ptr %this, i32 %i) {
  %i1 = alloca i32, align 4
  store i32 %i, ptr %i1, align 4
  %i2 = getelementptr inbounds %Foo, ptr %this, i32 0, i32 0
  %i.load = load i32, ptr %i1, align 4
  store i32 %i.load, ptr %i2, align 4
  ret void
}

define void @_EN4main3Bar4initE3int3int(ptr %this, i32 %j, i32 %i) {
  %j1 = alloca i32, align 4
  %i2 = alloca i32, align 4
  store i32 %j, ptr %j1, align 4
  store i32 %i, ptr %i2, align 4
  %i3 = getelementptr inbounds %Bar, ptr %this, i32 0, i32 0
  %i.load = load i32, ptr %i2, align 4
  store i32 %i.load, ptr %i3, align 4
  %j4 = getelementptr inbounds %Bar, ptr %this, i32 0, i32 1
  %j.load = load i32, ptr %j1, align 4
  store i32 %j.load, ptr %j4, align 4
  ret void
}
