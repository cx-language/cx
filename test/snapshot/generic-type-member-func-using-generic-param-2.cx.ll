
%"C<bool>" = type {}

define i32 @main() {
  %c = alloca %"C<bool>", align 8
  call void @_EN4main1CI4boolE4initE(%"C<bool>"* %c)
  call void @_EN4main1CI4boolE1fE(%"C<bool>"* %c)
  ret i32 0
}

define void @_EN4main1CI4boolE4initE(%"C<bool>"* %this) {
  ret void
}

define void @_EN4main1CI4boolE1fE(%"C<bool>"* %this) {
  call void @_EN4main1CI4boolE1gE(%"C<bool>"* %this)
  ret void
}

define void @_EN4main1CI4boolE1gE(%"C<bool>"* %this) {
  %a = alloca i64, align 8
  store i64 ptrtoint (i1* getelementptr (i1, i1* null, i32 1) to i64), i64* %a, align 4
  ret void
}
