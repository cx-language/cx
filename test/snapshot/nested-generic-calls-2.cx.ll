
%"M<S>" = type { %S }
%S = type { i32 }
%"A<S>" = type {}

define i32 @main() {
  %m = alloca %"M<S>", align 8
  call void @_EN4main1MI1SE4initE(%"M<S>"* %m)
  call void @_EN4main1MI1SE1fE(%"M<S>"* %m)
  ret i32 0
}

define void @_EN4main1MI1SE4initE(%"M<S>"* %this) {
  ret void
}

define void @_EN4main1MI1SE1fE(%"M<S>"* %this) {
  %a = alloca %"A<S>", align 8
  %1 = alloca %S, align 8
  call void @_EN4main1AI1SE4initE(%"A<S>"* %a)
  %2 = call %S @_EN4main1AI1SE1aE(%"A<S>"* %a)
  store %S %2, %S* %1, align 4
  call void @_EN4main1S1iE(%S* %1)
  ret void
}

define void @_EN4main1AI1SE4initE(%"A<S>"* %this) {
  ret void
}

define %S @_EN4main1AI1SE1aE(%"A<S>"* %this) {
  %1 = alloca %S, align 8
  call void @_EN4main1S4initE(%S* %1)
  %.load = load %S, %S* %1, align 4
  ret %S %.load
}

define void @_EN4main1S1iE(%S* %this) {
  ret void
}

define void @_EN4main1S4initE(%S* %this) {
  ret void
}
