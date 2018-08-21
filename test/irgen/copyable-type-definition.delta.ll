
%Foo = type { i32, i1 }

define void @_ENM4main3Foo4initE1a3int1b4bool(%Foo* %this, i32 %a, i1 %b) {
  %a1 = getelementptr inbounds %Foo, %Foo* %this, i32 0, i32 0
  store i32 %a, i32* %a1
  %b2 = getelementptr inbounds %Foo, %Foo* %this, i32 0, i32 1
  store i1 %b, i1* %b2
  %a3 = getelementptr inbounds %Foo, %Foo* %this, i32 0, i32 0
  %1 = load i32, i32* %a3
  %2 = add i32 %1, 1
  store i32 %2, i32* %a3
  ret void
}

define i32 @main() {
  %f = alloca %Foo
  %bar = alloca i32
  %qux = alloca i1
  call void @_ENM4main3Foo4initE1a3int1b4bool(%Foo* %f, i32 666, i1 true)
  %a = getelementptr inbounds %Foo, %Foo* %f, i32 0, i32 0
  %1 = load i32, i32* %a
  %2 = add i32 %1, 1
  store i32 %2, i32* %a
  %a1 = getelementptr inbounds %Foo, %Foo* %f, i32 0, i32 0
  %3 = load i32, i32* %a1
  store i32 %3, i32* %bar
  %b = getelementptr inbounds %Foo, %Foo* %f, i32 0, i32 1
  %4 = load i1, i1* %b
  store i1 %4, i1* %qux
  ret i32 0
}
