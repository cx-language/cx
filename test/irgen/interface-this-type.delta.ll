
%S = type { %S* }

define void @_EN4main1S4initE(%S* %this) {
  %that = getelementptr inbounds %S, %S* %this, i32 0, i32 0
  store %S* %this, %S** %that
  ret void
}

define %S* @_EN4main1S3fooE1aP1S(%S* %this, %S* %a) {
  ret %S* %this
}

define %S* @_EN4main1S3barE(%S* %this) {
  %1 = call %S* @_EN4main1S3fooE1aP1S(%S* %this, %S* %this)
  %2 = call %S* @_EN4main1S3fooE1aP1S(%S* %this, %S* %1)
  ret %S* %2
}

define i32 @main() {
  %a = alloca %S
  %b = alloca %S
  call void @_EN4main1S4initE(%S* %a)
  call void @_EN4main1S4initE(%S* %b)
  %1 = call %S* @_EN4main1S3fooE1aP1S(%S* %a, %S* %b)
  call void @_EN4main1fI1SEE1aP1S(%S* %1)
  ret i32 0
}

define void @_EN4main1fI1SEE1aP1S(%S* %a) {
  %1 = call %S* @_EN4main1S3fooE1aP1S(%S* %a, %S* %a)
  %2 = call %S* @_EN4main1S3fooE1aP1S(%S* %a, %S* %1)
  ret void
}
