
%S = type { ptr }

define i32 @main() {
  %1 = alloca %S, align 8
  %2 = alloca i32, align 4
  store i32 1, ptr %2, align 4
  call void @_EN4main1S4initEP3int(ptr %1, ptr %2)
  call void @_EN4main1S3fooE(ptr %1)
  ret i32 0
}

define void @_EN4main1S4initEP3int(ptr %this, ptr %a) {
  %a1 = alloca ptr, align 8
  store ptr %a, ptr %a1, align 8
  %a2 = getelementptr inbounds %S, ptr %this, i32 0, i32 0
  %a.load = load ptr, ptr %a1, align 8
  store ptr %a.load, ptr %a2, align 8
  ret void
}

define void @_EN4main1S3fooE(ptr %this) {
  %b = alloca ptr, align 8
  %a = getelementptr inbounds %S, ptr %this, i32 0, i32 0
  %a.load = load ptr, ptr %a, align 8
  store ptr %a.load, ptr %b, align 8
  ret void
}
