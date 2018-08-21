
%Foo = type { i32 }

define i32 @main() {
  %x = alloca %Foo
  %x1 = load %Foo, %Foo* %x
  call void @foo(%Foo %x1)
  ret i32 0
}

declare void @foo(%Foo)
