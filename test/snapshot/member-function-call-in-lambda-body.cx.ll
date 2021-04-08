
%X = type {}

define i32 @main() {
  %lambda = alloca i32 (%X*)*, align 8
  store i32 (%X*)* @_EN4main9__lambda0EP1X, i32 (%X*)** %lambda, align 8
  ret i32 0
}

define i32 @_EN4main9__lambda0EP1X(%X* %x) {
  %1 = call i32 @_EN4main1X3fooE(%X* %x)
  ret i32 %1
}

define i32 @_EN4main1X3fooE(%X* %this) {
  ret i32 42
}
