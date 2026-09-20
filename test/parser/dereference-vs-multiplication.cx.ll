
define i32 @_EN4main1gE() {
  ret i32 1
}

define void @_EN4main1fEP3int3int(ptr %a, i32 %b) {
  %a1 = alloca ptr, align 8
  %b2 = alloca i32, align 4
  %m = alloca i32, align 4
  %n = alloca i32, align 4
  store ptr %a, ptr %a1, align 8
  store i32 %b, ptr %b2, align 4
  %1 = call i32 @_EN4main1gE()
  store i32 %1, ptr %m, align 4
  %a.load = load ptr, ptr %a1, align 8
  %b.load = load i32, ptr %b2, align 4
  store i32 %b.load, ptr %a.load, align 4
  %2 = call i32 @_EN4main1gE()
  %b.load3 = load i32, ptr %b2, align 4
  %3 = mul i32 %2, %b.load3
  store i32 %3, ptr %n, align 4
  ret void
}
