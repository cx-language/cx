
define i32 @main() {
  %s = alloca {}
  call void @_ENM4main1SI3intE4initE({}* %s)
  %s1 = load {}, {}* %s
  call void @_EN4main1SI3intE1fE({} %s1)
  ret i32 0
}

define void @_ENM4main1SI3intE4initE({}* %this) {
  ret void
}

define void @_EN4main1SI3intE1fE({} %this) {
  %t = alloca i32
  call void @_EN4main1SI3intE1gE({} %this)
  ret void
}

define void @_EN4main1SI3intE1gE({} %this) {
  %t2 = alloca i32
  ret void
}
