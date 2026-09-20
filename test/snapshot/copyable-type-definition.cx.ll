
%Foo = type { i32, i1 }

define i32 @main() {
  %f = alloca %Foo, align 8
  %bar = alloca i32, align 4
  %qux = alloca i1, align 1
  call void @_EN4main3Foo4initE3int4bool(ptr %f, i32 666, i1 true)
  %a = getelementptr inbounds %Foo, ptr %f, i32 0, i32 0
  %a.load = load i32, ptr %a, align 4
  %1 = add i32 %a.load, 1
  store i32 %1, ptr %a, align 4
  %a1 = getelementptr inbounds %Foo, ptr %f, i32 0, i32 0
  %a.load2 = load i32, ptr %a1, align 4
  store i32 %a.load2, ptr %bar, align 4
  %b = getelementptr inbounds %Foo, ptr %f, i32 0, i32 1
  %b.load = load i1, ptr %b, align 1
  store i1 %b.load, ptr %qux, align 1
  ret i32 0
}

define void @_EN4main3Foo4initE3int4bool(ptr %this, i32 %a, i1 %b) {
  %a1 = alloca i32, align 4
  %b2 = alloca i1, align 1
  store i32 %a, ptr %a1, align 4
  store i1 %b, ptr %b2, align 1
  %a3 = getelementptr inbounds %Foo, ptr %this, i32 0, i32 0
  %a.load = load i32, ptr %a1, align 4
  store i32 %a.load, ptr %a3, align 4
  %b4 = getelementptr inbounds %Foo, ptr %this, i32 0, i32 1
  %b.load = load i1, ptr %b2, align 1
  store i1 %b.load, ptr %b4, align 1
  %a5 = getelementptr inbounds %Foo, ptr %this, i32 0, i32 0
  %a.load6 = load i32, ptr %a5, align 4
  %1 = add i32 %a.load6, 1
  store i32 %1, ptr %a5, align 4
  ret void
}
