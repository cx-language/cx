
%S = type { i32 }
%T = type { i32 }

define void @_EN4main1fE1S(%S %a) {
  %a1 = alloca %S, align 8
  store %S %a, ptr %a1, align 4
  ret void
}

define void @_EN4main1fE1T(%T %a) {
  %a1 = alloca %T, align 8
  store %T %a, ptr %a1, align 4
  ret void
}

define void @_EN4mainplE1S1S(%S %a, %S %b) {
  %a1 = alloca %S, align 8
  %b2 = alloca %S, align 8
  store %S %a, ptr %a1, align 4
  store %S %b, ptr %b2, align 4
  ret void
}

define void @_EN4mainplE1T1T(%T %a, %T %b) {
  %a1 = alloca %T, align 8
  %b2 = alloca %T, align 8
  store %T %a, ptr %a1, align 4
  store %T %b, ptr %b2, align 4
  ret void
}

define i32 @main() {
  %s = alloca %S, align 8
  %t = alloca %T, align 8
  call void @_EN4main1S4initE(ptr %s)
  call void @_EN4main1T4initE(ptr %t)
  %s.load = load %S, ptr %s, align 4
  call void @_EN4main1fE1S(%S %s.load)
  %t.load = load %T, ptr %t, align 4
  call void @_EN4main1fE1T(%T %t.load)
  %s.load1 = load %S, ptr %s, align 4
  %s.load2 = load %S, ptr %s, align 4
  call void @_EN4mainplE1S1S(%S %s.load1, %S %s.load2)
  %t.load3 = load %T, ptr %t, align 4
  %t.load4 = load %T, ptr %t, align 4
  call void @_EN4mainplE1T1T(%T %t.load3, %T %t.load4)
  ret i32 0
}

define void @_EN4main1S4initE(ptr %this) {
  %i = getelementptr inbounds %S, ptr %this, i32 0, i32 0
  store i32 0, ptr %i, align 4
  ret void
}

define void @_EN4main1T4initE(ptr %this) {
  %i = getelementptr inbounds %T, ptr %this, i32 0, i32 0
  store i32 0, ptr %i, align 4
  ret void
}
