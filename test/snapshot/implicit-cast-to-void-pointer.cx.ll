
define i32 @main() {
  %p = alloca i32*, align 8
  %cp = alloca i32*, align 8
  %v = alloca i8*, align 8
  %b = alloca i8*, align 8
  %i = alloca i32, align 4
  %p.load = load i32*, i32** %p, align 8
  %1 = bitcast i32* %p.load to i8*
  store i8* %1, i8** %v, align 8
  %p.load1 = load i32*, i32** %p, align 8
  %2 = bitcast i32* %p.load1 to i8*
  call void @_EN4main3barEP4void(i8* %2)
  %v.load = load i8*, i8** %v, align 8
  call void @_EN4main3barEP4void(i8* %v.load)
  %v.load2 = load i8*, i8** %v, align 8
  store i8* %v.load2, i8** %b, align 8
  %b.load = load i8*, i8** %b, align 8
  store i8* %b.load, i8** %v, align 8
  %cp.load = load i32*, i32** %cp, align 8
  %3 = bitcast i32* %cp.load to i8*
  call void @_EN4main3bazEOP4void(i8* %3)
  store i32 0, i32* %i, align 4
  %4 = bitcast i32* %i to i8*
  call void @_EN4main3bazEOP4void(i8* %4)
  ret i32 0
}

define void @_EN4main3barEP4void(i8* %p) {
  ret void
}

define void @_EN4main3bazEOP4void(i8* %p) {
  ret void
}
