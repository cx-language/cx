
define i32 @main() {
  %foo = alloca i32*
  %bar = alloca i32
  %foo1 = load i32*, i32** %foo
  %bar2 = load i32, i32* %bar
  %1 = load i32, i32* %foo1
  %2 = icmp slt i32 %1, %bar2
  ret i32 0
}
