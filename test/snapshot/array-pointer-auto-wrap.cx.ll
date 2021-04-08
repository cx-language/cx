
define i32 @main() {
  %p = alloca i32*, align 8
  %o = alloca i32*, align 8
  %p.load = load i32*, i32** %p, align 8
  store i32* %p.load, i32** %o, align 8
  ret i32 0
}
