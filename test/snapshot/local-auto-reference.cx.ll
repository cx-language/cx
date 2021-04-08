
%Foo = type { i32 }

define i32 @main() {
  %f = alloca %Foo, align 8
  %rf = alloca %Foo*, align 8
  %pf = alloca %Foo*, align 8
  store %Foo* %f, %Foo** %rf, align 8
  store %Foo* %f, %Foo** %pf, align 8
  ret i32 0
}
