
%"C<bool>" = type {}

define i32 @main() {
  %c = alloca %"C<bool>", align 8
  call void @_EN4main1CI4boolE4initE(ptr %c)
  call void @_EN4main1CI4boolE1fE(ptr %c)
  ret i32 0
}

define void @_EN4main1CI4boolE4initE(ptr %this) {
  ret void
}

define void @_EN4main1CI4boolE1fE(ptr %this) {
  call void @_EN4main1CI4boolE1gE(ptr %this)
  ret void
}

define void @_EN4main1CI4boolE1gE(ptr %this) {
  %a = alloca i64, align 8
  store i64 1, ptr %a, align 4
  ret void
}
