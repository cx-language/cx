
define void @_EN4main1fEP3int(i32* %a) {
  %p = alloca i32*, align 8
  store i32* %a, i32** %p, align 8
  %p.load = load i32*, i32** %p, align 8
  %p.load.load = load i32, i32* %p.load, align 4
  %1 = add i32 %p.load.load, 1
  store i32 %1, i32* %p.load, align 4
  %a.load = load i32, i32* %a, align 4
  %2 = add i32 %a.load, -1
  store i32 %2, i32* %a, align 4
  ret void
}
