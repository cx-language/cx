
%Foo = type { i32 }

define void @_EN4main3fooEP3Foo(ptr %ref_f) {
  %ref_f1 = alloca ptr, align 8
  store ptr %ref_f, ptr %ref_f1, align 8
  ret void
}

define i32 @main() {
  %f = alloca %Foo, align 8
  call void @_EN4main3fooEP3Foo(ptr %f)
  ret i32 0
}
