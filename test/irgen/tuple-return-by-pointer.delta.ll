
%S = type { { i32 } }

define { i32 }* @_EN4main1S1fE(%S* %this) {
  %t = getelementptr inbounds %S, %S* %this, i32 0, i32 0
  ret { i32 }* %t
}

define { i32 }* @_EN4main1S1gE(%S* %this) {
  %t = getelementptr inbounds %S, %S* %this, i32 0, i32 0
  ret { i32 }* %t
}
