
%Foo = type { float }

define i32 @main() {
  %f = alloca %Foo, align 8
  call void @_EN4main3Foo4initE3int(ptr %f, i32 5)
  call void @_EN4main3Foo6deinitE(ptr %f)
  ret i32 0
}

define void @_EN4main3Foo4initE3int(ptr %this, i32 %i) {
  %i1 = alloca i32, align 4
  store i32 %i, ptr %i1, align 4
  %f = getelementptr inbounds %Foo, ptr %this, i32 0, i32 0
  store float 0.000000e+00, ptr %f, align 4
  ret void
}

define void @_EN4main3Foo6deinitE(ptr %this) {
  ret void
}
