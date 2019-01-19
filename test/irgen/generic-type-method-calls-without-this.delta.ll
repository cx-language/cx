
define i32 @main() {
  %1 = alloca {}
  %2 = alloca {}
  call void @_ENM4main1SI3intE4initE({}* %1)
  %.load = load {}, {}* %1
  call void @_EN4main1SI3intE1fE({} %.load)
  call void @_ENM4main1SI5floatE4initE({}* %2)
  %.load1 = load {}, {}* %2
  call void @_EN4main1SI5floatE1fE({} %.load1)
  ret i32 0
}

define void @_EN4main1SI3intE1fE({} %this) {
  call void @_EN4main1SI3intE1gE({} %this)
  ret void
}

define void @_ENM4main1SI3intE4initE({}* %this) {
  ret void
}

define void @_EN4main1SI5floatE1fE({} %this) {
  call void @_EN4main1SI5floatE1gE({} %this)
  ret void
}

define void @_ENM4main1SI5floatE4initE({}* %this) {
  ret void
}

define void @_EN4main1SI5floatE1gE({} %this) {
  ret void
}

define void @_EN4main1SI3intE1gE({} %this) {
  ret void
}
