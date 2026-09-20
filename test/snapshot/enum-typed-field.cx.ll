
%S = type { i32, i32 }

define i32 @_EN4main1fE1S(%S %s) {
  %s1 = alloca %S, align 8
  store %S %s, ptr %s1, align 4
  %f = getelementptr inbounds %S, ptr %s1, i32 0, i32 1
  %f.load = load i32, ptr %f, align 4
  ret i32 %f.load
}

define i32 @_EN4main1gEP1S(ptr %s) {
  %s1 = alloca ptr, align 8
  store ptr %s, ptr %s1, align 8
  %s.load = load ptr, ptr %s1, align 8
  %f = getelementptr inbounds %S, ptr %s.load, i32 0, i32 1
  %f.load = load i32, ptr %f, align 4
  ret i32 %f.load
}
