
%Foo = type { i32 }

define void @_EN4main3ptrER3Foo(ptr %p) {
  %p1 = alloca ptr, align 8
  store ptr %p, ptr %p1, align 8
  ret void
}

define i32 @main() {
  %f = alloca %Foo, align 8
  call void @_EN4main3ptrER3Foo(ptr %f)
  ret i32 0
}
