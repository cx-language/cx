
define void @_EN4main3fooEPP4charP4char(i8** %p, i8* %x) {
  %pp = alloca i8**, align 8
  store i8** %p, i8*** %pp, align 8
  %pp.load = load i8**, i8*** %pp, align 8
  store i8* %x, i8** %pp.load, align 8
  %pp.load1 = load i8**, i8*** %pp, align 8
  %pp.load.load = load i8*, i8** %pp.load1, align 8
  store i8 120, i8* %pp.load.load, align 1
  ret void
}
