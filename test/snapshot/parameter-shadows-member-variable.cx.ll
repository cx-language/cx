
%S = type { i32 }

define i32 @main() {
  %1 = alloca %S, align 8
  call void @_EN4main1S4initE(ptr %1)
  call void @_EN4main1S3fooE3int(ptr %1, i32 30)
  ret i32 0
}

define void @_EN4main1S4initE(ptr %this) {
  ret void
}

define void @_EN4main1S3fooE3int(ptr %this, i32 %bar) {
  %bar1 = alloca i32, align 4
  store i32 %bar, ptr %bar1, align 4
  %bar.load = load i32, ptr %bar1, align 4
  %1 = add i32 %bar.load, 42
  ret void
}
