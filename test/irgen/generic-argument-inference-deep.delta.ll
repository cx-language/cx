
%"S<int>" = type { i32 }
%"S<bool>" = type { i1 }

define void @_EN4main1fE1c1SI3intE1d1SI3intE(%"S<int>" %c, %"S<int>" %d) {
  %1 = call i1 @_EN4maineqI3intEE1a1SI3intE1b1SI3intE(%"S<int>" %c, %"S<int>" %d)
  ret void
}

define i1 @_EN4maineqI3intEE1a1SI3intE1b1SI3intE(%"S<int>" %a, %"S<int>" %b) {
  ret i1 true
}

define void @_EN4main1fE1cP1SI4boolE1dP1SI4boolE(%"S<bool>"* %c, %"S<bool>"* %d) {
  %1 = load %"S<bool>", %"S<bool>"* %c
  %2 = load %"S<bool>", %"S<bool>"* %d
  %3 = call i1 @_EN4maineqI4boolEE1a1SI4boolE1b1SI4boolE(%"S<bool>" %1, %"S<bool>" %2)
  ret void
}

define i1 @_EN4maineqI4boolEE1a1SI4boolE1b1SI4boolE(%"S<bool>" %a, %"S<bool>" %b) {
  ret i1 true
}
