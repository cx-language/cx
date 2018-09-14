
define void @_EN4main1fE3fooP3int3bar3int(i32* %foo, i32 %bar) {
  %1 = load i32, i32* %foo
  %2 = icmp slt i32 %1, %bar
  ret void
}
