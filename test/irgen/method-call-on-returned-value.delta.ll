
%S = type { i32 }
%C = type { i32 }

define void @_ENM4main1S4initE(%S* %this) {
  %i = getelementptr inbounds %S, %S* %this, i32 0, i32 0
  store i32 0, i32* %i
  ret void
}

define void @_EN4main1S1fE(%S %this) {
  ret void
}

define void @_ENM4main1C4initE(%C* %this) {
  %i = getelementptr inbounds %C, %C* %this, i32 0, i32 0
  store i32 0, i32* %i
  ret void
}

define void @_EN4main1C1fE(%C* %this) {
  ret void
}

define %S @_EN4main1sE() {
  %1 = alloca %S
  call void @_ENM4main1S4initE(%S* %1)
  %2 = load %S, %S* %1
  ret %S %2
}

define %C @_EN4main1cE() {
  %1 = alloca %C
  call void @_ENM4main1C4initE(%C* %1)
  %2 = load %C, %C* %1
  ret %C %2
}

define i32 @main() {
  %1 = alloca %C
  %2 = alloca %S
  %3 = alloca %C
  %4 = call %S @_EN4main1sE()
  call void @_EN4main1S1fE(%S %4)
  %5 = call %C @_EN4main1cE()
  store %C %5, %C* %1
  call void @_EN4main1C1fE(%C* %1)
  call void @_ENM4main1S4initE(%S* %2)
  %6 = load %S, %S* %2
  call void @_EN4main1S1fE(%S %6)
  call void @_ENM4main1C4initE(%C* %3)
  call void @_EN4main1C1fE(%C* %3)
  ret i32 0
}
