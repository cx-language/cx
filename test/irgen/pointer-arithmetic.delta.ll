
define void @_EN4main1fE1pP3int(i32* %p) {
  %a = alloca i32*
  %1 = getelementptr i32, i32* %p, i32 4
  %2 = getelementptr i32, i32* %1, i32 -1
  store i32* %2, i32** %a
  %a.load = load i32*, i32** %a
  %3 = getelementptr i32, i32* %a.load, i32 3
  store i32* %3, i32** %a
  %a.load1 = load i32*, i32** %a
  %4 = getelementptr i32, i32* %a.load1, i32 -3
  store i32* %4, i32** %a
  %a.load2 = load i32*, i32** %a
  %5 = getelementptr i32, i32* %a.load2, i32 1
  store i32* %5, i32** %a
  %a.load3 = load i32*, i32** %a
  %6 = getelementptr i32, i32* %a.load3, i32 -1
  store i32* %6, i32** %a
  ret void
}
