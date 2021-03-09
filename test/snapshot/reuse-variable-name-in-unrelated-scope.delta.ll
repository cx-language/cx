
define void @_EN4main1fE() {
  %index = alloca i32, align 4
  %x = alloca {}, align 8
  %1 = alloca {}, align 8
  store i32 0, i32* %index, align 4
  %2 = call {} @_EN4main1gE()
  store {} %2, {}* %x, align 1
  call void @_EN4main1XI3intE4initE({}* %1)
  call void @_EN4main1XI3intE1gE({}* %1)
  ret void
}

define {} @_EN4main1gE() {
  %x = alloca {}, align 8
  call void @_EN4main1XI3intE4initE({}* %x)
  %x.load = load {}, {}* %x, align 1
  ret {} %x.load
}

define void @_EN4main1XI3intE4initE({}* %this) {
  ret void
}

define void @_EN4main1XI3intE1gE({}* %this) {
  %index = alloca i32, align 4
  store i32 0, i32* %index, align 4
  ret void
}
