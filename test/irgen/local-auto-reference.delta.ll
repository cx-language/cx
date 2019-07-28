
%Foo = type { i32 }

define void @_EN4main3Foo4initE1i3int(%Foo* %this, i32 %i) {
  %i1 = getelementptr inbounds %Foo, %Foo* %this, i32 0, i32 0
  store i32 %i, i32* %i1
  ret void
}

define i32 @main() {
  %f = alloca %Foo
  %rf = alloca %Foo*
  %pf = alloca %Foo*
  store %Foo* %f, %Foo** %rf
  store %Foo* %f, %Foo** %pf
  ret i32 0
}
