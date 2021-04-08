
%S = type { i32 }
%C = type { i32 }

define %S @_EN4main1sE() {
  %1 = alloca %S, align 8
  call void @_EN4main1S4initE(%S* %1)
  %.load = load %S, %S* %1, align 4
  ret %S %.load
}

define void @_EN4main1S4initE(%S* %this) {
  %i = getelementptr inbounds %S, %S* %this, i32 0, i32 0
  store i32 0, i32* %i, align 4
  ret void
}

define %C @_EN4main1cE() {
  %1 = alloca %C, align 8
  call void @_EN4main1C4initE(%C* %1)
  %.load = load %C, %C* %1, align 4
  ret %C %.load
}

define void @_EN4main1C4initE(%C* %this) {
  %i = getelementptr inbounds %C, %C* %this, i32 0, i32 0
  store i32 0, i32* %i, align 4
  ret void
}

define i32 @main() {
  %1 = alloca %S, align 8
  %2 = alloca %C, align 8
  %3 = alloca %S, align 8
  %4 = alloca %C, align 8
  %5 = call %S @_EN4main1sE()
  store %S %5, %S* %1, align 4
  call void @_EN4main1S1fE(%S* %1)
  %6 = call %C @_EN4main1cE()
  store %C %6, %C* %2, align 4
  call void @_EN4main1C1fE(%C* %2)
  call void @_EN4main1S4initE(%S* %3)
  call void @_EN4main1S1fE(%S* %3)
  call void @_EN4main1C4initE(%C* %4)
  call void @_EN4main1C1fE(%C* %4)
  ret i32 0
}

define void @_EN4main1S1fE(%S* %this) {
  ret void
}

define void @_EN4main1C1fE(%C* %this) {
  ret void
}
