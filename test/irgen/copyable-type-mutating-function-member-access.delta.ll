
%S = type { i32 }

define void @_EN4main1S3fooE(%S* %this) {
  %b = alloca i32
  %a = getelementptr inbounds %S, %S* %this, i32 0, i32 0
  %a.load = load i32, i32* %a
  store i32 %a.load, i32* %b
  ret void
}

define void @_EN4main1S4initE1a3int(%S* %this, i32 %a) {
  %a1 = getelementptr inbounds %S, %S* %this, i32 0, i32 0
  store i32 %a, i32* %a1
  ret void
}
