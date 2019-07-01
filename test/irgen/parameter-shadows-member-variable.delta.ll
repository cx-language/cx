
%S = type { i32 }

define void @_EN4main1S4initE(%S* %this) {
  ret void
}

define void @_EN4main1S3fooE3bar3int(%S* %this, i32 %bar) {
  %1 = add i32 %bar, 42
  ret void
}

define i32 @main() {
  %s = alloca %S
  call void @_EN4main1S4initE(%S* %s)
  ret i32 0
}
