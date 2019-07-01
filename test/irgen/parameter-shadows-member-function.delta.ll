
%S = type { i32 }

define void @_EN4main1S4initE(%S* %this) {
  ret void
}

define void @_EN4main1S3barE(%S* %this) {
  ret void
}

define void @_EN4main1S3fooE3bar3int(%S* %this, i32 %bar) {
  %a = alloca i32
  store i32 %bar, i32* %a
  call void @_EN4main1S3barE(%S* %this)
  ret void
}

define i32 @main() {
  %s = alloca %S
  call void @_EN4main1S4initE(%S* %s)
  ret i32 0
}
