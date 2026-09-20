
define i32 @_EN4main1fE3int(i32 %a) {
  %a1 = alloca i32, align 4
  store i32 %a, ptr %a1, align 4
  %a.load = load i32, ptr %a1, align 4
  ret i32 %a.load
}

define i32 @main() {
  call void @foo(ptr @_EN4main1fE3int)
  call void @bar(ptr @_EN4main1fE3int)
  call void @baz(ptr @_EN4main1fE3int)
  ret i32 0
}

declare void @foo(ptr)

declare void @bar(ptr)

declare void @baz(ptr)
