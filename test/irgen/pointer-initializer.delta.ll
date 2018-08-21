
define void @_EN4main3fooE1pPMP4char1xP4char(i8** %p, i8* %x) {
  %pp = alloca i8**
  store i8** %p, i8*** %pp
  %pp1 = load i8**, i8*** %pp
  store i8* %x, i8** %pp1
  %pp2 = load i8**, i8*** %pp
  %1 = load i8*, i8** %pp2
  store i8 120, i8* %1
  ret void
}
