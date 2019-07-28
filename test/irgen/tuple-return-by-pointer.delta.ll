
%S = type { { i32 } }

define { i32 }* @_EN4main1S1fE(%S* %this) {
  %t = getelementptr inbounds %S, %S* %this, i32 0, i32 0
  ret { i32 }* %t
}

define { i32 }* @_EN4main1S1gE(%S* %this) {
  %t = getelementptr inbounds %S, %S* %this, i32 0, i32 0
  ret { i32 }* %t
}

define void @_EN4main1S4initE1tT3int_(%S* %this, { i32 } %t) {
  %t1 = getelementptr inbounds %S, %S* %this, i32 0, i32 0
  store { i32 } %t, { i32 }* %t1
  ret void
}
