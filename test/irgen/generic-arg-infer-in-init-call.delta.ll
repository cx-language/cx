
%"Foo<int>" = type { i32 }
%"Foo<string>" = type { %string }
%string = type { %"ArrayRef<char>" }
%"ArrayRef<char>" = type { i8*, i32 }

@0 = private unnamed_addr constant [1 x i8] zeroinitializer, align 1

define i32 @main() {
  %i = alloca %"Foo<int>"
  %b = alloca %"Foo<string>"
  %__str6 = alloca %string
  call void @_EN4main3FooI3intE4initE1t3int(%"Foo<int>"* %i, i32 42)
  call void @_EN3std6string4initE7pointerP4char6length3int(%string* %__str6, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @0, i32 0, i32 0), i32 0)
  %__str6.load = load %string, %string* %__str6
  call void @_EN4main3FooI6stringE4initE1t6string(%"Foo<string>"* %b, %string %__str6.load)
  ret i32 0
}

define void @_EN4main3FooI3intE4initE1t3int(%"Foo<int>"* %this, i32 %t) {
  %t1 = getelementptr inbounds %"Foo<int>", %"Foo<int>"* %this, i32 0, i32 0
  store i32 %t, i32* %t1
  ret void
}

define void @_EN4main3FooI6stringE4initE1t6string(%"Foo<string>"* %this, %string %t) {
  %t1 = getelementptr inbounds %"Foo<string>", %"Foo<string>"* %this, i32 0, i32 0
  store %string %t, %string* %t1
  ret void
}

declare void @_EN3std6string4initE7pointerP4char6length3int(%string*, i8*, i32)
