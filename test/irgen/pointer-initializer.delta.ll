
define void @_EN4main3fooE1pPP4char1xP4char(i8** %p, i8* %x) {
  %pp = alloca i8**
  store i8** %p, i8*** %pp
  %pp.load = load i8**, i8*** %pp
  store i8* %x, i8** %pp.load
  %pp.load1 = load i8**, i8*** %pp
  %pp.load1.load = load i8*, i8** %pp.load1
  store i8 120, i8* %pp.load1.load
  ret void
}
