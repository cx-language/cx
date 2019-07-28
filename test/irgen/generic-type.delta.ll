
%"A<int>" = type { i32 }
%"A<A<A<bool>>>" = type { %"A<A<bool>>" }
%"A<A<bool>>" = type { %"A<bool>" }
%"A<bool>" = type { i1 }

define i32 @main() {
  %a = alloca %"A<int>"
  %aaa = alloca %"A<A<A<bool>>>"
  ret i32 0
}

define void @_EN4main1AI3intE4initE1a3int(%"A<int>"* %this, i32 %a) {
  %a1 = getelementptr inbounds %"A<int>", %"A<int>"* %this, i32 0, i32 0
  store i32 %a, i32* %a1
  ret void
}

define void @_EN4main1AI4boolE4initE1a4bool(%"A<bool>"* %this, i1 %a) {
  %a1 = getelementptr inbounds %"A<bool>", %"A<bool>"* %this, i32 0, i32 0
  store i1 %a, i1* %a1
  ret void
}

define void @_EN4main1AI1AI4boolEE4initE1a1AI4boolE(%"A<A<bool>>"* %this, %"A<bool>" %a) {
  %a1 = getelementptr inbounds %"A<A<bool>>", %"A<A<bool>>"* %this, i32 0, i32 0
  store %"A<bool>" %a, %"A<bool>"* %a1
  ret void
}

define void @_EN4main1AI1AI1AI4boolEEE4initE1a1AI1AI4boolEE(%"A<A<A<bool>>>"* %this, %"A<A<bool>>" %a) {
  %a1 = getelementptr inbounds %"A<A<A<bool>>>", %"A<A<A<bool>>>"* %this, i32 0, i32 0
  store %"A<A<bool>>" %a, %"A<A<bool>>"* %a1
  ret void
}
