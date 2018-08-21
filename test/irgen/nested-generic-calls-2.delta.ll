
%S = type { i32 }
%"M<S>" = type { %S }

define void @_ENM4main1S4initE(%S* %this) {
  ret void
}

define void @_EN4main1S1iE(%S %this) {
  ret void
}

define i32 @main() {
  %m = alloca %"M<S>"
  call void @_ENM4main1MI1SE4initE(%"M<S>"* %m)
  call void @_EN4main1MI1SE1fE(%"M<S>"* %m)
  ret i32 0
}

define void @_ENM4main1MI1SE4initE(%"M<S>"* %this) {
  ret void
}

define void @_EN4main1MI1SE1fE(%"M<S>"* %this) {
  %a = alloca {}
  call void @_ENM4main1AI1SE4initE({}* %a)
  %1 = call %S @_EN4main1AI1SE1aE({}* %a)
  call void @_EN4main1S1iE(%S %1)
  ret void
}

define void @_ENM4main1AI1SE4initE({}* %this) {
  ret void
}

define %S @_EN4main1AI1SE1aE({}* %this) {
  %1 = alloca %S
  call void @_ENM4main1S4initE(%S* %1)
  %2 = load %S, %S* %1
  ret %S %2
}
