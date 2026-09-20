
%S = type { ptr }

define i32 @main() {
  %a = alloca %S, align 8
  %b = alloca %S, align 8
  call void @_EN4main1S4initE(ptr %a)
  call void @_EN4main1S4initE(ptr %b)
  %1 = call ptr @_EN4main1S3fooEP1S(ptr %a, ptr %b)
  call void @_EN4main1fI1SEEP1S(ptr %1)
  ret i32 0
}

define void @_EN4main1S4initE(ptr %this) {
  %that = getelementptr inbounds %S, ptr %this, i32 0, i32 0
  store ptr %this, ptr %that, align 8
  ret void
}

define ptr @_EN4main1S3fooEP1S(ptr %this, ptr %a) {
  %a1 = alloca ptr, align 8
  store ptr %a, ptr %a1, align 8
  ret ptr %this
}

define void @_EN4main1fI1SEEP1S(ptr %a) {
  %a1 = alloca ptr, align 8
  store ptr %a, ptr %a1, align 8
  %a.load = load ptr, ptr %a1, align 8
  %a.load2 = load ptr, ptr %a1, align 8
  %1 = call ptr @_EN4main1S3barE(ptr %a.load2)
  %2 = call ptr @_EN4main1S3fooEP1S(ptr %a.load, ptr %1)
  ret void
}

define ptr @_EN4main1S3barE(ptr %this) {
  %1 = call ptr @_EN4main1S3fooEP1S(ptr %this, ptr %this)
  %2 = call ptr @_EN4main1S3fooEP1S(ptr %this, ptr %1)
  ret ptr %2
}
