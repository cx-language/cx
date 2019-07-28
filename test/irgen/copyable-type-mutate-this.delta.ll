
%S = type { i32 }

define void @_EN4main1S1sE(%S* %this) {
  %i = getelementptr inbounds %S, %S* %this, i32 0, i32 0
  store i32 2, i32* %i
  ret void
}

define void @_EN4main1S4initE1i3int(%S* %this, i32 %i) {
  %i1 = getelementptr inbounds %S, %S* %this, i32 0, i32 0
  store i32 %i, i32* %i1
  ret void
}
