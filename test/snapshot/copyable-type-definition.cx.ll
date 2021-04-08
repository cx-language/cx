
%Foo = type { i32, i1 }

define i32 @main() {
  %f = alloca %Foo, align 8
  %bar = alloca i32, align 4
  %qux = alloca i1, align 1
  call void @_EN4main3Foo4initE3int4bool(%Foo* %f, i32 666, i1 true)
  %a = getelementptr inbounds %Foo, %Foo* %f, i32 0, i32 0
  %a.load = load i32, i32* %a, align 4
  %1 = add i32 %a.load, 1
  store i32 %1, i32* %a, align 4
  %a1 = getelementptr inbounds %Foo, %Foo* %f, i32 0, i32 0
  %a.load2 = load i32, i32* %a1, align 4
  store i32 %a.load2, i32* %bar, align 4
  %b = getelementptr inbounds %Foo, %Foo* %f, i32 0, i32 1
  %b.load = load i1, i1* %b, align 1
  store i1 %b.load, i1* %qux, align 1
  ret i32 0
}

define void @_EN4main3Foo4initE3int4bool(%Foo* %this, i32 %a, i1 %b) {
  %a1 = getelementptr inbounds %Foo, %Foo* %this, i32 0, i32 0
  store i32 %a, i32* %a1, align 4
  %b2 = getelementptr inbounds %Foo, %Foo* %this, i32 0, i32 1
  store i1 %b, i1* %b2, align 1
  %a3 = getelementptr inbounds %Foo, %Foo* %this, i32 0, i32 0
  %a.load = load i32, i32* %a3, align 4
  %1 = add i32 %a.load, 1
  store i32 %1, i32* %a3, align 4
  ret void
}
