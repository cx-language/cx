
%B = type {}

define i32 @main() {
  %b = alloca %B, align 8
  %x = alloca i32, align 4
  call void @_EN4main1B4initE(%B* %b)
  %1 = call i32 @_EN4main1B1gE(%B* %b)
  store i32 %1, i32* %x, align 4
  ret i32 0
}

define void @_EN4main1B4initE(%B* %this) {
  ret void
}

define i32 @_EN4main1B1gE(%B* %this) {
  %1 = call i32 @_EN4main1B1fE(%B* %this)
  %2 = mul i32 %1, 2
  ret i32 %2
}

define i32 @_EN4main1B1fE(%B* %this) {
  ret i32 21
}
