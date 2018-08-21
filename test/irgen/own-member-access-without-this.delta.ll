
%A = type { i32, i32 }
%B = type { i32, i32 }

define void @_ENM4main1A4initE(%A* %this) {
  %j = getelementptr inbounds %A, %A* %this, i32 0, i32 1
  store i32 42, i32* %j
  ret void
}

define void @_EN4main1A3fooE(%A* %this) {
  %j = getelementptr inbounds %A, %A* %this, i32 0, i32 1
  %j1 = load i32, i32* %j
  ret void
}

define void @_ENM4main1A3barE(%A* %this) {
  %j = getelementptr inbounds %A, %A* %this, i32 0, i32 1
  store i32 1, i32* %j
  ret void
}

define void @_ENM4main1B4initE(%B* %this) {
  %j = getelementptr inbounds %B, %B* %this, i32 0, i32 1
  store i32 42, i32* %j
  ret void
}

define void @_EN4main1B3fooE(%B %this) {
  %j = extractvalue %B %this, 1
  ret void
}

define void @_ENM4main1B3barE(%B* %this) {
  %j = getelementptr inbounds %B, %B* %this, i32 0, i32 1
  store i32 1, i32* %j
  ret void
}
