
%X = type {}

define i32 @_EN4main1gE() {
  ret i32 0
}

define i1 @_EN4main1hE() {
  ret i1 false
}

define i32 @main() {
  %x = alloca %X, align 8
  call void @_EN4main1X4initE(%X* %x)
  %1 = call i32 @_EN4main1X1fI3intEEF_3int(%X* %x, i32 ()* @_EN4main1gE)
  %2 = call i1 @_EN4main1X1fI4boolEEF_4bool(%X* %x, i1 ()* @_EN4main1hE)
  ret i32 0
}

define void @_EN4main1X4initE(%X* %this) {
  ret void
}

define i32 @_EN4main1X1fI3intEEF_3int(%X* %this, i32 ()* %t) {
  %1 = call i32 %t()
  ret i32 %1
}

define i1 @_EN4main1X1fI4boolEEF_4bool(%X* %this, i1 ()* %t) {
  %1 = call i1 %t()
  ret i1 %1
}
