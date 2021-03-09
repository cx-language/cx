
%S = type { i32 }

define i32 @main() {
  %s = alloca %S, align 8
  call void @_EN4main1S4initE(%S* %s)
  call void @_EN4main1S3fooE3int(%S* %s, i32 30)
  ret i32 0
}

define void @_EN4main1S4initE(%S* %this) {
  ret void
}

define void @_EN4main1S3fooE3int(%S* %this, i32 %bar) {
  %a = alloca i32, align 4
  store i32 %bar, i32* %a, align 4
  call void @_EN4main1S3barE(%S* %this)
  ret void
}

define void @_EN4main1S3barE(%S* %this) {
  ret void
}
