
%S = type { i32* }

define void @_EN4main1S3fooE(%S %this) {
  %b = alloca i32*
  %a = extractvalue %S %this, 0
  store i32* %a, i32** %b
  ret void
}
