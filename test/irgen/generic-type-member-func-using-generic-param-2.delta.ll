
define i32 @main() {
  %c = alloca {}
  call void @_ENM4main1CI4boolE4initE({}* %c)
  %c1 = load {}, {}* %c
  call void @_EN4main1CI4boolE1fE({} %c1)
  ret i32 0
}

define void @_ENM4main1CI4boolE4initE({}* %this) {
  ret void
}

define void @_EN4main1CI4boolE1fE({} %this) {
  call void @_EN4main1CI4boolE1gE({} %this)
  ret void
}

define void @_EN4main1CI4boolE1gE({} %this) {
  %a = alloca i64
  store i64 ptrtoint (i1* getelementptr (i1, i1* null, i32 1) to i64), i64* %a
  ret void
}
