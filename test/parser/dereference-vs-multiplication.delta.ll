
define i32 @_EN4main1gE() {
  ret i32 1
}

define void @_EN4main1fE1aP3int1b3int(i32* %a, i32 %b) {
  %m = alloca i32
  %n = alloca i32
  %1 = call i32 @_EN4main1gE()
  store i32 %1, i32* %m
  store i32 %b, i32* %a
  %2 = call i32 @_EN4main1gE()
  %3 = mul i32 %2, %b
  store i32 %3, i32* %n
  ret void
}
