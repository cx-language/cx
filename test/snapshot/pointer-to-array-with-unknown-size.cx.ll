
define i32 @main() {
  %a = alloca [3 x i32], align 4
  %p = alloca i32*, align 8
  %t = alloca i32, align 4
  store [3 x i32] [i32 1, i32 2, i32 3], [3 x i32]* %a, align 4
  %1 = getelementptr inbounds [3 x i32], [3 x i32]* %a, i32 0, i32 0
  store i32* %1, i32** %p, align 8
  %2 = bitcast [3 x i32]* %a to i32*
  store i32* %2, i32** %p, align 8
  %3 = bitcast [3 x i32]* %a to i32*
  store i32* %3, i32** %p, align 8
  %p.load = load i32*, i32** %p, align 8
  %4 = getelementptr inbounds i32, i32* %p.load, i32 1
  %.load = load i32, i32* %4, align 4
  store i32 %.load, i32* %t, align 4
  ret i32 0
}
