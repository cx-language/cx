
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
  %i1 = getelementptr inbounds %Foo, ptr %this, i32 0, i32 0
  store i32 %i, ptr %i1, align 4
  ret void
}

define void @_EN4main3Bar4initE3int3int(ptr %this, i32 %j, i32 %i) {
  %i1 = getelementptr inbounds %Bar, ptr %this, i32 0, i32 0
  store i32 %i, ptr %i1, align 4
  %j2 = getelementptr inbounds %Bar, ptr %this, i32 0, i32 1
  store i32 %j, ptr %j2, align 4
  ret void
}
