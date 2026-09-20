
%"S<int>" = type { i32 }
%"S<bool>" = type { i1 }

define void @_EN4main1fE1SI3intE1SI3intE(%"S<int>" %c, %"S<int>" %d) {
  %c1 = alloca %"S<int>", align 8
  %d2 = alloca %"S<int>", align 8
  store %"S<int>" %c, ptr %c1, align 4
  store %"S<int>" %d, ptr %d2, align 4
  %c.load = load %"S<int>", ptr %c1, align 4
  %d.load = load %"S<int>", ptr %d2, align 4
  %1 = call i1 @_EN4maineqI3intEE1SI3intE1SI3intE(%"S<int>" %c.load, %"S<int>" %d.load)
  ret void
}

define i1 @_EN4maineqI3intEE1SI3intE1SI3intE(%"S<int>" %a, %"S<int>" %b) {
  %a1 = alloca %"S<int>", align 8
  %b2 = alloca %"S<int>", align 8
  store %"S<int>" %a, ptr %a1, align 4
  store %"S<int>" %b, ptr %b2, align 4
  ret i1 true
}

define void @_EN4main1fEP1SI4boolEP1SI4boolE(ptr %c, ptr %d) {
  %c1 = alloca ptr, align 8
  %d2 = alloca ptr, align 8
  store ptr %c, ptr %c1, align 8
  store ptr %d, ptr %d2, align 8
  %c.load = load ptr, ptr %c1, align 8
  %c.load.load = load %"S<bool>", ptr %c.load, align 1
  %d.load = load ptr, ptr %d2, align 8
  %d.load.load = load %"S<bool>", ptr %d.load, align 1
  %1 = call i1 @_EN4maineqI4boolEE1SI4boolE1SI4boolE(%"S<bool>" %c.load.load, %"S<bool>" %d.load.load)
  ret void
}

define i1 @_EN4maineqI4boolEE1SI4boolE1SI4boolE(%"S<bool>" %a, %"S<bool>" %b) {
  %a1 = alloca %"S<bool>", align 8
  %b2 = alloca %"S<bool>", align 8
  store %"S<bool>" %a, ptr %a1, align 1
  store %"S<bool>" %b, ptr %b2, align 1
  ret i1 true
}
