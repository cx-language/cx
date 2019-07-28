
%S = type { %S* }

define void @_EN4main1S4initE4nextP1S(%S* %this, %S* %next) {
  %next1 = getelementptr inbounds %S, %S* %this, i32 0, i32 0
  store %S* %next, %S** %next1
  ret void
}

define void @_EN4main1fE1s1S(%S %s) {
  ret void
}
