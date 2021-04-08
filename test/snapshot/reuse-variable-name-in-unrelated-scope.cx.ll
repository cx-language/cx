
%"X<int>" = type {}

define void @_EN4main1fE() {
  %index = alloca i32, align 4
  %x = alloca %"X<int>", align 8
  %1 = alloca %"X<int>", align 8
  store i32 0, i32* %index, align 4
  %2 = call %"X<int>" @_EN4main1gE()
  store %"X<int>" %2, %"X<int>"* %x, align 1
  call void @_EN4main1XI3intE4initE(%"X<int>"* %1)
  call void @_EN4main1XI3intE1gE(%"X<int>"* %1)
  ret void
}

define %"X<int>" @_EN4main1gE() {
  %x = alloca %"X<int>", align 8
  call void @_EN4main1XI3intE4initE(%"X<int>"* %x)
  %x.load = load %"X<int>", %"X<int>"* %x, align 1
  ret %"X<int>" %x.load
}

define void @_EN4main1XI3intE4initE(%"X<int>"* %this) {
  ret void
}

define void @_EN4main1XI3intE1gE(%"X<int>"* %this) {
  %index = alloca i32, align 4
  store i32 0, i32* %index, align 4
  ret void
}
