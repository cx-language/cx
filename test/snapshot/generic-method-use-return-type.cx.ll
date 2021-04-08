
%"X<int>" = type {}
%"X<bool>" = type {}

define i32 @main(%"X<int>" %x) {
  %a = alloca %"X<bool>", align 8
  %1 = alloca %"X<int>", align 8
  store %"X<int>" %x, %"X<int>"* %1, align 1
  %2 = call %"X<bool>" @_EN4main1XI3intE1fI4boolEE4bool(%"X<int>"* %1, i1 false)
  store %"X<bool>" %2, %"X<bool>"* %a, align 1
  call void @_EN4main1XI4boolE1gE(%"X<bool>"* %a)
  ret i32 0
}

define %"X<bool>" @_EN4main1XI3intE1fI4boolEE4bool(%"X<int>"* %this, i1 %u) {
  %x = alloca %"X<bool>", align 8
  %x.load = load %"X<bool>", %"X<bool>"* %x, align 1
  ret %"X<bool>" %x.load
}

define void @_EN4main1XI4boolE1gE(%"X<bool>"* %this) {
  ret void
}
