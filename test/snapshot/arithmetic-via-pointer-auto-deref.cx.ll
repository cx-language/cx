
define i32 @main() {
  %a = alloca i32, align 4
  %b = alloca i32, align 4
  %p = alloca i32*, align 8
  %q = alloca i32*, align 8
  %c = alloca i32, align 4
  store i32 2, i32* %a, align 4
  store i32 3, i32* %b, align 4
  store i32* %a, i32** %p, align 8
  store i32* %b, i32** %q, align 8
  %p.load = load i32*, i32** %p, align 8
  %p.load.load = load i32, i32* %p.load, align 4
  %q.load = load i32*, i32** %q, align 8
  %q.load.load = load i32, i32* %q.load, align 4
  %1 = mul i32 %p.load.load, %q.load.load
  store i32 %1, i32* %c, align 4
  ret i32 0
}
