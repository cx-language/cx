
%S = type { i32 }

define i32 @main() {
  %1 = alloca %S, align 8
  call void @_EN4main1S4initE3int(ptr %1, i32 1)
  call void @_EN4main1S3fooE(ptr %1)
  ret i32 0
}

define void @_EN4main1S4initE3int(ptr %this, i32 %a) {
  %a1 = alloca i32, align 4
  store i32 %a, ptr %a1, align 4
  %a2 = getelementptr inbounds %S, ptr %this, i32 0, i32 0
  %a.load = load i32, ptr %a1, align 4
  store i32 %a.load, ptr %a2, align 4
  ret void
}

define void @_EN4main1S3fooE(ptr %this) {
  %b = alloca i32, align 4
  %a = getelementptr inbounds %S, ptr %this, i32 0, i32 0
  %a.load = load i32, ptr %a, align 4
  store i32 %a.load, ptr %b, align 4
  ret void
}
