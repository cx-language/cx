
%F = type { i32, [1 x i32] }

define void @_EN4main1F4initE(%F* %this) {
  %v = getelementptr inbounds %F, %F* %this, i32 0, i32 0
  store i32 42, i32* %v
  %v1 = getelementptr inbounds %F, %F* %this, i32 0, i32 0
  store i32 43, i32* %v1
  %a = getelementptr inbounds %F, %F* %this, i32 0, i32 1
  store [1 x i32] [i32 1], [1 x i32]* %a
  %a2 = getelementptr inbounds %F, %F* %this, i32 0, i32 1
  %1 = getelementptr [1 x i32], [1 x i32]* %a2, i32 0, i32 0
  store i32 44, i32* %1
  ret void
}
