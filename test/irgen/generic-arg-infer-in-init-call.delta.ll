
%"Foo<int>" = type { i32 }
%"Foo<StringRef>" = type { %StringRef }
%StringRef = type { %"ArrayRef<char>" }
%"ArrayRef<char>" = type { i8*, i32 }

@0 = private unnamed_addr constant [1 x i8] zeroinitializer

define i32 @main() {
  %i = alloca %"Foo<int>"
  %b = alloca %"Foo<StringRef>"
  %__str0 = alloca %StringRef
  call void @_ENM4main3FooI3intE4initE1t3int(%"Foo<int>"* %i, i32 42)
  call void @_ENM3std9StringRef4initE7pointerP4char6length3int(%StringRef* %__str0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @0, i32 0, i32 0), i32 0)
  %__str0.load = load %StringRef, %StringRef* %__str0
  call void @_ENM4main3FooI9StringRefE4initE1t9StringRef(%"Foo<StringRef>"* %b, %StringRef %__str0.load)
  ret i32 0
}

define void @_ENM4main3FooI3intE4initE1t3int(%"Foo<int>"* %this, i32 %t) {
  %t1 = getelementptr inbounds %"Foo<int>", %"Foo<int>"* %this, i32 0, i32 0
  store i32 %t, i32* %t1
  ret void
}

define void @_ENM4main3FooI9StringRefE4initE1t9StringRef(%"Foo<StringRef>"* %this, %StringRef %t) {
  %t1 = getelementptr inbounds %"Foo<StringRef>", %"Foo<StringRef>"* %this, i32 0, i32 0
  store %StringRef %t, %StringRef* %t1
  ret void
}

declare void @_ENM3std9StringRef4initE7pointerP4char6length3int(%StringRef*, i8*, i32)
