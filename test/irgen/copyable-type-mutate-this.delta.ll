
%S = type { i32 }

define void @_ENM4main1S1sE(%S* %this) {
  %i = getelementptr inbounds %S, %S* %this, i32 0, i32 0
  store i32 2, i32* %i
  ret void
}
