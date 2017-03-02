%Foo = type { i32, i1 }

define %Foo @__init_Foo(i32 %a, i1 %b) {
  %1 = alloca %Foo
  %2 = getelementptr inbounds %Foo, %Foo* %1, i32 0, i32 0
  store i32 %a, i32* %2
  %3 = getelementptr inbounds %Foo, %Foo* %1, i32 0, i32 1
  store i1 %b, i1* %3
  %4 = getelementptr inbounds %Foo, %Foo* %1, i32 0, i32 0
  %5 = load i32, i32* %4
  %6 = add i32 %5, 1
  store i32 %6, i32* %4
  %7 = load %Foo, %Foo* %1
  ret %Foo %7
}

define i32 @main() {
  %f = alloca %Foo
  %bar = alloca i32
  %qux = alloca i1
  %1 = call %Foo @__init_Foo(i32 666, i1 true)
  store %Foo %1, %Foo* %f
  %2 = getelementptr inbounds %Foo, %Foo* %f, i32 0, i32 0
  %3 = load i32, i32* %2
  %4 = add i32 %3, 1
  store i32 %4, i32* %2
  %5 = getelementptr inbounds %Foo, %Foo* %f, i32 0, i32 0
  %6 = load i32, i32* %5
  store i32 %6, i32* %bar
  %7 = getelementptr inbounds %Foo, %Foo* %f, i32 0, i32 1
  %8 = load i1, i1* %7
  store i1 %8, i1* %qux
  ret i32 0
}
