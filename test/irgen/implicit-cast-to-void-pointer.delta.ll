
define i32 @main() {
  %p = alloca i32*
  %mp = alloca i32*
  %v = alloca i8*
  %b = alloca i8*
  %i = alloca i32
  %p1 = load i32*, i32** %p
  %1 = bitcast i32* %p1 to i8*
  store i8* %1, i8** %v
  %p2 = load i32*, i32** %p
  %2 = bitcast i32* %p2 to i8*
  call void @_EN4main3barE1pP4void(i8* %2)
  %v3 = load i8*, i8** %v
  call void @_EN4main3barE1pP4void(i8* %v3)
  %v4 = load i8*, i8** %v
  store i8* %v4, i8** %b
  %b5 = load i8*, i8** %b
  store i8* %b5, i8** %v
  %mp6 = load i32*, i32** %mp
  %3 = bitcast i32* %mp6 to i8*
  call void @_EN4main3bazE1pOP4void(i8* %3)
  store i32 0, i32* %i
  %4 = bitcast i32* %i to i8*
  call void @_EN4main3bazE1pOP4void(i8* %4)
  ret i32 0
}

define void @_EN4main3barE1pP4void(i8* %p) {
  ret void
}

define void @_EN4main3bazE1pOP4void(i8* %p) {
  ret void
}
