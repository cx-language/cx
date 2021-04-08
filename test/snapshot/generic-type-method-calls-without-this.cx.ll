
%"S<int>" = type {}
%"S<float>" = type {}

define i32 @main() {
  %1 = alloca %"S<int>", align 8
  %2 = alloca %"S<float>", align 8
  call void @_EN4main1SI3intE4initE(%"S<int>"* %1)
  call void @_EN4main1SI3intE1fE(%"S<int>"* %1)
  call void @_EN4main1SI5floatE4initE(%"S<float>"* %2)
  call void @_EN4main1SI5floatE1fE(%"S<float>"* %2)
  ret i32 0
}

define void @_EN4main1SI3intE4initE(%"S<int>"* %this) {
  ret void
}

define void @_EN4main1SI3intE1fE(%"S<int>"* %this) {
  call void @_EN4main1SI3intE1gE(%"S<int>"* %this)
  ret void
}

define void @_EN4main1SI5floatE4initE(%"S<float>"* %this) {
  ret void
}

define void @_EN4main1SI5floatE1fE(%"S<float>"* %this) {
  call void @_EN4main1SI5floatE1gE(%"S<float>"* %this)
  ret void
}

define void @_EN4main1SI3intE1gE(%"S<int>"* %this) {
  ret void
}

define void @_EN4main1SI5floatE1gE(%"S<float>"* %this) {
  ret void
}
