define i32 @main() {
  %three = alloca [3 x i32]
  %b = alloca i32
  %ref = alloca { i32*, i32 }
  %bb = alloca i32
  store [3 x i32] [i32 0, i32 42, i32 0], [3 x i32]* %three
  %1 = getelementptr [3 x i32], [3 x i32]* %three, i32 0, i32 0
  %2 = insertvalue { i32*, i32 } undef, i32* %1, 0
  %3 = insertvalue { i32*, i32 } %2, i32 3, 1
  call void @foo({ i32*, i32 } %3)
  call void @bar([3 x i32]* %three)
  store i32 3, i32* %b
  %4 = getelementptr [3 x i32], [3 x i32]* %three, i32 0, i32 0
  %5 = insertvalue { i32*, i32 } undef, i32* %4, 0
  %6 = insertvalue { i32*, i32 } %5, i32 3, 1
  store { i32*, i32 } %6, { i32*, i32 }* %ref
  %ref1 = load { i32*, i32 }, { i32*, i32 }* %ref
  %count = extractvalue { i32*, i32 } %ref1, 1
  store i32 %count, i32* %bb
  ret i32 0
}

define void @foo({ i32*, i32 } %ints) {
  %a = alloca i32
  %b = alloca i32
  %c = alloca { i32*, i32 }
  %1 = extractvalue { i32*, i32 } %ints, 0
  %2 = getelementptr i32, i32* %1, i32 1
  %3 = load i32, i32* %2
  store i32 %3, i32* %a
  %count = extractvalue { i32*, i32 } %ints, 1
  store i32 %count, i32* %b
  store { i32*, i32 } %ints, { i32*, i32 }* %c
  ret void
}

define void @bar([3 x i32]* %ints) {
  %b = alloca i32
  %ref = alloca { i32*, i32 }
  store i32 3, i32* %b
  %1 = getelementptr [3 x i32], [3 x i32]* %ints, i32 0, i32 0
  %2 = insertvalue { i32*, i32 } undef, i32* %1, 0
  %3 = insertvalue { i32*, i32 } %2, i32 3, 1
  store { i32*, i32 } %3, { i32*, i32 }* %ref
  ret void
}
