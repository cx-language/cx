
define void @_EN4main3fooEOP3int(i32* %p) {
  ret void
}

define i32 @main() {
  %i = alloca i32, align 4
  %x = alloca i32*, align 8
  store i32 42, i32* %i, align 4
  store i32* %i, i32** %x, align 8
  %x.load = load i32*, i32** %x, align 8
  call void @_EN4main3fooEOP3int(i32* %x.load)
  call void @_EN4main3fooEOP3int(i32* %i)
  ret i32 0
}
