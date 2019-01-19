
define void @_EN4main1fE3fooP3int3bar3int(i32* %foo, i32 %bar) {
  %foo.load = load i32, i32* %foo
  %1 = icmp slt i32 %foo.load, %bar
  ret void
}
