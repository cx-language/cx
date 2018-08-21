
%S = type { i32 }
%T = type { i32 }

define void @_ENM4main1S4initE(%S* %this) {
  %i = getelementptr inbounds %S, %S* %this, i32 0, i32 0
  store i32 0, i32* %i
  ret void
}

define void @_ENM4main1T4initE(%T* %this) {
  %i = getelementptr inbounds %T, %T* %this, i32 0, i32 0
  store i32 0, i32* %i
  ret void
}

define void @_EN4main1fE1a1S(%S %a) {
  ret void
}

define void @_EN4main1fE1a1T(%T %a) {
  ret void
}

define void @_EN4mainplE1a1S1b1S(%S %a, %S %b) {
  ret void
}

define void @_EN4mainplE1a1T1b1T(%T %a, %T %b) {
  ret void
}

define i32 @main() {
  %s = alloca %S
  %t = alloca %T
  call void @_ENM4main1S4initE(%S* %s)
  call void @_ENM4main1T4initE(%T* %t)
  %s1 = load %S, %S* %s
  call void @_EN4main1fE1a1S(%S %s1)
  %t2 = load %T, %T* %t
  call void @_EN4main1fE1a1T(%T %t2)
  %s3 = load %S, %S* %s
  %s4 = load %S, %S* %s
  call void @_EN4mainplE1a1S1b1S(%S %s3, %S %s4)
  %t5 = load %T, %T* %t
  %t6 = load %T, %T* %t
  call void @_EN4mainplE1a1T1b1T(%T %t5, %T %t6)
  ret i32 0
}
