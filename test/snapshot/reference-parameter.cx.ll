
define i1 @_EN4main1fER3int(i32* %i) {
  %i.load = load i32, i32* %i, align 4
  %1 = icmp eq i32 %i.load, 1
  ret i1 %1
}

define i32 @main() {
  %1 = alloca i32, align 4
  store i32 1, i32* %1, align 4
  %2 = call i1 @_EN4main1fER3int(i32* %1)
  ret i32 0
}
