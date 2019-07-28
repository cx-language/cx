
%"S<int>" = type { i32 }
%"S<bool>" = type { i1 }

define void @_EN4main1SI3intE4initE1f3int(%"S<int>"* %this, i32 %f) {
  %f1 = getelementptr inbounds %"S<int>", %"S<int>"* %this, i32 0, i32 0
  store i32 %f, i32* %f1
  ret void
}

define void @_EN4main1fE1c1SI3intE1d1SI3intE(%"S<int>" %c, %"S<int>" %d) {
  %1 = call i1 @_EN4maineqI3intEE1a1SI3intE1b1SI3intE(%"S<int>" %c, %"S<int>" %d)
  ret void
}

define i1 @_EN4maineqI3intEE1a1SI3intE1b1SI3intE(%"S<int>" %a, %"S<int>" %b) {
  ret i1 true
}

define void @_EN4main1SI4boolE4initE1f4bool(%"S<bool>"* %this, i1 %f) {
  %f1 = getelementptr inbounds %"S<bool>", %"S<bool>"* %this, i32 0, i32 0
  store i1 %f, i1* %f1
  ret void
}

define void @_EN4main1fE1cP1SI4boolE1dP1SI4boolE(%"S<bool>"* %c, %"S<bool>"* %d) {
  %c.load = load %"S<bool>", %"S<bool>"* %c
  %d.load = load %"S<bool>", %"S<bool>"* %d
  %1 = call i1 @_EN4maineqI4boolEE1a1SI4boolE1b1SI4boolE(%"S<bool>" %c.load, %"S<bool>" %d.load)
  ret void
}

define i1 @_EN4maineqI4boolEE1a1SI4boolE1b1SI4boolE(%"S<bool>" %a, %"S<bool>" %b) {
  ret i1 true
}
