
%Foo = type { i32 }
%Bar = type { i32 }

define i32 @main() {
  %1 = alloca %Foo, align 8
  %2 = alloca %Bar, align 8
  call void @_EN4main3Foo4initE3int(ptr %1, i32 1)
  call void @_EN4main3Foo7doStuffE(ptr %1)
  call void @_EN4main3Bar4initE3int(ptr %2, i32 2)
  call void @_EN4main3Bar7doStuffE(ptr %2)
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

define void @_EN4main3Foo7doStuffE(ptr %this) {
  ret void
}

define void @_EN4main3Bar4initE3int(ptr %this, i32 %i) {
  %i1 = alloca i32, align 4
  store i32 %i, ptr %i1, align 4
  %i2 = getelementptr inbounds %Bar, ptr %this, i32 0, i32 0
  %i.load = load i32, ptr %i1, align 4
  store i32 %i.load, ptr %i2, align 4
  ret void
}

define void @_EN4main3Bar7doStuffE(ptr %this) {
  ret void
}
