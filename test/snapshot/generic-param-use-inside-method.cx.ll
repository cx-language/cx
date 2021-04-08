
%"S<int>" = type {}

define i32 @main() {
  %s = alloca %"S<int>", align 8
  call void @_EN4main1SI3intE4initE(%"S<int>"* %s)
  call void @_EN4main1SI3intE1fE(%"S<int>"* %s)
  ret i32 0
}

define void @_EN4main1SI3intE4initE(%"S<int>"* %this) {
  ret void
}

define void @_EN4main1SI3intE1fE(%"S<int>"* %this) {
  %t = alloca i32, align 4
  call void @_EN4main1SI3intE1gE(%"S<int>"* %this)
  ret void
}

define void @_EN4main1SI3intE1gE(%"S<int>"* %this) {
  %t2 = alloca i32, align 4
  ret void
}
