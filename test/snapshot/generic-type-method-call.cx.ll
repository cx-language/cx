
%"S<A>" = type {}
%A = type {}

define i32 @main() {
  %x = alloca %"S<A>", align 8
  call void @_EN4main1SI1AE4initE(%"S<A>"* %x)
  call void @_EN4main1SI1AE1sE(%"S<A>"* %x)
  ret i32 0
}

define void @_EN4main1SI1AE4initE(%"S<A>"* %this) {
  %1 = alloca %A, align 8
  call void @_EN4main1A4initE(%A* %1)
  call void @_EN4main1A1hE(%A* %1)
  ret void
}

define void @_EN4main1SI1AE1sE(%"S<A>"* %this) {
  ret void
}

define void @_EN4main1A4initE(%A* %this) {
  ret void
}

define void @_EN4main1A1hE(%A* %this) {
  ret void
}
