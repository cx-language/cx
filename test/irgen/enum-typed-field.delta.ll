
%S = type { i32, i32 }

define void @_EN4main1S4initE1e1E1f1E(%S* %this, i32 %e, i32 %f) {
  %e1 = getelementptr inbounds %S, %S* %this, i32 0, i32 0
  store i32 %e, i32* %e1
  %f2 = getelementptr inbounds %S, %S* %this, i32 0, i32 1
  store i32 %f, i32* %f2
  ret void
}

define i32 @_EN4main1fE1s1S(%S %s) {
  %f = extractvalue %S %s, 1
  ret i32 %f
}

define i32 @_EN4main1gE1sP1S(%S* %s) {
  %f = getelementptr inbounds %S, %S* %s, i32 0, i32 1
  %f.load = load i32, i32* %f
  ret i32 %f.load
}
